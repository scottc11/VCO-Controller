#include "main.h"
#include "BeatClock.h"

Ticker ticker;
Timer timer;
InterruptIn extClockInput(PB_10);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);

int newClockPeriod;
int oldClockPeriod;
int clockPeriod;

void tick() {
  bClock.tick();
}

void extTick() {
  oldClockPeriod = newClockPeriod;
  newClockPeriod = timer.read_us();
  clockPeriod = newClockPeriod - oldClockPeriod;
  ticker.attach_us(&tick, clockPeriod / PPQ);
}

int main() {
  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);
  
  while(1) {
    wait(2);
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop