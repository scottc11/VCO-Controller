#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "CAP1208.h"

class GlobalControl {
public:

  CAP1208 *cap;
  
  GlobalControl(CAP1208 *cap_ptr) {
    cap = cap_ptr;
  }
  
  void init();

};


#endif