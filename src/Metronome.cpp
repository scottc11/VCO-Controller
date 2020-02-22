#include "Metronome.h"

// clock initialization
void Metronome::init() {
  bpm = 120;
  currStep = 1;
  numSteps = DEFAULT_CHANNEL_LOOP_STEPS;
  ticksPerStep = PPQN;
  currTick = 1;
  pulseDuration = 5;
}


void Metronome::tick() {
  currTick += 1;
  position += 1;

  if (currTick > 2) {
    stepLed->write(LOW);
    startLed->write(LOW);
  }
  
  // reset tick count and increment step by 1
  if (currTick > ticksPerStep) {
    currTick = 1;
    currStep += 1;
    stepLed->write(HIGH);
    
    // reset step count / reset loop
    if (currStep > numSteps) {
      currStep = 1;
      position = 1;
      startLed->write(HIGH);
    }
  }
}

void Metronome::setNumberOfSteps(int num) {
  numSteps = num;
}