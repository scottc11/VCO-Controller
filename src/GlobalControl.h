#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"
#include "TouchChannel.h"

class GlobalControl {
public:

  CAP1208 *cap;
  DigitalOut ledA;
  DigitalOut ledB;
  DigitalOut ledC;
  DigitalOut ledD;
  TouchChannel *channels[4];
  InterruptIn touchInterupt;
  int selectedChannel;          //   
  int currTouched;              // variable for holding the currently touched buttons
  int prevTouched;              // variable for holding previously touched buttons
  volatile bool touchDetected;

  GlobalControl(CAP1208 *cap_ptr, PinName _ledA, PinName _ledB, PinName _ledC, PinName _ledD, PinName cap_int ) : ledA(_ledA), ledB(_ledB), ledC(_ledC), ledD(_ledD), touchInterupt(cap_int, PullUp) {
    cap = cap_ptr;
    selectedChannel = 0;
    touchInterupt.fall(callback(this, &GlobalControl::handleTouchInterupt));
  }
  
  void init();
  void poll();
  void handleFreeze();
  void handleTouch(int pad);
  void handleRelease(int pad);
  void handleTouchEvent();
  void handleTouchInterupt() {
    touchDetected = true;
  }
private:
  enum PadNames {
    CTRL_FREEZE = 0,
    CTRL_ALT = 1,
    CTRL_A = 5,
    CTRL_B = 4,
    CTRL_C = 3,
    CTRL_D = 2,
  };
};


#endif