#include "TLC59116.h"

void TLC59116::initialize() {
  writeRegister(MODE1, 0b00000000);
  writeRegister(MODE2, 0b00000000);
  
  writeRegister(GRPPWM, 0xFF);
  writeRegister(GRPFREQ, 0xFF);

  writeRegister(LEDOUT0, 0b11111111);
  writeRegister(LEDOUT1, 0b11111111);
  writeRegister(LEDOUT2, 0b11111111);
  writeRegister(LEDOUT3, 0b11111111);

  writeRegister(PWM0, 0x00);
  writeRegister(PWM1, 0x00);
  writeRegister(PWM2, 0x00);
  writeRegister(PWM3, 0x00);
  writeRegister(PWM4, 0x00);
  writeRegister(PWM5, 0x00);
  writeRegister(PWM6, 0x00);
  writeRegister(PWM7, 0x00);
}

void setMode1() {
  // 7 AI2 R
  // 6 AI1 R
  // 5 AI0 R
  // 4 OSC R/W
  // 3 SUB1 R/W
  // 2 SUB2 R/W
  // 1 SUB3 R/W
  // 0 ALLCALL R/W
  // default: 0001 0001
}