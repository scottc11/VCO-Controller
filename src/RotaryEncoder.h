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

  enum ButtonState {
    PRESSED,
    RELEASED
  };

  InterruptIn channelA;
  DigitalIn channelB;
  InterruptIn button;
  Direction direction;             // 0 or 1
  Callback<void()> _function;      // 
  bool btnState;                   // non-blocking state of encoder button
  int position;
  bool btnPressed;                 // interupt flag
  bool btnReleased;                // interupt flag
  int maxValue;
  int minValue;
  int value;

  RotaryEncoder(PinName chanA, PinName chanB, PinName btn) : channelA(chanA, PullUp), channelB(chanB, PullUp), button(btn, PullUp) {
    position = 0;
  }

  void init(int min = 0, int max = 255) {
    maxValue = max;
    minValue = min;
    value = minValue;

    channelA.fall(callback(this, &RotaryEncoder::sigAFall));
    // channelA.rise(callback(this, &RotaryEncoder::encode));
    button.fall(callback(this, &RotaryEncoder::btnPressCallback));
    button.rise(callback(this, &RotaryEncoder::btnReleaseCallback));
  }

  /**
   * to attach a function     -->   attachBtnCallback(&myFunc)
   * to attach a class method -->   attachBtnCallback(callback(this, &MyClass::myClassMethod))
  */ 
  void attachBtnCallback(Callback<void()> func) {
    _function = func;
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
      if (value > minValue) {
        value -= 1;
      }
      direction = COUNTERCLOCKWISE;
    } else {
      // going clockwise
      position += 1;
      if (value < maxValue) {
        value += 1;
      }
      direction = CLOCKWISE;
    }
    wait_us(5);
  }

  void btnPressCallback() {
    _function();
  }

  void btnReleaseCallback() {
    _function();
  }

  bool btnIsPressed() {
    return btnState;
  }

  void setValue(int val) {
    value = val;
  }

  int getValue() {
    return value;
  }

};

#endif