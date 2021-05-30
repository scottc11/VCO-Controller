#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "BitwiseMethods.h"
#include "TouchChannel.h"
#include "VCOCalibrator.h"
#include "DualDigitDisplay.h"
#include "Metronome.h"
#include "MCP23017.h"
#include "MCP23008.h"

class GlobalControl {
public:
  enum Mode {
    DEFAULT,
    CALIBRATING
  };

  EventQueue *eventQueue;
  MCP23017 io;
  MCP23008 leds;
  Metronome *metronome;
  VCOCalibrator calibrator;
  TouchChannel *channels[4];
  Timer timer;
  InterruptIn ctrlInterupt;
  DigitalOut freezeLED;
  DigitalOut recLED;
  uint32_t flashAddr = 0x08060000;   // should be 'sector 7', program memory address starts @ 0x08000000

  Mode mode;
  bool recordEnabled;                // used for toggling REC led among other things...
  int selectedChannel;
  bool buttonPressed;
  uint16_t buttonsState;

  GlobalControl(
      EventQueue *queue_ptr,
      Metronome *metronome_ptr,
      I2C *i2c_ptr,
      TouchChannel *chanA_ptr,
      TouchChannel *chanB_ptr,
      TouchChannel *chanC_ptr,
      TouchChannel *chanD_ptr
      ) : io(i2c_ptr, MCP23017_CTRL_ADDR), leds(i2c_ptr, MCP23008_IO_ADDR), ctrlInterupt(CTRL_INT), freezeLED(FREEZE_LED), recLED(REC_LED)
  {
    mode = Mode::DEFAULT;
    eventQueue = queue_ptr;
    metronome = metronome_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    ctrlInterupt.fall(callback(this, &GlobalControl::handleControlInterupt));
    freezeLED.write(0);
    recLED.write(0);
  }

  void init();
  void poll();
  void selectChannel(int channel);
  void clearAllChannelEvents();
  void calibrateChannel(int chan);
  void saveCalibrationToFlash(bool reset=false);
  void loadCalibrationDataFromFlash();

  void handleFreeze(bool enable);
  void handleClockReset();
  void enableLoopLengthUI();
  void enablePitchBendRangeUI();
  void disablePitchBendRangeUI();

  void handleTouch(int pad);
  void handleRelease(int pad);
  bool handleGesture();
  void handleButtonPress();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelLoopMultiplier(int pad);

  void tickChannels();

  void handleControlInterupt() {
    buttonPressed = true;
  }

private:
  enum PadNames
  {                  // integers correlate to 8-bit index position
    SEQ_LENGTH = 0xFFBF,
    PB_RANGE =   0xFFDF,
    RECORD =     0xEFFF,
    CLEAR_SEQ =  0xBFFF,
    CLEAR_BEND = 0xDFFF,
    BEND_MODE =  0x7FFF,
    RESET =      0xFEFF,
    FREEZE =     0xFBFF,
    QUANTIZE_SEQ = 0xFDFF,
    QUANTIZE_AMOUNT = 0xFFEF,
    SHIFT =      0xFF7F,
    CTRL_A =     0xFFF7,
    CTRL_B =     0xFFFB,
    CTRL_C =     0xFFFD,
    CTRL_D =     0xFFFE
  };

  enum Gestures
  {
    CALIBRATE_A = 0xFF77,
    CALIBRATE_B = 0xFF7B,
    CALIBRATE_C = 0xFF7D,
    CALIBRATE_D = 0xFF7E,
    RESET_LOOP_A = 0b10100000,            // CHANNEL + RESET
    RESET_LOOP_B = 0b10010000,            // CHANNEL + RESET
    RESET_LOOP_C = 0b10001000,            // CHANNEL + RESET
    RESET_LOOP_D = 0b10000100,            // CHANNEL + RESET
    CLEAR_CH_A_LOOP = 0b0001000001000000, // CLEAR_SEQ + CHANNEL
    CLEAR_CH_B_LOOP = 0b0010000001000000,
    CLEAR_CH_C_LOOP = 0b0100000001000000,
    CLEAR_CH_D_LOOP = 0b1000000001000000,
    CLEAR_CH_A_PB = 0b0001000010000000, // CLEAR_BEND + CHANNEL
    CLEAR_CH_B_PB = 0b0010000010000000,
    CLEAR_CH_C_PB = 0b0100000010000000,
    CLEAR_CH_D_PB = 0b1000000010000000,
    CLEAR_SEQ_ALL = 0b0000100001000000,
    RESET_CALIBRATION = 0b0000100000001000 // CTRL_ALL + CALIBRATE
  };
};


#endif