#include "GlobalControl.h"


void GlobalControl::init() {
  cap->init();
  ledA.write(HIGH);
  wait_ms(50);
  ledA.write(LOW);
  ledB.write(HIGH);
  wait_ms(50);
  ledB.write(LOW);
  ledC.write(HIGH);
  wait_ms(50);
  ledC.write(LOW);
  ledD.write(HIGH);
  wait_ms(50);
  ledD.write(LOW);
  ledA.write(HIGH);
}


void GlobalControl::poll() {
  if (touchDetected) {
    handleTouchEvent();
    touchDetected = false;
  }
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
  ledA.write(LOW);
  ledB.write(LOW);
  ledC.write(LOW);
  ledD.write(LOW);
  switch (pad) {
    case CTRL_FREEZE:
      handleFreeze();
      break;
    case CTRL_ALT:
      break;
    case CTRL_A:
      selectedChannel = 0;
      ledA.write(HIGH);
      break;
    case CTRL_B:
      selectedChannel = 1;
      ledB.write(HIGH);
      break;
    case CTRL_C:
      selectedChannel = 2;
      ledC.write(HIGH);
      break;
    case CTRL_D:
      selectedChannel = 3;
      ledD.write(HIGH);
      break;
  }

}

void GlobalControl::handleRelease(int pad) {
  
}

void GlobalControl::handleFreeze() {
  if (currTouched & 0b00000001) {  // if no other pads being touched
    // freeze all channels
  }
}

