#include "GlobalControl.h"


void GlobalControl::init() {
  cap->init();
  display.init();
  encoder.init(0, 99);
}


void GlobalControl::poll() {
  if (touchDetected) {
    handleTouchEvent();
    touchDetected = false;
  }
  if (encoder.getValue() != channels[selectedChannel]->numLoopSteps) {
    handleEncoderRotation();
  }
}

void GlobalControl::handleEncoderRotation() {
  switch (encoder.direction) {
    case RotaryEncoder::CLOCKWISE:
      channels[selectedChannel]->setNumLoopSteps(encoder.getValue());
      break;
    case RotaryEncoder::COUNTERCLOCKWISE:
      channels[selectedChannel]->setNumLoopSteps(encoder.getValue());
      break;
  }

  display.write(channels[selectedChannel]->numLoopSteps);
}

void GlobalControl::handleTouchEvent() {
  currTouched = cap->touched();
  if (currTouched != prevTouched) {
    for (int i=0; i<8; i++) {
      if (cap->padIsTouched(i, currTouched, prevTouched)) {
        handleTouch(i);
      }
      if (cap->padWasTouched(i, currTouched, prevTouched)) {

      }
    }
    prevTouched = currTouched;
  }
}

void GlobalControl::handleTouch(int pad) {
  channels[0]->ctrlLed.write(LOW);
  channels[1]->ctrlLed.write(LOW);
  channels[2]->ctrlLed.write(LOW);
  channels[3]->ctrlLed.write(LOW);
  
  switch (pad) {
    case CTRL_FREEZE:
      handleFreeze();
      break;
    case CTRL_ALT:
      break;
    case CTRL_A:
      selectedChannel = 0;
      break;
    case CTRL_B:
      selectedChannel = 1;
      break;
    case CTRL_C:
      selectedChannel = 2;
      break;
    case CTRL_D:
      selectedChannel = 3;
      break;
  }
  
  channels[selectedChannel]->ctrlLed.write(HIGH);
  encoder.setValue(channels[selectedChannel]->numLoopSteps);
  display.write(encoder.value);
}

void GlobalControl::handleRelease(int pad) {
  
}

void GlobalControl::handleFreeze() {
  if (currTouched & 0b00000001) {  // if no other pads being touched
    // freeze all channels
    channels[0]->freeze();
    channels[1]->freeze();
    channels[2]->freeze();
    channels[3]->freeze();
  }
}

