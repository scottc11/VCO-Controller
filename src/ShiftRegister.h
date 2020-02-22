#ifndef __SHIFT_REGISTER_H
#define __SHIFT_REGISTER_H

#include "main.h"

/*
tested with SN74xx164
*/

class ShiftRegister {
  public:
    ShiftRegister(PinName data_pin, PinName clock_pin, PinName latch_pin) : data(data_pin), clock(clock_pin), latch(latch_pin) {
      // do something
      latch.write(LOW);
      data.write(LOW);
      clock.write(LOW);
    }
    

    DigitalOut data;
    DigitalOut clock;
    DigitalOut latch;

    void writeByte(unsigned char byte) {
      for (int i = 0; i < 8; i++) {
        data = (byte & 0x01 << i) >> i;
        clock = 1;
        wait_us(1);
        clock = 0;
      }
    }

    void writeByte16(char16_t byte) {
      for (int i = 0; i < 16; i++) {
        data = (byte & 0x01 << i) >> i;
        clock = 1;
        wait_us(1);
        clock = 0;
      }
    }

    void pulseLatch() {
      latch = 1;
      wait_us(1);
      latch = 0;
    }

    void clear() {
      // reset pin low then high
    }

    //Writes a bit to the shift register UNTESTED
    void clearBit(int position){
      writeByte((0x00 & ~(1 << position)));
      pulseLatch();
    }

    void setBit(int position){
      writeByte((0x00 | (1 << position)));
      pulseLatch();
    }

    int getBit(char byte, int position) {
      return (byte >> position) & 1;
    }

};

/*
set --> set bit to 1
get --> get value of bit
clear --> set bit to 0
toggle --> flip bit
*/


#endif