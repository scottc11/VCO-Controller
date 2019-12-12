#include "main.h"
#include "BeatClock.h"
#include "ChannelEventList.h"


DigitalOut eventLed(PB_5);
DigitalIn button(PA_7);
Ticker ticker;
Timer timer;
InterruptIn extClockInput(PB_10);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList chEventList;

bool ETL = false;       // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
int newClockPeriod;
int oldClockPeriod;
int clockPeriod;
int newButtonState;
int oldButtonState;

void tick() {
  bClock.tick();
}

void extTick() {
  oldClockPeriod = newClockPeriod;
  newClockPeriod = timer.read_us();
  clockPeriod = newClockPeriod - oldClockPeriod;
  ticker.attach_us(&tick, clockPeriod / PPQ);  // potentially write this as a flag and update in main loop
}

int main() {
  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);
  
  while(1) {
    newButtonState = button.read();
    if (newButtonState != oldButtonState) {

      // BUTTON PRESSED
      if (newButtonState == HIGH) {
        ETL = false; // deactivate event triggering loop
        eventLed.write(HIGH);
        chEventList.createEvent(bClock.position);
      }

      // BUTTON RELEASED
      else if (newButtonState == LOW) {     
        eventLed.write(LOW);
        chEventList.addEvent(bClock.position);
        ETL = true; // activate event triggering loop
      }

      oldButtonState = newButtonState;
      wait_us(2); // debounce
    }

    if (chEventList.hasEventInQueue() && ETL ) {
      chEventList.handleQueuedEvent(bClock.position);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop