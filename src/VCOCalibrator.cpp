#include "VCOCalibrator.h"

void VCOCalibrator::setChannel(TouchChannel *chan)
{
    channel = chan;
}

void VCOCalibrator::enableCalibrationMode()
{
    calibrationFinished = false;

    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::LOW);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(5000);
    channel->setAllLeds(TouchChannel::LOW);
    wait_us(5000);
    channel->setLed(0, TouchChannel::HIGH);

    channel->dac->write(channel->dacChannel, channel->dacVoltageValues[0]); // start at bottom most note.

    ticker.attach_us(callback(this, &VCOCalibrator::sampleVCOFrequency), VCO_SAMPLE_RATE_US);
}

void VCOCalibrator::disableCalibrationMode()
{
    ticker.detach();  // disable ticker
    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(100000);
    channel->setAllLeds(TouchChannel::LOW);
    wait_us(100000);
    channel->setAllLeds(TouchChannel::HIGH);
    wait_us(100000);
    channel->setAllLeds(TouchChannel::LOW);

    // deactivate calibration mode
    calNoteIndex = 0;            // ?
    calibrationFinished = true;
    channel->setMode(TouchChannel::MONO);
}

void VCOCalibrator::calibrateVCO()
{
    // wait till MAX_FREQ_SAMPLES has been obtained
    if (readyToCalibrate)
    {
        // handle first iteration of calibrating by finding the frequency to start at
        if (prevAvgFreq == 0 && avgFreq != 0)
        {
            int n = 0;
            while (avgFreq > PITCH_FREQ[n])
            {
                initialFrequencyIndex = n;
                calNoteIndex = initialFrequencyIndex;
                n += 1;
                if (n >= NUM_PITCH_FREQENCIES - 1)
                {
                    break;
                }
            }
        }

        prevAvgFreq = avgFreq;
        float threshold = 0.1;
        avgFreq = this->calculateAverageFreq(); // determine the new average frequency
        int dacIndex = calNoteIndex - initialFrequencyIndex;

        // if avgFreq is close enough to desired freq
        if ((avgFreq <= PITCH_FREQ[calNoteIndex] + threshold && avgFreq >= PITCH_FREQ[calNoteIndex] - threshold) || calibrationAttemps > MAX_CALIB_ATTEMPTS)
        {

            // move to next pitch to be calibrated
            if (calNoteIndex < CALIBRATION_LENGTH + initialFrequencyIndex)
            {
                // reset leds LOW every octave
                channel->setLed(CALIBRATION_LED_MAP[dacIndex == 0 ? 0 : dacIndex - 1], TouchChannel::LOW);
                adjustment = DEFAULT_VOLTAGE_ADJMNT; // reset to default
                calibrationAttemps = 0;
                calNoteIndex += 1; // increase note index by 1

                channel->setLed(CALIBRATION_LED_MAP[dacIndex], TouchChannel::HIGH);
            }

            else // finished calibrating
            {
                channel->generateDacVoltageMap(); // set dac map to use new calibrated values
                this->disableCalibrationMode();
            }
        }
        else     // avgFreq not close enough
        {
            // every time avgFreq over/undershoots the desired frequency, decrement the 'adjustment' value by half.

            int currVal = channel->dacVoltageValues[dacIndex]; // pre-calibrated value to be adjusted

            if (avgFreq > PITCH_FREQ[calNoteIndex] + threshold)
            { // if overshoot
                if (prevAvgFreq < PITCH_FREQ[calNoteIndex] - threshold)
                {
                    overshoot = true;
                    adjustment = (adjustment / 2) + 1; // so it never becomes zero
                }
                currVal -= adjustment;
                channel->dacVoltageValues[dacIndex] = currVal;
            }

            else if (avgFreq < PITCH_FREQ[calNoteIndex] - threshold)
            { // if undershoot
                if (prevAvgFreq > PITCH_FREQ[calNoteIndex] + threshold)
                {
                    overshoot = false;
                    adjustment = (adjustment / 2) + 1; // so it never becomes zero
                }
                currVal += adjustment;
                channel->dacVoltageValues[dacIndex] = currVal;
            }
        }
        
        // output new voltage and reset calibration process
        channel->dac->write(channel->dacChannel, channel->dacVoltageValues[dacIndex]);
        wait_us(10000);           // give time for new voltage to 'settle'
        freqSampleIndex = 0;
        readyToCalibrate = false; // flag telling interupt routine to start sampling again
        calibrationAttemps += 1;
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
    if (!readyToCalibrate)
    {
        currVCOInputVal = channel->cvInput.read_u16();  // sample the ADC

        // NEGATIVE SLOPE
        // TODO: maybe plus threshold zero crossing
        if (currVCOInputVal >= (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal < (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && slopeIsPositive)
        {
            slopeIsPositive = false;
        }
        // POSITIVE SLOPE
        // TODO: maybe negative threshold zero crossing
        else if (currVCOInputVal <= (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal > (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && !slopeIsPositive)
        {
            vcoPeriod = numSamplesTaken;                   // how many samples have occurred between positive zero crossings
            vcoFrequency = 8000.00 / vcoPeriod;            // sample rate divided by period of input signal
            freqSamples[freqSampleIndex] = vcoFrequency;   // store sample in array
            numSamplesTaken = 0;                           // reset sample count to zero for the next sampling routine

            if (freqSampleIndex < MAX_FREQ_SAMPLES)
            {
                freqSampleIndex += 1;
            }
            else
            {
                readyToCalibrate = true;
                freqSampleIndex = 0;
            }
            slopeIsPositive = true;
        }

        prevVCOInputVal = currVCOInputVal;
        numSamplesTaken++;
    }
}