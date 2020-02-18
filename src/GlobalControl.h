#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"

class GlobalControl {
public:

  CAP1208 *cap;
  DigitalOut ledA;
  DigitalOut ledB;
  DigitalOut ledC;
  DigitalOut ledD;
  GlobalControl(CAP1208 *cap_ptr, PinName _ledA, PinName _ledB, PinName _ledC, PinName _ledD ) : ledA(_ledA), ledB(_ledB), ledC(_ledC), ledD(_ledD) {
    cap = cap_ptr;
  }
  
  void init();

};


#endif