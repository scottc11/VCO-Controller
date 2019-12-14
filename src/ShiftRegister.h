#ifndef __SHIFT_REGISTER_H
#define __SHIFT_REGISTER_H

#include "main.h"

/*
tested with SN74xx164
*/

class ShiftRegister {
  public:
    ShiftRegister(PinName data_pin, PinName clock_pin) : data(data_pin), clock(clock_pin) {
      // do something
    }

    DigitalOut data;
    DigitalOut clock;

    void writeByte(unsigned char byte) {
      for (int i = 0; i < 8; i++) {
        data = (byte & 0x01<<i)>>i;
        clock = 1;
        wait_us(2);
        clock = 0;
      }
    }

    //Writes a bit to the shift register UNTESTED
    void writeBit(unsigned char bit){
      data = bit & 0x01;
      clock = 1;
      wait_us(2);
      clock = 0;
      wait_us(2);
    }

};

#endif