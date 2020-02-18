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

