#include "main.h"
#include "BeatClock.h"
#include "ChannelEventList.h"
#include "CAP1208.h"
#include "ShiftRegister.h"
#include "MIDI.h"
#include "TCA9544A.h"
#include "MCP23017.h"


I2C i2c1(I2C_SDA, I2C_SCL);
I2C i2c3(PC_9, PA_8);
DigitalOut boardLED(LED1);
Ticker ticker;
Timer timer;
InterruptIn extClockInput(EXT_CLOCK_INPUT);
InterruptIn touchAInt(PB_4, PullUp);
InterruptIn touchBInt(PB_5, PullUp);
InterruptIn degreeInt(PC_12);

MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);
MIDI midi;
ShiftRegister reg(SHIFT_REG_DATA, SHIFT_REG_CLOCK, SHIFT_REG_LATCH);
ShiftRegister display(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LATCH);
DigitalOut latch(DISPLAY_LATCH);
CAP1208 touchA;
CAP1208 touchB;
TCA9544A i2cMux(&i2c1, TCA9544A_ADDR);
BeatClock bClock(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
ChannelEventList chEventList(CHANNEL_GATE, &reg, &midi);

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
  boardLED.write(HIGH);

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

  if (!touchA.isConnected()) { boardLED.write(HIGH); }
  else { boardLED.write(LOW); }
  touchA.calibrate();

  touchBInt.fall(&CAP1208_Interupt);

  touchB.init(&i2c1, &i2cMux, 1);

  if (!touchB.isConnected()) { boardLED.write(HIGH); }
  else { boardLED.write(LOW); }
  touchB.calibrate();
  

  int touched = 0;
  int prevTouched = 0;

  timer.start();
  newClockPeriod = timer.read_us();
  bClock.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);

  while(1) {
    
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
        reg.writeByte(touched); // toggle channel LEDs
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