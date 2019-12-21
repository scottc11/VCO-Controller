#include "main.h"
#include "BeatClock.h"
#include "ChannelEventList.h"
#include "CAP1208.h"
#include "ShiftRegister.h"
#include "MIDI.h"

I2C i2c(I2C_SDA, I2C_SCL);
DigitalOut boardLED(LED1);
Ticker ticker;
Timer timer;
InterruptIn extClockInput(EXT_CLOCK_INPUT);

MIDI midi;
ShiftRegister reg(SHIFT_REG_DATA, SHIFT_REG_CLOCK, SHIFT_REG_LATCH);
ShiftRegister display(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LATCH);
DigitalOut latch(DISPLAY_LATCH);
CAP1208 cap;
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList chEventList(CHANNEL_GATE, &reg, &midi);

bool ETL = false;       // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
int newClockPeriod;
int oldClockPeriod;
int clockPeriod;
int newButtonState;
int oldButtonState;

const char numbers[10] = { 0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b00111110, 0b11100000, 0b11111110, 0b11100110 };

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

  // init display
  for (int i = 0; i < 10; i++)
  {
    display.writeByte(numbers[i]);
    display.writeByte(numbers[1]);
    display.pulseLatch();
    wait_ms(100);
  }


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
        // if it *is* touched and *wasnt* touched before, alert!
        if (cap.getBitStatus(touched, i) && !cap.getBitStatus(prevTouched, i)) {
          ETL = false; // deactivate event triggering loop
          chEventList.createEvent(bClock.position, i);
        }
        // if it *was* touched and now *isnt*, alert!
        if (!cap.getBitStatus(touched, i) && cap.getBitStatus(prevTouched, i)) {
          chEventList.addEvent(bClock.position);
          ETL = true; // activate event triggering loop
        }
      }
      reg.writeByte(touched); // toggle channel LEDs
      prevTouched = touched;
    }

    if (chEventList.hasEventInQueue() && ETL ) {
      chEventList.handleQueuedEvent(bClock.position);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop