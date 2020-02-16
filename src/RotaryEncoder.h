#ifndef __ROTARY_ENCODER_H
#define __ROTARY_ENCODER_H

#include "mbed.h"
#include "main.h"

#define ENCODER_PPR          24

class RotaryEncoder {
public:
  
  enum Direction {
    CLOCKWISE = 1,
    COUNTERCLOCKWISE = 0,
  };

  InterruptIn channelA;
  DigitalIn channelB;
  InterruptIn button;
  bool btnState;          // non-blocking state of encoder button
  int position;
  Direction direction;    // 0 or 1
  bool btnPressed;        // interupt flag
  bool btnReleased;       // interupt flag

  RotaryEncoder(PinName chanA, PinName chanB, PinName btn) : channelA(chanA, PullUp), channelB(chanB, PullUp), button(btn, PullUp) {
    // do something
  }

  void init() {
    channelA.fall(callback(this, &RotaryEncoder::sigAFall));
    // channelA.rise(callback(this, &RotaryEncoder::encode));
    button.fall(callback(this, &RotaryEncoder::btnPressCallback));
    button.rise(callback(this, &RotaryEncoder::btnReleaseCallback));
  }

  void poll() {
    if (btnPressed) {
      // do something
      btnState = true;
      wait_us(10);
      btnPressed = false;
    }
    if (btnReleased) {
      // do something
      btnState = false;
      wait_us(10);
      btnReleased = false;
    }
  }

  void sigAFall() {
    if (channelB.read() == 0) {
      // going counter-clockwise
      position -= 1;
      direction = COUNTERCLOCKWISE;
    } else {
      // going clockwise
      position += 1;
      direction = CLOCKWISE;
    }
    wait_us(10);
  }

  void btnPressCallback() {
    btnPressed = true;
  }

  void btnReleaseCallback() {
    btnReleased = true;
  }

  bool btnIsPressed() {
    return btnState;
  }

};

#endif