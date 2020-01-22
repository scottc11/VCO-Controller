#ifndef __ROTARY_ENCODER_H
#define __ROTARY_ENCODER_H

#include "mbed.h"
#include "main.h"

#define ENCODER_PPR          24

class RotaryEncoder {
public:
  
  InterruptIn signalA;
  DigitalIn signalB;
  InterruptIn button;
  bool btnState;
  int position;

  RotaryEncoder(PinName chanA, PinName chanB, PinName btn) : signalA(chanA, PullUp), signalB(chanB, PullUp), button(btn, PullUp) {
    // do something
  }

  void init() {
    signalA.rise(callback(this, &RotaryEncoder::sigARise));
    button.fall(callback(this, &RotaryEncoder::btnPressCallback));
    button.rise(callback(this, &RotaryEncoder::btnReleasedCallback));
  }


  void sigARise() {
    // todo: add software debounce
    if (signalB.read() == 0) {
      // going clockwise
      position += 1;
    } else {
      // going counter-clockwise
      position -= 1;
    }
  }

  void btnPressCallback() {
    // todo: add software debounce
    btnState = true;
  }

  void btnReleasedCallback() {
    // todo: add software debounce
    btnState = false;
  }

  bool btnPressed() {
    return btnState;
  }

};


#endif