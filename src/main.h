#ifndef __MAIN_H
#define __MAIN_H

#include <mbed.h>

#define LOW                  0
#define HIGH                 1
#define PPQ                  24
#define DISPLAY_CLK          PC_8
#define DISPLAY_DATA         PC_6
#define DISPLAY_LATCH        PC_5
#define LOOP_START_LED_PIN   PA_9
#define LOOP_STEP_LED_PIN    PA_8
#define ENCODER_CHAN_A       PB_4
#define ENCODER_CHAN_B       PB_5
#define ENCODER_PPR          20
#define EXT_CLOCK_INPUT      PB_10
#define SHIFT_REG_DATA       PB_13
#define SHIFT_REG_CLOCK      PB_14
#define SHIFT_REG_LATCH      PA_10
#define CHANNEL_GATE         PA_7
#define MIDI_TX              PA_11
#define MIDI_RX              PA_12
#define MIDI_BAUD            31250

#endif