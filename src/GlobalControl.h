#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "BitwiseMethods.h"
#include "CAP1208.h"
#include "TouchChannel.h"
#include "DualDigitDisplay.h"
#include "RotaryEncoder.h"


class GlobalControl {
public:

  CAP1208 *touchCtrl;
  CAP1208 *touchOctAB;
  CAP1208 *touchOctCD;
  TouchChannel *channels[4];
  Metronome *metronome;
  InterruptIn ctrlInterupt;
  InterruptIn octaveInterupt;

  int selectedChannel;
  int currTouchedChannel;       // ???
  int currTouched;              // variable for holding the currently touched buttons
  int prevTouched;              // variable for holding previously touched buttons
  uint16_t currOctavesTouched;
  uint16_t prevOctavesTouched;
  volatile bool touchDetected;
  bool octaveTouchDetected;

  GlobalControl(
    CAP1208 *ctrl_ptr,
    CAP1208 *tchAB_ptr,
    CAP1208 *tchCD_ptr,
    PinName ctrl_int,
    PinName oct_int,
    TouchChannel *chanA_ptr,
    TouchChannel *chanB_ptr,
    TouchChannel *chanC_ptr,
    TouchChannel *chanD_ptr,
    Metronome *metronome_ptr ) : ctrlInterupt(ctrl_int, PullUp), octaveInterupt(oct_int) {
    
    touchCtrl = ctrl_ptr;
    touchOctAB = tchAB_ptr;
    touchOctCD = tchCD_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    metronome = metronome_ptr;
    ctrlInterupt.fall(callback(this, &GlobalControl::handleTouchInterupt));
    octaveInterupt.fall(callback(this, &GlobalControl::handleOctaveInterupt));
  }
  
  void init();
  void poll();
  void selectChannel(int channel);
  void setCalibration();
  void clearAllChannelEvents();
  void handleFreeze(bool enable);
  void handleClockReset();
  void handleClearLoop();
  void enableLoopLengthUI();
  void handleTouch(int pad);
  void handleRelease(int pad);
  void handleTouchEvent();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelLoopMultiplier(int pad);

  void handleTouchInterupt() {
    touchDetected = true;
  }

  void handleOctaveInterupt() {
    octaveTouchDetected = true;
  }

private:
  enum PadNames {
    RESET = 7,         // 0b10000000
    FREEZE = 6,        // 0b01000000
    LOOP_LENGTH = 0,   // 0b00000001
    RECORD = 1,        // 0b00000010
    CTRL_A = 5,        // 0b00100000
    CTRL_B = 4,        // 0b00010000
    CTRL_C = 3,        // 0b00001000
    CTRL_D = 2,        // 0b00000100
  };

  enum Gestures {
    _FREEZE = 0b01000000,
    CLEAR_LOOP = 0b10000010,      // REC + RESET
    CALIBRATE  = 0b00000000,      // REC + FREEZE + 3 seconds
    CLEAR_CH_A_LOOP = 0b10100010,
    CLEAR_CH_B_LOOP = 0b10010010,
    CLEAR_CH_C_LOOP = 0b10001010,
    CLEAR_CH_D_LOOP = 0b10000110,
  };

  enum Modes {
    DEFAULT,
    CALIBRATING
  };
};


#endif