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

#define EXT_CLOCK_INPUT      PB_10

#define ADC_A                PA_6
#define ADC_B                PA_7
#define ADC_C                PC_5
#define ADC_D                PC_4

#define GATE_OUT_A           PC_2
#define GATE_OUT_B           PC_3
#define GATE_OUT_C           PC_7
#define GATE_OUT_D           PC_6

#define TOUCH_INT_A          PC_1
#define TOUCH_INT_B          PC_0
#define TOUCH_INT_C          PC_15
#define TOUCH_INT_D          PC_14
#define TOUCH_INT_CTRL_1     PC_13
#define TOUCH_INT_CTRL_2     PB_7
#define TOUCH_INT_OCT_AB     PB_6
#define TOUCH_INT_OCT_CD     PB_5

#define DEGREES_INT          PB_4

#define DAC1_CS              PB_12
#define DAC2_CS              PC_8

#define TCA9548A_ADDR            0x70 // 1110000
#define CAP1208_ADDR             0x50 // 0010100  via mux
#define CAP1208_CTRL_ADDR        0x50 // 0010100

#define MCP23017_DEGREES_ADDR    0x20 // 0100000

#define SX1509_CHAN_A_ADDR       0x3E
#define SX1509_CHAN_B_ADDR       0x70
#define SX1509_CHAN_C_ADDR       0x3F
#define SX1509_CHAN_D_ADDR       0x71


// DEFAULT INITIALIZATION VALUES
#define DEGREE_COUNT                   8
#define OCTAVE_COUNT                   4
#define DEFAULT_CHANNEL_LOOP_STEPS     8
#define EVENT_END_BUFFER               4
#define CV_QUANT_BUFFER                1000
#define SLEW_CV_BUFFER                 1000
#define MAX_LOOP_STEPS                 32

#define DEFAULT_VOLTAGE_ADJMNT      200
#define MAX_CALIB_ATTEMPTS          20
#define MAX_FREQ_SAMPLES            25    // how many frequency calculations we want to use to obtain our average frequency prediction of the input. The higher the number, the more accurate the result
#define VCO_SAMPLE_RATE_US          125     // 8000hz is equal to 125us (microseconds)
#define VCO_ZERO_CROSSING           32767   // ADC range is 0v - 3.3v, so the midpoint of the sine wave should be 1.65v (ie. 65535 / 2 32767)
#define VCO_ZERO_CROSS_THRESHOLD    500     // for handling hysterisis at zero crossing point

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
  { 4387, 5461, 6553 },
  { 5461, 6553, 7646 },
  { 7646, 8738, 9830 },
  { 9830, 10922, 12015 },
  { 12015, 13160, 14256 },
  { 13160, 14256, 15291 }
};

// const int DAC_VOLTAGE_VALUES[59] = {
// // A     A#     B      C      C#     D      D#     E      F      F#     G      G#
//   1097,  2193,  3290,  4387,  5461,  6553,  7646,  8738,  9830,  10922, 12015, 13160,
//   14256, 15353, 16450, 17546, 18643, 19739, 20836, 21933, 23029, 24126, 25223, 26319,
//   27416, 28513, 29609, 30706, 31802, 32899, 33996, 35092, 36189, 37286, 38382, 39479, 
//   40576, 41672, 42769, 43865, 44962, 46059, 47155, 48252, 49349, 50445, 51542, 52639, 
//   53735, 54832, 55928, 57025, 58122, 59218, 60315, 61412, 62508, 63605, 64702
// //                             END
// };

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
const int DAC_OCTAVE_MAP[4] = { 0, 8, 16, 24 };
const int MIDI_OCTAVE_MAP[4] = { 36, 48, 60, 72 };

#define CALIBRATION_LENGTH   60

const int CALIBRATION_LED_MAP[CALIBRATION_LENGTH] = {
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
};

const int DAC_VOLTAGE_VALUES[CALIBRATION_LENGTH] = {
// A     A#     B      C      C#     D      D#     E      F      F#     G      G#
  5630,  6568,  7506,   8445,  9383,  10321, 11260, 12198, 13137, 14075, 15013, 15952,
  16890, 17828, 18767,  19705, 20643, 21582, 22520, 23458, 24397, 25335, 26274, 27212,
  28150, 29089, 30027,  30965, 31904, 32842, 33780, 34719, 35657, 36596, 37534, 38472,
  39411, 40349, 41287,  42226, 43164, 44102, 45041, 45979, 46917, 47856, 48794, 49733,
  50671, 51609, 52548,  53486, 54424, 55363, 56301, 57239, 58178, 59116, 60054, 60993
};

#endif
