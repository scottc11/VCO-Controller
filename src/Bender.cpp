#include "Bender.h"
#include "ArrayMethods.h"

void Bender::init() {
    dac->init();
    calibrate();
    updateDAC(0);
    this->mode = 0;
}

void Bender::calibrate()
{
    // NOTE: this calibration process is currently flawed, because in the off chance there is an erratic
    // sensor ready in the positive or negative direction, the min / max values used to determine the debounce
    // value would be too far apart, giving a poor debounce value. Additionally, idleValue would also not be very accurate due to these
    // readings. I actually think some DSP smoothing is necessary here, to remove the "noise". Or perhaps just adding debounce caps
    // on the hardware will help this problem.

    // populate calibration array
    for (int i = 0; i < PB_CALIBRATION_RANGE; i++)
    {
        calibrationSamples[i] = adc.read_u16();
        wait_us(100);
    }

    // RUNNING-MEAN time series filter
    // the start and end of the filtered signal is always going to look weird, and you will not want to include it in your final output
    int filteredSignal[PB_CALIBRATION_RANGE];
    int sampleWindow = 2; // how many samples to use both forwards and backwards in the array

    for (int i = sampleWindow + 1; i < PB_CALIBRATION_RANGE - sampleWindow - 1; i++)
    {
        int sum = 0;
        int mean = 0;

        for (int x = i - sampleWindow; x < i + sampleWindow; x++) // calulate the mean of sample window
        {
            sum += calibrationSamples[x];
        }

        mean = sum / (sampleWindow * 2);

        filteredSignal[i] = mean;
    }

    // find min/max value from calibration results
    int max = arr_max(calibrationSamples, PB_CALIBRATION_RANGE);
    int min = arr_min(calibrationSamples, PB_CALIBRATION_RANGE);
    // int max = arr_max(filteredSignal + sampleWindow + 1, PB_CALIBRATION_RANGE - (sampleWindow * 2) - 2);
    // int min = arr_min(filteredSignal + sampleWindow + 1, PB_CALIBRATION_RANGE - (sampleWindow * 2) - 2);
    this->idleDebounce = (max - min);

    // zero the sensor
    idleValue = arr_average(filteredSignal + sampleWindow + 1, PB_CALIBRATION_RANGE - (sampleWindow * 2) - 2); // use the mean filtered signal

    int minMaxOffset = 10000;
    maxBend = idleValue + minMaxOffset < 65000 ? idleValue + minMaxOffset : 65000;
    minBend = idleValue - minMaxOffset < 500 ? idleValue - minMaxOffset : 500;
}


void Bender::poll()
{
    prevReading = currReading;    // not sure what prevReading is for
    currReading = adc.read_u16();
    
    if (this->isIdle())
    {
        updateDAC(0);
        if (idleCallback) { idleCallback(); }
    }
    else
    {
        // determine direction of bend
        output = calculateOutput(currReading);
        updateDAC(output);
        
        if (activeCallback) { activeCallback(currReading); }
    }
}

/**
 * Map the ADC input to a greater range so the DAC can make use of all 16-bits
 * Output will be between 0V and 2.5V, centered at 2.5V/2
*/
int Bender::calculateOutput(uint16_t value)
{
    if (value > idleValue && value < maxBend)
    {
        // map the ADC reading within a range to be output through the DAC
        return ((outputRange / (maxBend - idleValue)) * (value - idleValue)) * 1; // non-inverted
    }
    else if (value < idleValue && value > minBend)
    {
        return ((outputRange / (minBend - idleValue)) * (value - idleValue)) * -1; // inverted
    }
}

void Bender::updateDAC(uint16_t value)
{
    dac->write(dacChan, 32767 + value);
}

bool Bender::isIdle() {
    if (currReading > idleValue + idleDebounce || currReading < idleValue - idleDebounce) {
        return false;
    } else {
        return true;
    }
}