#include "main.h"
#include "BeatClock.h"
#include "ChannelEventList.h"
#include "CAP1208.h"
#include "ShiftRegister.h"


I2C i2c(I2C_SDA, I2C_SCL);  // PB_8, PB_9
DigitalOut boardLED(LED1);

Ticker ticker;
Timer timer;
InterruptIn extClockInput(PB_10);

ShiftRegister reg(SHIFT_REG_DATA, SHIFT_REG_CLOCK);
CAP1208 cap;
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList chEventList(CHANNEL_GATE);

bool ETL = false;       // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
int newClockPeriod;
int oldClockPeriod;
int clockPeriod;
int newButtonState;
int oldButtonState;

// bitNum starts at 0-7 for 8-bits
bool getBitStatus(int b, int bitNum) {
  return (b & (1 << bitNum));
}

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
  boardLED.write(HIGH);
  
  cap.init(&i2c);

  if (!cap.isConnected()) {
    boardLED.write(HIGH);
  } else {
    boardLED.write(LOW);
  }


  cap.getControlStatus();
  cap.getGeneralStatus();
  cap.calibrate();
  int touched = 0;
  int prevTouched = 0;

  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);
  
  while(1) {

    touched = cap.touched();
    if (touched != prevTouched) {
      for (int i=0; i<8; i++) {
        // it if *is* touched and *wasnt* touched before, alert!
        if (getBitStatus(touched, i) && !getBitStatus(prevTouched, i)) {
          reg.writeByte(touched);
          ETL = false; // deactivate event triggering loop
          chEventList.createEvent(bClock.position);
        }
        // if it *was* touched and now *isnt*, alert!
        if (!getBitStatus(touched, i) && getBitStatus(prevTouched, i)) {
          reg.writeByte(touched);
          chEventList.addEvent(bClock.position);
          ETL = true; // activate event triggering loop
        }
      }
      prevTouched = touched;
    }

    if (chEventList.hasEventInQueue() && ETL ) {
      chEventList.handleQueuedEvent(bClock.position);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop