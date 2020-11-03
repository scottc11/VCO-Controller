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
#include "AD525X.h"
#include "SX1509.h"

// read 08040000

FlashIAP flashStorage; // start address should equal 0x08060000 (ie. ADDR_FLASH_SECTOR_7)

int OCTAVE_LED_PINS_A[4] = { 0, 1, 2, 3 };     // via TLC59116
int OCTAVE_LED_PINS_B[4] = { 4, 5, 6, 7 };     // via TLC59116
int OCTAVE_LED_PINS_C[4] = { 8, 9, 10, 11 };   // via TLC59116
int OCTAVE_LED_PINS_D[4] = { 12, 13, 14, 15 }; // via TLC59116

I2C i2c1(I2C1_SDA, I2C1_SCL);
I2C i2c3(I2C3_SDA, I2C3_SCL);

Ticker ticker;
Timer timer;
MIDI midi(MIDI_TX, MIDI_RX);
InterruptIn extClockInput(EXT_CLOCK_INPUT);

AD525X digiPot(&i2c1);
DAC8554 dac1(SPI2_MOSI, SPI2_SCK, DAC1_CS);
DAC8554 dac2(SPI2_MOSI, SPI2_SCK, DAC2_CS);
MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);

SX1509 ioA(&i2c3, SX1509_CHAN_A_ADDR);
SX1509 ioB(&i2c3, SX1509_CHAN_B_ADDR);
SX1509 ioC(&i2c3, SX1509_CHAN_C_ADDR);
SX1509 ioD(&i2c3, SX1509_CHAN_D_ADDR);

TCA9548A i2cMux(&i2c1, TCA9548A_ADDR);

CAP1208 touchA(&i2c1, &i2cMux, TCA9548A::CH3);
CAP1208 touchB(&i2c1, &i2cMux, TCA9548A::CH2);
CAP1208 touchC(&i2c1, &i2cMux, TCA9548A::CH1);
CAP1208 touchD(&i2c1, &i2cMux, TCA9548A::CH0);
CAP1208 touchOctAB(&i2c1, &i2cMux, TCA9548A::CH4);
CAP1208 touchOctCD(&i2c1, &i2cMux, TCA9548A::CH5);
CAP1208 touchCTRL1(&i2c1, &i2cMux, TCA9548A::CH6);
CAP1208 touchCTRL2(&i2c1, &i2cMux, TCA9548A::CH7);

Degrees degrees(DEGREES_INT, &io);

TouchChannel channelA(0, &timer, &ticker, GATE_OUT_A, TOUCH_INT_A, IO_INT_PIN_A, ADC_A, PB_ADC_A, &touchA, &ioA, &degrees, &midi, &dac1, DAC8554::CHAN_A, &dac2, DAC8554::CHAN_A, &digiPot, AD525X::CHAN_A);
TouchChannel channelB(0, &timer, &ticker, GATE_OUT_B, TOUCH_INT_B, IO_INT_PIN_B, ADC_B, PB_ADC_B, &touchB, &ioB, &degrees, &midi, &dac1, DAC8554::CHAN_B, &dac2, DAC8554::CHAN_B, &digiPot, AD525X::CHAN_B);
TouchChannel channelC(0, &timer, &ticker, GATE_OUT_C, TOUCH_INT_C, IO_INT_PIN_C, ADC_C, PB_ADC_C, &touchC, &ioC, &degrees, &midi, &dac1, DAC8554::CHAN_C, &dac2, DAC8554::CHAN_C, &digiPot, AD525X::CHAN_C);
TouchChannel channelD(0, &timer, &ticker, GATE_OUT_D, TOUCH_INT_D, IO_INT_PIN_D, ADC_D, PB_ADC_D, &touchD, &ioD, &degrees, &midi, &dac1, DAC8554::CHAN_D, &dac2, DAC8554::CHAN_D, &digiPot, AD525X::CHAN_D);

GlobalControl globalCTRL(&touchCTRL1, &touchCTRL2, &touchOctAB, &touchOctCD, TOUCH_INT_CTRL_1, TOUCH_INT_CTRL_2, TOUCH_INT_OCT_AB, TOUCH_INT_OCT_CD, REC_LED, &channelA, &channelB, &channelC, &channelD);

Metronome metronome(TEMPO_LED, TEMPO_POT, INT_CLOCK_OUTPUT, PPQN, DEFAULT_CHANNEL_LOOP_STEPS);

int newClockTimeStamp;
int lastClockTimeStamp;
int clockPeriod;


void extTick() {
  // you need to advance every quarter note when an external clock signal is detected
  // additionally, set the ticker timer to a division of the input pulse duration / PPQN
  // the ticker will handle precise timing between quarter notes, and this interupt will advance each channels step

  // lastClockTimeStamp = newClockTimeStamp;
  // newClockTimeStamp = timer.read_us();
  // clockPeriod = newClockTimeStamp - lastClockTimeStamp;

  // channelA.stepClock();
  // channelB.stepClock();
  // channelC.stepClock();
  // channelD.stepClock();

  // channelA.tickClock();
  // channelB.tickClock();
  // channelC.tickClock();
  // channelD.tickClock();

  // ticker.attach_us(&tick, clockPeriod / PPQN);  // potentially write this as a flag and update in main loop
}


int main() {
  i2c1.frequency(400000);
  i2c3.frequency(400000);
  
  timer.start();

  metronome.init();

  degrees.init();

  channelA.init();
  channelB.init();
  channelC.init();
  channelD.init();

  globalCTRL.init();

  extClockInput.rise(&extTick);

  while(1) {

    if (globalCTRL.mode == GlobalControl::CALIBRATING) {
      if (!globalCTRL.channels[globalCTRL.selectedChannel]->calibrationFinished) {
        globalCTRL.channels[globalCTRL.selectedChannel]->calibrateVCO();
      } else {
        globalCTRL.mode = GlobalControl::DEFAULT;
      }
    } else {
      metronome.poll();
      globalCTRL.poll();
      degrees.poll();
      channelA.poll();
      channelB.poll();
      channelC.poll();
      channelD.poll();
    }
    
    
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop