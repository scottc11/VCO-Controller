#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"

class GlobalControl {
public:

  // CAP1208 touch;
  
  GlobalControl(I2C *touchI2C_ptr) {

  }
  
  void init();

};


#endif