#include "VCOCalibrator.h"

void VCOCalibrator::setChannel(TouchChannel *chan)
{
    channel = chan;
}

void VCOCalibrator::enableCalibrationMode()
{
    calibrationFinished = false;
    calibrationAttemps = 0;
    pitchIndex = 0;
    initialPitchIndex = 0;
    freqSampleIndex = 0;
    numSamplesTaken = 0;
    avgFreq = 0;
    prevAvgFreq = 0;
    calLedIndex = 0;
    sampleVCO = true;
    currState = SAMPLING_LOW;
    adjustment = DEFAULT_VOLTAGE_ADJMNT;

    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::LOW);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::LOW);
    wait_us(5000);
    channel->setLed(0, TouchChannel::HIGH);

    channel->output1V.resetVoltageMap();

    channel->setOctaveLed(0, TouchChannel::LOW);
    channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[0]); // start at bottom most voltage.

    ticker.attach_us(callback(this, &VCOCalibrator::sampleVCOFrequency), VCO_SAMPLE_RATE_US);
}


void VCOCalibrator::calibrateVCO()
{
    // wait till MAX_FREQ_SAMPLES has been obtained
    if (!sampleVCO)
    {
        switch (currState) {
            
            case SAMPLING_LOW:
                channel->setOctaveLed(0, TouchChannel::LedState::HIGH);
                samples.s1.frequency = this->calculateAverageFreq(); // determine the average frequency of all frewuency samples
                samples.s1.voltage = channel->output1V.dacVoltageMap[0];
                // prepare sample s2
                channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[24]);
                wait_us(100);
                freqSampleIndex = 0;
                currState = State::SAMPLING_MID;
                sampleVCO = true;
                break;

            case SAMPLING_MID:
                channel->setOctaveLed(1, TouchChannel::LedState::HIGH);
                samples.s2.frequency = this->calculateAverageFreq(); // determine the average frequency of all frewuency samples
                samples.s2.voltage = channel->output1V.dacVoltageMap[24];
                // prepare sample s3
                channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[60]);
                wait_us(100);
                freqSampleIndex = 0;
                currState = State::SAMPLING_HIGH;
                sampleVCO = true;
                break;
            
            case SAMPLING_HIGH:
                channel->setOctaveLed(2, TouchChannel::LedState::HIGH);
                samples.s3.frequency = this->calculateAverageFreq(); // determine the average frequency of all frewuency samples
                samples.s3.voltage = channel->output1V.dacVoltageMap[60];
                freqSampleIndex = 0;
                currState = State::CALIBRATING;

                for (int i = 0; i < DAC_1VO_ARR_SIZE; i++)
                {
                    uint16_t predictedVoltage = samples.predictVoltage(PITCH_FREQ[i + 6]);
                    channel->output1V.dacVoltageMap[i] = predictedVoltage;
                }
                

                break;
            
            default:
                break;
        }
    }
}


// Since once frequency detection is not always accurate, take a running average of MAX_FREQ_SAMPLES
// NOTE: the first sample in freqSamples[] will always be scewed since the numSamplesTaken (since the first positive zero crossing)
// could be any value between 1 and the average sample time (between positive zero crossings)
float VCOCalibrator::calculateAverageFreq()
{
    float sum = 0;
    for (int i = 1; i < MAX_FREQ_SAMPLES; i++)
    {
        sum += this->freqSamples[i];
    }
    return sum / (MAX_FREQ_SAMPLES - 1);
}


/**
 * CALLBACK executing at desired VCO_SAMPLE_RATE_US
 * TODO: implement CircularBuffer -> https://os.mbed.com/docs/mbed-os/v5.14/apis/circularbuffer.html as it is "thread safe"
 * 
*/
void VCOCalibrator::sampleVCOFrequency()
{
    if (sampleVCO)
    {
        currVCOInputVal = channel->cvInput.read_u16();  // sample the ADC
        // NEGATIVE SLOPE
        if (currVCOInputVal >= (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal < (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && slopeIsPositive)
        {
            slopeIsPositive = false;
        }
        // POSITIVE SLOPE
        else if (currVCOInputVal <= (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal > (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && !slopeIsPositive)
        {
            float vcoPeriod = numSamplesTaken;             // how many samples have occurred between positive zero crossings
            vcoFrequency = 8000.0 / vcoPeriod;             // sample rate divided by period of input signal
            freqSamples[freqSampleIndex] = vcoFrequency;   // store sample in array
            numSamplesTaken = 0;                           // reset sample count to zero for the next sampling routine

            if (freqSampleIndex < MAX_FREQ_SAMPLES)
            {
                freqSampleIndex += 1;
            }
            else
            {
                freqSampleIndex = 0;
                sampleVCO = false;
            }
            slopeIsPositive = true;
        }

        prevVCOInputVal = currVCOInputVal;
        numSamplesTaken++;
    }
}