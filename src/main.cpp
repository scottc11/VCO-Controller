#include "main.h"
#include "BeatClock.h"
#include "ChannelEventList.h"
#include "CAP1208.h"
#include "ShiftRegister.h"
#include "MIDI.h"
#include "TCA9544A.h"
#include "MCP23017.h"
#include "RotaryEncoder.h"


I2C i2c1(I2C_SDA, I2C_SCL);
I2C i2c3(I2C3_SDA, I2C3_SCL);

Ticker ticker;
Timer timer;
InterruptIn extClockInput(EXT_CLOCK_INPUT);
InterruptIn touchAInt(TOUCH_INT_A, PullUp);
InterruptIn touchBInt(TOUCH_INT_B, PullUp);
InterruptIn degreeInt(DEGREES_INT);

MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);
MCP23017 ioA(&i2c3, MCP23017_CHAN_A_ADDR);
MCP23017 ioB(&i2c3, MCP23017_CHAN_B_ADDR);
MCP23017 ioC(&i2c3, MCP23017_CHAN_C_ADDR);
MCP23017 ioD(&i2c3, MCP23017_CHAN_D_ADDR);
MIDI midi;

ShiftRegister display(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LATCH);
DigitalOut latch(DISPLAY_LATCH);
CAP1208 touchA;
CAP1208 touchB;
TCA9544A i2cMux(&i2c1, TCA9544A_ADDR);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList channelA(GATE_OUT_A, &ioA, &midi);
RotaryEncoder encoder(ENCODER_CHAN_A, ENCODER_CHAN_B, ENCODER_BTN);

bool ETL = false;       // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
int newClockPeriod;
int oldClockPeriod;
int clockPeriod;
volatile int interupt = 0;
bool degreeFlag = false;

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

void CAP1208_Interupt() {
  interupt = 1;
}

void degreeInteruptCallback() {
  degreeFlag = true;
}

int main() {
  
  encoder.init();
  
  channelA.init();

  degreeInt.fall(&degreeInteruptCallback);


  // init display
  // display.writeByte(numbers[9]);
  // display.writeByte(numbers[9]);
  // display.pulseLatch();

  touchAInt.fall(&CAP1208_Interupt);

  touchA.init(&i2c1, &i2cMux, 0);

  if (!touchA.isConnected()) {
    display.writeByte(numbers[5]);
    display.writeByte(numbers[6]);
    display.pulseLatch();
  }
  else {
    display.writeByte(numbers[9]);
    display.writeByte(numbers[9]);
    display.pulseLatch();
  }
  touchA.calibrate();

  int touched = touchA.touched();
  int prevTouched = 0;

  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);

  while(1) {
    
    if (encoder.btnPressed()) {
      // somthin
    } else {
      // boardLED.write(LOW);
    }

    if (degreeFlag) {
      io.digitalRead(MCP23017_PORTA);
      io.digitalWrite(MCP23017_PORTB, ~io.digitalRead(MCP23017_PORTB));
      degreeFlag = false;
    }

    if (interupt == 1) {
      touched = touchA.touched();
      if (touched != prevTouched) {
        for (int i=0; i<8; i++) {
          // if it *is* touched and *wasnt* touched before, alert!
          if (touchA.getBitStatus(touched, i) && !touchA.getBitStatus(prevTouched, i)) {
            ETL = false; // deactivate event triggering loop
            channelA.createEvent(bClock.position, i);
            channelA.setLed(i);
            // channelA.updateLeds(touched);
          }
          // if it *was* touched and now *isnt*, alert!
          if (!touchA.getBitStatus(touched, i) && touchA.getBitStatus(prevTouched, i)) {
            channelA.addEvent(bClock.position);
            // channelA.updateLeds(touched);
            ETL = true; // activate event triggering loop
          }
        }
        // reg.writeByte(touched); // toggle channel LEDs
        prevTouched = touched;
      }

      interupt = 0;
    }



    if (channelA.hasEventInQueue() && ETL ) {
      channelA.handleQueuedEvent(bClock.position);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop