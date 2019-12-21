#ifndef BEAT_CLOCK_H
#define BEAT_CLOCK_H

#include "main.h"

class BeatClock {
  public:
    BeatClock(PinName startLedPin, PinName stepLedPin) {
      startLed = new DigitalOut(startLedPin);
      stepLed = new DigitalOut(stepLedPin);
    };

    uint8_t bpm;
    uint8_t currStep;          // used to calculate an events position
    uint8_t numSteps;          // used to calculate total clock loop length (in ticks)
    uint8_t ticksPerStep;      // equal to PPQ
    uint8_t currTick;          // relative to PPQ
    uint16_t position;         // the clocks current position with the loop. Will be a multiplication of currTick and currStep
    uint32_t loopStart;        // time when the first step occurs on the system clock
    uint32_t pulseDuration;    // how long, in microseconds, the clock led will be lit
    uint32_t lastClock;        // time of the last clocked event

    DigitalOut* startLed;      // digital out
    DigitalOut* stepLed;       // digital out

    void init();
    void tick();
    void reset();
    void handleEncoder();
};


#endif