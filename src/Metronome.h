#ifndef BEAT_CLOCK_H
#define BEAT_CLOCK_H

#include "main.h"

class Metronome {
  public:
    Metronome() {};

    uint8_t bpm;
    uint8_t currStep;          // used to calculate an events position
    uint8_t numSteps;          // used to calculate total clock loop length (in ticks)
    uint8_t ticksPerStep;      // equal to PPQN
    uint8_t currTick;          // relative to PPQN
    uint16_t position;         // the clocks current position with the loop. Will be a multiplication of currTick and currStep
    uint32_t loopStart;        // time when the first step occurs on the system clock
    uint32_t pulseDuration;    // how long, in microseconds, the clock led will be lit
    uint32_t lastClock;        // time of the last clocked event

    void init();
    void tick();
    void step();
    void reset();
    void setNumberOfSteps(int num);
    void handleEncoder();
};


#endif