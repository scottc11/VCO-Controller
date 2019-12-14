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
    void clearBit(int position){
      writeByte((0x00 & ~(1 << position)));
    }

    void setBit(int position){
      writeByte((0x00 | (1 << position)));
    }

};

/*
set --> set bit to 1
get --> get value of bit
clear --> set bit to 0
toggle --> flip bit
*/


#endif