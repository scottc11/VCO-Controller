#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"

class GlobalControl {
public:

  CAP1208 touch;
  
  GlobalControl() {

  }
  
  void init(I2C *touchI2C_ptr);

};


#endif