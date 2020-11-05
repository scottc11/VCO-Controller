#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "BitwiseMethods.h"
#include "CAP1208.h"
#include "TouchChannel.h"
#include "DualDigitDisplay.h"
#include "RotaryEncoder.h"
#include "Metronome.h"


class GlobalControl {
public:
  enum Mode {
    DEFAULT,
    CALIBRATING
  };

  Metronome *metronome;
  CAP1208 *touchCtrl1;
  CAP1208 *touchCtrl2;
  CAP1208 *touchOctAB;
  CAP1208 *touchOctCD;
  TouchChannel *channels[4];
  Timer timer;
  DigitalOut rec_led;
  InterruptIn ctrl1Interupt;
  InterruptIn ctrl2Interupt;
  InterruptIn octaveInteruptAB;
  InterruptIn octaveInteruptCD;
  FlashIAP flash;
  uint32_t flashAddr = 0x08060000;   // should be 'sector 7', program memory address starts @ 0x08000000

  Mode mode;
  bool recordEnabled;                // used for toggling REC led among other things...
  int selectedChannel;
  int currTouchedChannel;            // ???
  uint16_t currTouched;              // variable for holding the currently touched buttons. It is a combination of two 8-bit values from two CAP1208 ICs
  uint16_t prevTouched;              // variable for holding previously touched buttons
  uint16_t currOctavesTouched;
  uint16_t prevOctavesTouched;
  volatile bool touchDetected;
  bool octaveTouchDetected;

  GlobalControl(
      Metronome *metronome_ptr,
      CAP1208 * ctrl1_ptr,
      CAP1208 *ctrl2_ptr,
      CAP1208 *tchAB_ptr,
      CAP1208 *tchCD_ptr,
      PinName ctrl1_int,
      PinName ctrl2_int,
      PinName oct_int_ab,
      PinName oct_int_cd,
      PinName recLedPin,
      TouchChannel *chanA_ptr,
      TouchChannel *chanB_ptr,
      TouchChannel *chanC_ptr,
      TouchChannel *chanD_ptr) : ctrl1Interupt(ctrl1_int, PullUp), ctrl2Interupt(ctrl2_int, PullUp), octaveInteruptAB(oct_int_ab), octaveInteruptCD(oct_int_cd), rec_led(recLedPin)
  {
    mode = Mode::DEFAULT;
    metronome = metronome_ptr;
    touchCtrl1 = ctrl1_ptr;
    touchCtrl2 = ctrl2_ptr;
    touchOctAB = tchAB_ptr;
    touchOctCD = tchCD_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    rec_led.write(0);
    ctrl1Interupt.fall(callback(this, &GlobalControl::handleTouchInterupt));
    ctrl2Interupt.fall(callback(this, &GlobalControl::handleTouchInterupt));
    octaveInteruptAB.fall(callback(this, &GlobalControl::handleOctaveInterupt));
    octaveInteruptCD.fall(callback(this, &GlobalControl::handleOctaveInterupt));
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
  void handleTouch(int pad);
  void handleRelease(int pad);
  bool handleGesture();
  void handleTouchEvent();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelLoopMultiplier(int pad);

  void tickChannels();

  void handleTouchInterupt() {
    touchDetected = true;
  }

  void handleOctaveInterupt() {
    octaveTouchDetected = true;
  }

private:
  enum PadNames
  {                  // integers correlate to 8-bit index position
    RESET = 9,      // 0b10000000
    FREEZE = 10,     // 0b01000000
    LOOP_LENGTH = 0, // 0b00000001
    PB_RANGE = 1,
    CALIBRATE = 3,
    RECORD = 5,      // 0b00000010
    CLEAR_SEQ = 6,
    CLEAR_BEND = 7,
    CTRL_A = 12,     // 0b00010000
    CTRL_B = 13,     // 0b00100000
    CTRL_C = 14,     // 0b01000000
    CTRL_D = 15,     // 0b10000000
  };

  enum Gestures
  {
    RESET_LOOP_A = 0b10100000, // CHANNEL + RESET
    RESET_LOOP_B = 0b10010000, // CHANNEL + RESET
    RESET_LOOP_C = 0b10001000, // CHANNEL + RESET
    RESET_LOOP_D = 0b10000100, // CHANNEL + RESET
    CLEAR_CH_A_LOOP = 0b0001000001000000,  // CLEAR_SEQ + CHANNEL
    CLEAR_CH_B_LOOP = 0b0010000001000000,
    CLEAR_CH_C_LOOP = 0b0100000001000000,
    CLEAR_CH_D_LOOP = 0b1000000001000000,
  };
};


#endif