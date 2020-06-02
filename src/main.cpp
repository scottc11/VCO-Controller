#include "main.h"
#include "Metronome.h"
#include "TouchChannel.h"
#include "GlobalControl.h"
#include "Degrees.h"
#include "CAP1208.h"
#include "MIDI.h"
#include "DAC8554.h"
#include "TCA9548A.h"
#include "MCP23017.h"
#include "MCP4922.h"
#include "TLC59116.h"


int OCTAVE_LED_PINS_A[4] = { 0, 1, 2, 3 };     // via TLC59116
int OCTAVE_LED_PINS_B[4] = { 4, 5, 6, 7 };     // via TLC59116
int OCTAVE_LED_PINS_C[4] = { 8, 9, 10, 11 };   // via TLC59116
int OCTAVE_LED_PINS_D[4] = { 12, 13, 14, 15 }; // via TLC59116

I2C i2c1(I2C1_SDA, I2C1_SCL);
I2C i2c3(I2C3_SDA, I2C3_SCL);

Ticker ticker;
Timer timer;
MIDI midi(MIDI_TX, MIDI_RX);
Metronome metronome;
InterruptIn extClockInput(EXT_CLOCK_INPUT);

DAC8554 dac(SPI2_MOSI, SPI2_SCK, DAC_CS);

MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);

TLC59116 ledsA(&i2c3, TLC59116_CHAN_A_ADDR);
TLC59116 ledsB(&i2c3, TLC59116_CHAN_B_ADDR);
TLC59116 ledsC(&i2c3, TLC59116_CHAN_C_ADDR);
TLC59116 ledsD(&i2c3, TLC59116_CHAN_D_ADDR);
TLC59116 octaveLeds(&i2c3, TLC59116_OCT_LEDS_ADDR);

TCA9548A i2cMux(&i2c1, TCA9548A_ADDR);


CAP1208 touchA(&i2c1, &i2cMux, TCA9548A::CH6);
CAP1208 touchB(&i2c1, &i2cMux, TCA9548A::CH7);
CAP1208 touchC(&i2c1, &i2cMux, TCA9548A::CH1);
CAP1208 touchD(&i2c1, &i2cMux, TCA9548A::CH0);
CAP1208 touchCTRL(&i2c1, &i2cMux, TCA9548A::CH5);
CAP1208 touchOctAB(&i2c1, &i2cMux, TCA9548A::CH4);
CAP1208 touchOctCD(&i2c1, &i2cMux, TCA9548A::CH3);

Degrees degrees(DEGREES_INT, &io);

TouchChannel channelA(0, GATE_OUT_A, TOUCH_INT_A, CTRL_LED_A, ADC_A, &touchA, &ledsA, &octaveLeds, OCTAVE_LED_PINS_A, &degrees, &midi, &metronome, &dac, DAC8554::CHAN_A);
TouchChannel channelB(1, GATE_OUT_B, TOUCH_INT_B, CTRL_LED_B, ADC_B, &touchB, &ledsB, &octaveLeds, OCTAVE_LED_PINS_B, &degrees, &midi, &metronome, &dac, DAC8554::CHAN_B);
TouchChannel channelC(2, GATE_OUT_C, TOUCH_INT_C, CTRL_LED_C, ADC_C, &touchC, &ledsC, &octaveLeds, OCTAVE_LED_PINS_C, &degrees, &midi, &metronome, &dac, DAC8554::CHAN_C);
TouchChannel channelD(3, GATE_OUT_D, TOUCH_INT_D, CTRL_LED_D, ADC_D, &touchD, &ledsD, &octaveLeds, OCTAVE_LED_PINS_D, &degrees, &midi, &metronome, &dac, DAC8554::CHAN_D);

// GlobalControl globalCTRL(&touchCTRL, TOUCH_INT_CTRL, &channelA, &channelB, &channelC, &channelD, &metronome);

int newClockTimeStamp;
int lastClockTimeStamp;
int clockPeriod;

void tick() {
  metronome.tick();
  // channelA.tickClock();
  // channelB.tickClock();
  // channelC.tickClock();
  // channelD.tickClock();
}

void extTick() {
  // you need to advance every quarter note when an external clock signal is detected
  // additionally, set the ticker timer to a division of the input pulse duration / PPQN
  // the ticker will handle precise timing between quarter notes, and this interupt will advance each channels step

  lastClockTimeStamp = newClockTimeStamp;
  newClockTimeStamp = timer.read_us();
  clockPeriod = newClockTimeStamp - lastClockTimeStamp;
  metronome.step();
  // channelA.stepClock();
  // channelB.stepClock();
  // channelC.stepClock();
  // channelD.stepClock();

  ticker.attach_us(&tick, clockPeriod / PPQN);  // potentially write this as a flag and update in main loop
}


int main() {
  
  

  degrees.init();
  octaveLeds.initialize();

  channelA.init();
  channelB.init();
  channelC.init();
  channelD.init();

  // globalCTRL.init();


  // timer.start();
  // newClockTimeStamp = timer.read_us();
  // metronome.init();

  // ticker.attach_us(&tick, (1000000/2) / PPQN); //approx 120 bpm
  // extClockInput.rise(&extTick);

  while(1) {

    degrees.poll();
    channelA.poll();
    channelB.poll();
    channelC.poll();
    channelD.poll();

    // globalCTRL.poll();
    
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop