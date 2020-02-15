#include "main.h"
#include "BeatClock.h"
#include "TouchChannel.h"
#include "CAP1208.h"
#include "ShiftRegister.h"
#include "MIDI.h"
#include "TCA9544A.h"
#include "MCP23017.h"
#include "MCP4922.h"
#include "RotaryEncoder.h"


I2C i2c1(I2C_SDA, I2C_SCL);
I2C i2c3(I2C3_SDA, I2C3_SCL);

Ticker ticker;
Timer timer;
MIDI midi;
BeatClock CLOCK(LOOP_STEP_LED_PIN, LOOP_START_LED_PIN);
RotaryEncoder encoder(ENCODER_CHAN_A, ENCODER_CHAN_B, ENCODER_BTN);
ShiftRegister display(DISPLAY_DATA, DISPLAY_CLK, DISPLAY_LATCH);

InterruptIn extClockInput(EXT_CLOCK_INPUT);
InterruptIn degreeInt(DEGREES_INT);

MCP4922 dacA(SPI2_MOSI, SPI2_SCK, DAC_A_CS);
MCP4922 dacB(SPI2_MOSI, SPI2_SCK, DAC_B_CS);

MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);
MCP23017 ioA(&i2c3, MCP23017_CHAN_A_ADDR);
MCP23017 ioB(&i2c3, MCP23017_CHAN_B_ADDR);
MCP23017 ioC(&i2c3, MCP23017_CHAN_C_ADDR);
MCP23017 ioD(&i2c3, MCP23017_CHAN_D_ADDR);
TCA9544A i2cMux(&i2c1, TCA9544A_ADDR);

TouchChannel channelA(0, GATE_OUT_A, CHAN_INT_A, TOUCH_INT_A, &ioA, &midi, &CLOCK, &dacA, MCP4922::DAC_A);
TouchChannel channelB(1, GATE_OUT_B, CHAN_INT_B, TOUCH_INT_B, &ioB, &midi, &CLOCK, &dacA, MCP4922::DAC_B);
TouchChannel channelC(2, GATE_OUT_C, CHAN_INT_C, TOUCH_INT_C, &ioC, &midi, &CLOCK, &dacB, MCP4922::DAC_A);
TouchChannel channelD(3, GATE_OUT_D, CHAN_INT_D, TOUCH_INT_D, &ioD, &midi, &CLOCK, &dacB, MCP4922::DAC_B);


int newClockPeriod;
int oldClockPeriod;
int clockPeriod;

bool degreeFlag = false;

const char numbers[10] = { 0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b00111110, 0b11100000, 0b11111110, 0b11100110 };

void tick() {
  CLOCK.tick();
}

void extTick() {
  oldClockPeriod = newClockPeriod;
  newClockPeriod = timer.read_us();
  clockPeriod = newClockPeriod - oldClockPeriod;
  ticker.attach_us(&tick, clockPeriod / PPQ);  // potentially write this as a flag and update in main loop
}


void degreeInteruptCallback() {
  degreeFlag = true;
}

int main() {
  
  encoder.init();
  degreeInt.fall(&degreeInteruptCallback);

  // init display
  display.writeByte(numbers[9]);
  display.writeByte(numbers[9]);
  display.pulseLatch();

  timer.start();
  newClockPeriod = timer.read_us();
  CLOCK.init();

  ticker.attach_us(&tick, (1000000/2) / PPQ); //approx 120 bpm
  extClockInput.rise(&extTick);

  channelA.init(&i2c1, &i2cMux);
  channelB.init(&i2c1, &i2cMux);
  channelC.init(&i2c1, &i2cMux);
  channelD.init(&i2c1, &i2cMux);

  while(1) {
    
    channelA.poll();
    channelB.poll();
    channelC.poll();
    channelD.poll();
    
    if (encoder.btnPressed()) {
      // somthin
    } else {
      // boardLED.write(LOW);
    }
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop