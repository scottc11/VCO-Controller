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
MIDI midi;

ShiftRegister display(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LATCH);
DigitalOut latch(DISPLAY_LATCH);
CAP1208 touchA;
CAP1208 touchB;
TCA9544A i2cMux(&i2c1, TCA9544A_ADDR);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList chEventList(GATE_OUT_A, &io, &midi);
RotaryEncoder encoder(ENCODER_CHAN_A, ENCODER_CHAN_B, ENCODER_BTN);

bool ETL = false;       // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
int newClockPeriod;
int oldClockPeriod;
int clockPeriod;
volatile int interupt = 0;
int numInterupts = 0;
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

void secondInterupt() {
  interupt = 1;
}

void degreeInteruptCallback() {
  degreeFlag = true;
}

int main() {
  
  encoder.init();
  
  // io init
  io.init();
  io.setDirection(MCP23017_PORTA, 0xFF);    // set all of the PORTA pins to input
  io.setDirection(MCP23017_PORTB, 0x00);    // set all of the PORTB pins to output
  io.setPullUp(MCP23017_PORTA, 0xFF);        // activate all of the PORTA pin pull-ups
  io.setInputPolarity(MCP23017_PORTA, 0xFF); // invert all of the PORTA pins input polarity
  io.setInterupt(MCP23017_PORTA, 0xFF);

  io.digitalWrite(MCP23017_PORTB, 0xFF);
  degreeInt.fall(&degreeInteruptCallback);


  // init display
  display.writeByte(numbers[0]);
  display.writeByte(numbers[0]);
  display.pulseLatch();

  touchAInt.fall(&secondInterupt);

  touchA.init(&i2c1, &i2cMux, 0);

  if (!touchA.isConnected()) {
    // throw error to display
  }
  else {
    // throw error to display
  }
  touchA.calibrate();

  touchBInt.fall(&CAP1208_Interupt);

  touchB.init(&i2c1, &i2cMux, 1);

  if (!touchB.isConnected()) {
    // throw error to display
  }
  else {
    // throw error to display
  }

  touchB.calibrate();
  
  int touched = 0;
  int prevTouched = 0;

  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);

  while(1) {
    
    if (encoder.btnPressed()) {
      // boardLED.write(HIGH);
    } else {
      // boardLED.write(LOW);
    }

    if (degreeFlag) {
      io.digitalRead(MCP23017_PORTA);
      io.digitalWrite(MCP23017_PORTB, ~io.digitalRead(MCP23017_PORTB));
      degreeFlag = false;
    }

    if (interupt == 1) {
      if (numInterupts > 9) {
        numInterupts = 0;
      } else {
        numInterupts += 1;
      }
      
      display.writeByte(numbers[0]);
      display.writeByte(numbers[numInterupts]);
      display.pulseLatch();

      touchA.touched();
      touched = touchB.touched();
      if (touched != prevTouched) {
        for (int i=0; i<8; i++) {
          // if it *is* touched and *wasnt* touched before, alert!
          if (touchB.getBitStatus(touched, i) && !touchB.getBitStatus(prevTouched, i)) {
            ETL = false; // deactivate event triggering loop
            chEventList.createEvent(bClock.position, i);
          }
          // if it *was* touched and now *isnt*, alert!
          if (!touchB.getBitStatus(touched, i) && touchB.getBitStatus(prevTouched, i)) {
            chEventList.addEvent(bClock.position);
            ETL = true; // activate event triggering loop
          }
        }
        // reg.writeByte(touched); // toggle channel LEDs
        prevTouched = touched;
      }

      interupt = 0;
    }



    if (chEventList.hasEventInQueue() && ETL ) {
      chEventList.handleQueuedEvent(bClock.position);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop