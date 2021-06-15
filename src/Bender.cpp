#include "Bender.h"
#include "ArrayMethods.h"

void Bender::init() {
    dac->init();
    calibrateIdle();
    updateDAC(0);
    this->mode = 0;
}

void Bender::calibrateIdle()
{
    // must set initial value for digital filter 
    filter.setInitial(adc.read_u16());

    // populate calibration array
    for (int i = 0; i < PB_CALIBRATION_RANGE; i++)
    {
        calibrationSamples[i] = this->read();
        wait_us(100);
    }

    // find min/max value from calibration results
    int max = arr_max(calibrationSamples, PB_CALIBRATION_RANGE);
    int min = arr_min(calibrationSamples, PB_CALIBRATION_RANGE);
    this->idleDebounce = (max - min);

    // zero the sensor
    this->zeroBend = arr_average(calibrationSamples, PB_CALIBRATION_RANGE);
    return;
}

/**
 * chan A @ 220ohm gain: Max = 45211, Min = 27228, zero = 35432, debounce = 496
 * chan B @ 220ohm gain: Max = XXXXX, Min = XXXXX, zero = 32931, debounce = 337
 * chan C @ 220ohm gain: Max = 48276, Min = 28566, zero = 38468, debounce = 448
 * chan D @ 220ohm gain: Max = 43002, Min = 25974, zero = 35851, debounce = 320
*/
void Bender::calibrateMinMax() {
    this->read();
    if (currBend > zeroBend + 1000) // bending upwards
    {
        if (currBend > maxBend) {
            maxBend = currBend;
        }
    }
    else if (currBend < zeroBend - 1000) // bending downwards
    {
        if (currBend < minBend) {
            minBend = currBend;
        }
    }
    prevBend = currBend;
}

// polling should no longer check if the bender is idle. It should just update the DAC and call the activeCallback
// there are no cycles being saved either way.
void Bender::poll()
{
    prevBend = currBend;    // not sure what prevBend is for
    this->read();
    
    if (this->isIdle())
    {
        updateDAC(0);
        if (idleCallback) { idleCallback(); }
    }
    else
    {
        // determine direction of bend
        output = calculateOutput(currBend);
        updateDAC(output);
        
        if (activeCallback) { activeCallback(currBend); }
    }
}

/**
 * Map the ADC input to a greater range so the DAC can make use of all 16-bits
 * Output will be between 0V and 2.5V, centered at 2.5V/2
*/
int Bender::calculateOutput(uint16_t value)
{
    if (value > zeroBend && value < maxBend)
    {
        // map the ADC reading within a range to be output through the DAC
        return ((dacOutputRange / (maxBend - zeroBend)) * (value - zeroBend)) * 1; // non-inverted
    }
    else if (value < zeroBend && value > minBend)
    {
        return ((dacOutputRange / (minBend - zeroBend)) * (value - zeroBend)) * -1; // inverted
    }
    else {
        return 0;
    }
}

void Bender::updateDAC(uint16_t value)
{
    dac->write(dacChan, 32767 + value);
}

bool Bender::isIdle() {
    if (currBend > zeroBend + idleDebounce || currBend < zeroBend - idleDebounce) {
        return false;
    } else {
        return true;
    }
}

void Bender::attachIdleCallback(Callback<void()> func)
{
    idleCallback = func;
}

void Bender::attachActiveCallback(Callback<void(uint16_t bend)> func)
{
    activeCallback = func;
}

uint16_t Bender::read() {
    currBend = filter(adc.read_u16());
    return currBend;
}