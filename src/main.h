#ifndef __MAIN_H
#define __MAIN_H

#include <mbed.h>

enum LedState: int {
  LOW = 0,
  HIGH = 1,
  BLINK = 2,
};

#define PPQN                 4

#define MIDI_BAUD            31250
#define MIDI_TX              PA_2
#define MIDI_RX              PA_3

#define I2C3_SDA             PC_9
#define I2C3_SCL             PA_8
#define I2C1_SDA             PB_9
#define I2C1_SCL             PB_8

#define SPI2_MOSI            PB_15
#define SPI2_MISO            PB_14
#define SPI2_SCK             PB_13

#define DAC_CS               PC_8

#define EXT_CLOCK_INPUT      PB_10

#define MODE_PIN_A           PA_9
#define MODE_PIN_B           PA_10
#define MODE_PIN_C           PA_11
#define MODE_PIN_D           PA_12

#define ADC_A                PA_6
#define ADC_B                PA_7
#define ADC_C                PC_4
#define ADC_D                PC_5

#define GATE_OUT_A           PC_2
#define GATE_OUT_B           PC_3
#define GATE_OUT_C           PA_0
#define GATE_OUT_D           PA_1

#define CTRL_LED_A           PB_7
#define CTRL_LED_B           PB_6
#define CTRL_LED_C           PB_5
#define CTRL_LED_D           PB_4

#define TOUCH_INT_A          PC_1
#define TOUCH_INT_B          PC_0
#define TOUCH_INT_C          PC_15
#define TOUCH_INT_D          PC_14
#define TOUCH_INT_CTRL       PC_13
#define TOUCH_INT_OCT        PC_7

#define DEGREES_INT          PB_2

#define TCA9548A_ADDR            0x70 // 1110000
#define CAP1208_ADDR             0x50 // 0010100  via mux
#define CAP1208_CTRL_ADDR        0x50 // 0010100

#define MCP23017_DEGREES_ADDR    0x20 // 0100000
#define TLC59116_CHAN_A_ADDR     0x60 // 1100000
#define TLC59116_CHAN_B_ADDR     0x61 // 1100001
#define TLC59116_CHAN_C_ADDR     0x62 // 1100010
#define TLC59116_CHAN_D_ADDR     0x64 // 1100100
#define TLC59116_OCT_LEDS_ADDR   0x68 // 1101000


// DEFAULT INITIALIZATION VALUES
#define DEGREE_COUNT                   8
#define DEFAULT_CHANNEL_LOOP_STEPS     8
#define EVENT_END_BUFFER               4
#define CV_QUANT_BUFFER                300
#define MAX_LOOP_STEPS                 32

// 83.333, 166.666, 249.999, 333.332, 416.66499999999996, 499.99799999999993, 583.3309999999999, 666.6639999999999, 749.9969999999998, 833.3299999999998, 916.6629999999998, 999.9959999999998
// 12-bit values => 83, 167, 250, 333, 417, 500, 583, 667, 750, 833, 917, 1000

// 16-bit values => 0,   1092.25, 2184.5, 3276.75, 4369, 5461.25, 6553.5, 7645.75, 8738, 9830.25, 10922.5, 12014.75, 13107
//                       1092     2184    3277     4369  5461     6553    7646     8738  9830     10922    12015     13107   1499
//                  B     C        C#      D        D#    E        F       F#       G     G#       A        A#        B       C

// 65535 / 12 = 5461.25 * 2 = 10922.5

// const int DAC_NOTE_MAP[8][3] = {
//   { 0, 83, 167 },
//   { 167, 250, 333 },
//   { 333, 417, 500 },
//   { 417, 500, 583 },
//   { 583, 667, 750 },
//   { 750, 833, 917 },
//   { 917, 1000, 1083 },
//   { 1000, 1083, 1083 }
// };

// const int DAC_NOTE_MAP[8][3] = {
//   { 0, 1092, 2184 },
//   { 2184, 3277, 4369 },
//   { 4369, 5461, 6553 },
//   { 5461, 6553, 7646 },
//   { 7646, 8738, 9830 },
//   { 9830, 10922, 12015 },
//   { 12015, 13107, 14199 },
//   { 13107, 14199, 15291 }
// };

const int DAC_NOTE_MAP[8][3] = {
  { 0, 1097, 2193 },
  { 2193, 3290, 4387 },
  { 4369, 5461, 6553 },
  { 5461, 6553, 7646 },
  { 7646, 8738, 9830 },
  { 9830, 10922, 12015 },
  { 12015, 13160, 14256 },
  { 13160, 14256, 15291 }
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

// const int DAC_OCTAVE_MAP[4] = {0, 13107, 26214, 39321 };
const int DAC_OCTAVE_MAP[4] = { 0, 13160, 26320, 39480 };
const int MIDI_OCTAVE_MAP[4] = { 36, 48, 60, 72 };

#endif
