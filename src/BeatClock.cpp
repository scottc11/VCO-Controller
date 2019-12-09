#include "BeatClock.h"

void BeatClock::init() {
  bpm = 120;
  currStep = 1;
  numSteps = 8;
  ticksPerStep = 24;
  currTick = 1;
  pulseDuration = 5;
}