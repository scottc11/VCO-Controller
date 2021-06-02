#ifndef __BENDER_H
#define __BENDER_H

#include "main.h"
#include "DAC8554.h"

#define PB_CALIBRATION_RANGE 64

class Bender {
public:
    
    DAC8554 *dac;                    // pointer to Pitch Bends DAC
    DAC8554::Channels dacChan;       // which dac to address
    AnalogIn adc;                    // CV input via Instrumentation Amplifier
    Callback<void()> idleCallback;   // MBED Callback which gets called when the Bender is idle / not-active
    Callback<void(uint16_t value)> activeCallback; // MBED Callback which gets called when the Bender is active / being bent

    int mode;
    int currReading;                       // 16 bit value (0..65,536)
    int prevReading;                       // 16 bit value (0..65,536)
    float outputRange = 32767;             // range in which the DAC can output (in either direction)
    int output;                            // the amount of Control Voltage to apply Pitch Bend DAC
    int calibrationSamples[PB_CALIBRATION_RANGE]; // an array which gets populated during initialization phase to determine a debounce value + zeroing

    uint16_t idleValue;                          // the average ADC value when pitch bend is idle
    uint16_t idleDebounce;                       // for debouncing the ADC when Pitch Bend is idle
    uint16_t maxBend;                            // the minimum value the ADC can achieve when Pitch Bend fully pulled
    uint16_t minBend;                            // the maximum value the ADC can achieve when Pitch Bend fully pressed

    Bender(DAC8554 *dac_ptr, DAC8554::Channels _dacChan, PinName adcPin) : adc(adcPin)
    {
        dac = dac_ptr;
        dacChan = _dacChan;
    };
    void init();
    void poll();
    void calibrate();
    void updateDAC(uint16_t value);
    bool isIdle();
    int setMode(int targetMode = 0);
    int calculateOutput(uint16_t value);
    void attachActiveCallback();
    void attachIdleCallback();
};

#endif