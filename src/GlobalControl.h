#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "BitwiseMethods.h"
#include "CAP1208.h"
#include "TouchChannel.h"
#include "VCOCalibrator.h"
#include "DualDigitDisplay.h"
#include "Metronome.h"


class GlobalControl {
public:
  enum Mode {
    DEFAULT,
    CALIBRATING
  };

  Metronome *metronome;
  VCOCalibrator calibrator;
  TouchChannel *channels[4];
  Timer timer;
  DigitalOut freezeLED;
  InterruptIn ctrlInterupt;
  uint32_t flashAddr = 0x08060000;   // should be 'sector 7', program memory address starts @ 0x08000000

  Mode mode;
  bool recordEnabled;                // used for toggling REC led among other things...
  int selectedChannel;

  GlobalControl(
      Metronome *metronome_ptr,
      PinName ctrl_int,
      PinName freezeLedPin,
      TouchChannel *chanA_ptr,
      TouchChannel *chanB_ptr,
      TouchChannel *chanC_ptr,
      TouchChannel *chanD_ptr) : ctrlInterupt(ctrl_int, PullUp), freezeLED(freezeLedPin)
  {
    mode = Mode::DEFAULT;
    metronome = metronome_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    ctrlInterupt.fall(callback(this, &GlobalControl::handleControlInterupt));
    freezeLED.write(1);
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
  void handleTouchEvent();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelLoopMultiplier(int pad);

  void tickChannels();

  void handleControlInterupt() {
    // touchDetected = true;
  }

private:
  enum PadNames
  {                  // integers correlate to 8-bit index position
    LOOP_LENGTH = 0, // 0b00000001
    PB_RANGE = 1,
    CALIBRATE = 3,   // 0b00001000
    RECORD = 5,      // 0b00100000
    CLEAR_SEQ = 6,   // 0b01000000
    CLEAR_BEND = 7,  // 0b10000000
    RESET = 9,       
    FREEZE = 10,     
    CTRL_ALL = 11,   // 0b00001000
    CTRL_A = 12,     // 0b00010000
    CTRL_B = 13,     // 0b00100000
    CTRL_C = 14,     // 0b01000000
    CTRL_D = 15,     // 0b10000000
  };

  enum Gestures
  {
    RESET_LOOP_A = 0b10100000,            // CHANNEL + RESET
    RESET_LOOP_B = 0b10010000,            // CHANNEL + RESET
    RESET_LOOP_C = 0b10001000,            // CHANNEL + RESET
    RESET_LOOP_D = 0b10000100,            // CHANNEL + RESET
    CLEAR_CH_A_LOOP   = 0b0001000001000000, // CLEAR_SEQ + CHANNEL
    CLEAR_CH_B_LOOP   = 0b0010000001000000,
    CLEAR_CH_C_LOOP   = 0b0100000001000000,
    CLEAR_CH_D_LOOP   = 0b1000000001000000,
    CLEAR_CH_A_PB     = 0b0001000010000000, // CLEAR_BEND + CHANNEL
    CLEAR_CH_B_PB     = 0b0010000010000000,
    CLEAR_CH_C_PB     = 0b0100000010000000,
    CLEAR_CH_D_PB     = 0b1000000010000000,
    CLEAR_SEQ_ALL     = 0b0000100001000000,
    RESET_CALIBRATION = 0b0000100000001000  // CTRL_ALL + CALIBRATE
  };
};


#endif