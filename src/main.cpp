#include <mbed.h>

#define LOOP_START_LED_PIN   PA_9
#define LOOP_STEP_LED_PIN    PA_8

#include "BeatClock.h"

Ticker ticker;
InterruptIn extClockInput(PB_10);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);

void tick() {
  bClock.toggle();
}

void extTick() {
  // nothing
}

int main() {

  bClock.init();

  ticker.attach(&tick, 1.0);
  extClockInput.rise(&extTick);
  
  while(1) {
    // put your main code here, to run repeatedly:
  }
}