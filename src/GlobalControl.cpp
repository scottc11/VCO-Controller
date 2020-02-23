#include "GlobalControl.h"

/**
 * TODO:
 * clear button for loop mode
 * reset button for loop mode
 * freeze button
*/

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


/**
 * CHANNEL SELECT
*/
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


/**
 * HANDLE ENCODER ROTATION
*/
void GlobalControl::handleEncoderRotation() {
  int value = encoder.getValue();
  channels[selectedChannel]->setNumLoopSteps(value);
  display.write(channels[selectedChannel]->numLoopSteps);
}

/**
 * HANDLE TOUCH EVENT
*/
void GlobalControl::handleTouchEvent() {
  currTouched = cap->touched();
  if (currTouched != prevTouched) {
    for (int i=0; i<8; i++) {
      if (cap->padIsTouched(i, currTouched, prevTouched)) {
        handleTouch(i);
      }
      if (cap->padWasTouched(i, currTouched, prevTouched)) {
        handleRelease(i);
      }
    }
    prevTouched = currTouched;
  }
}

/**
 * HANDLE TOUCH TOUCHED
 * 
*/
void GlobalControl::handleTouch(int pad) {
  
  switch (pad) {
    case CTRL_FREEZE:
      handleFreeze(true);
      break;
    case CTRL_RESET:
      handleReset();
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

/**
 * HANDLE TOUCH RELEASE
*/
void GlobalControl::handleRelease(int pad) {
  switch (pad) {
    case CTRL_FREEZE:
      handleFreeze(false);
      break;
    case CTRL_RESET:
      break;
    case CTRL_A:
      break;
    case CTRL_B:
      break;
    case CTRL_C:
      break;
    case CTRL_D:
      break;
  }
}


/**
 * HANDLE FREEZE
*/
void GlobalControl::handleFreeze(bool enable) {
  // freeze all channels
  channels[0]->freeze(enable);
  channels[1]->freeze(enable);
  channels[2]->freeze(enable);
  channels[3]->freeze(enable);
}


/**
 * HANDLE RESET
*/
void GlobalControl::handleReset() {
  // reset all channels
  channels[0]->reset();
  channels[1]->reset();
  channels[2]->reset();
  channels[3]->reset();
}

