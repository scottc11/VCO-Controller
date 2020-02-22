#include "GlobalControl.h"


void GlobalControl::init() {
  cap->init();
  display.init(channels[selectedChannel]->numLoopSteps);
  encoder.init(0, 99);
  selectChannel(0);
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

void GlobalControl::selectChannel(int channel) {
  for (int i = 0; i < 4; i++) {
    if (i != channel) {
      channels[i]->ctrlLed.write(LOW);
      channels[i]->isSelected = false;
    }
  }
  
  selectedChannel = channel;
  channels[selectedChannel]->isSelected = true;
  channels[selectedChannel]->ctrlLed.write(HIGH);
  encoder.setValue(channels[selectedChannel]->numLoopSteps);
  display.write(encoder.value);
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
  
  switch (pad) {
    case CTRL_FREEZE:
      handleFreeze();
      break;
    case CTRL_ALT:
      break;
    case CTRL_A:
      selectChannel(0);
      break;
    case CTRL_B:
      selectChannel(1);
      break;
    case CTRL_C:
      selectChannel(2);
      break;
    case CTRL_D:
      selectChannel(3);
      break;
  }
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

