#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"
#include "TouchChannel.h"
#include "DualDigitDisplay.h"
#include "RotaryEncoder.h"


class GlobalControl {
public:

  CAP1208 *cap;
  TouchChannel *channels[4];
  Metronome *metronome;
  InterruptIn touchInterupt;
  DualDigitDisplay display;
  RotaryEncoder encoder;

  bool currBtnState;
  bool prevBtnState;
  int selectedChannel;
  int currTouchedChannel;
  int currTouched;              // variable for holding the currently touched buttons
  int prevTouched;              // variable for holding previously touched buttons
  volatile bool touchDetected;

  GlobalControl(CAP1208 *cap_ptr, PinName cap_int, PinName disData, PinName disClk, PinName disLatch, PinName encChanA, PinName encChanB, PinName encBtn, TouchChannel *chanA_ptr, TouchChannel *chanB_ptr, TouchChannel *chanC_ptr, TouchChannel *chanD_ptr, Metronome *metronome_ptr ) : touchInterupt(cap_int, PullUp), display(disData, disClk, disLatch), encoder(encChanA, encChanB, encBtn) {
    cap = cap_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    metronome = metronome_ptr;
    touchInterupt.fall(callback(this, &GlobalControl::handleTouchInterupt));
  }
  
  void init();
  void poll();
  void selectChannel(int channel);
  void handleClearAllEvents();
  void handleEncoderPressed();
  void handleFreeze(bool enable);
  void handleReset();
  void handleTouch(int pad);
  void handleRelease(int pad);
  void handleTouchEvent();
  void handleEncoderRotation();
  void handleTouchInterupt() {
    touchDetected = true;
  }
private:
  enum PadNames {
    CTRL_FREEZE = 0,
    CTRL_RESET = 1,
    CTRL_A = 5,
    CTRL_B = 4,
    CTRL_C = 3,
    CTRL_D = 2,
  };
};


#endif