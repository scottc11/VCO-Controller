#ifndef __DUAL_DIG_DISPLAY_H
#define __DUAL_DIG_DISPLAY_H

#include "main.h"
#include "ShiftRegister.h"

class DualDigDisplay {
  public:
  DualDigDisplay() {
    // initialize
  }

  ShiftRegister display;

};


#endif