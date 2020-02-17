#include "GlobalControl.h"


void GlobalControl::init(I2C *touchI2C_ptr) {
  touch.init(touchI2C_ptr);
  if (!touch.isConnected()) {
    this->updateLeds(0xFF);
    return;
  }
  touch.calibrate();
  touch.clearInterupt();
}

