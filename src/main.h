#ifndef __MAIN_H
#define __MAIN_H

#include <mbed.h>

#define LOW                  0
#define HIGH                 1
#define PPQ                  24

#define MIDI_BAUD            31250
#define MIDI_TX              PA_2
#define MIDI_RX              PA_3

#define I2C3_SDA             PC_9
#define I2C3_SCL             PA_8

#define SPI2_MOSI            PB_15
#define SPI2_MISO            PB_14
#define SPI2_SCK             PB_13

#define DAC_A_CS             PB_12
#define DAC_B_CS             PC_7

#define DISPLAY_CLK          PB_2
#define DISPLAY_DATA         PB_10
#define DISPLAY_LATCH        PB_1

#define ENCODER_CHAN_A       PA_1
#define ENCODER_CHAN_B       PA_7
#define ENCODER_BTN          PA_0


#define ADC_A                PA_4
#define ADC_B                PA_5
#define ADC_C                PA_6
#define ADC_D                PB_0

#define GATE_OUT_A           PA_9
#define GATE_OUT_B           PA_10
#define GATE_OUT_C           PA_11
#define GATE_OUT_D           PA_12

#define CTRL_LED_A           PC_3
#define CTRL_LED_B           PC_2
#define CTRL_LED_C           PC_1
#define CTRL_LED_D           PC_0

#define LOOP_START_LED_PIN   PC_4
#define LOOP_STEP_LED_PIN    PC_5
#define EXT_CLOCK_INPUT      PC_6

#define CHAN_INT_A           PC_11
#define CHAN_INT_B           PC_10
#define CHAN_INT_C           PA_15
#define CHAN_INT_D           PC_8

#define DEGREES_INT          PC_12
#define TOUCH_INT_CTRL       PC_13
#define TOUCH_INT_A          PB_7
#define TOUCH_INT_B          PB_6
#define TOUCH_INT_C          PB_5
#define TOUCH_INT_D          PB_4

#define TCA9544A_ADDR          0x70 // 1110000
#define CAP1208_ADDR           0x50 // 0010100  via mux
#define CAP1208_CTRL_ADDR      0x50 // 0010100

#define MCP23017_DEGREES_ADDR  0x20 // 0100000
#define MCP23017_CHAN_A_ADDR  0x21 // 0100001
#define MCP23017_CHAN_B_ADDR  0x22 // 0100010
#define MCP23017_CHAN_C_ADDR  0x23 // 0100011
#define MCP23017_CHAN_D_ADDR  0x24 // 0100100

// 83.333, 166.666, 249.999, 333.332, 416.66499999999996, 499.99799999999993, 583.3309999999999, 666.6639999999999, 749.9969999999998, 833.3299999999998, 916.6629999999998, 999.9959999999998
// 83, 167, 250, 333, 417, 500, 583, 667, 750, 833, 917, 1000

const int DAC_NOTE_MAP[8][3] = {
  { 0, 83, 167 },
  { 167, 250, 333 },
  { 333, 417, 500 },
  { 417, 500, 583 },
  { 583, 667, 750 },
  { 750, 833, 917 },
  { 917, 1000, 1083 },
  { 1000, 1083, 1083 }
};

const int MIDI_NOTE_MAP[8][3] = {
  { 11, 12, 13 },
  { 13, 14, 15 },
  { 15, 16, 17 },
  { 16, 17, 18 },
  { 18, 19, 20 },
  { 20, 21, 22 },
  { 22, 23, 24 },
  { 23, 24, 25 },
};

const int DAC_OCTAVE_MAP[4] = {0, 1000, 2000, 3000 };
const int MIDI_OCTAVE_MAP[4] = { 36, 48, 60, 72 };

const int CV_INPUT_MAP[8] = { 2048, 4096, 6144, 8192, 10240, 12288, 14366, 16384 };
const int CV_OCTAVES[4] = { 16384, 32768, 49152, 65536 };
#endif