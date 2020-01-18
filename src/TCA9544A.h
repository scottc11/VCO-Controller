#ifndef __TCA9544A_H
#define __TCA9544A_H

#include "main.h"

#define PCA9544_DEFAULT_ADDR     0x70 // 1110000
// CHANNEL VALUES
#define CH0 0x01
#define CH1 0x02
#define CH2 0x04
#define CH3 0x08


class TCA9544A {

  TCA9544A(int _channel, uint8_t _address = PCA9544_DEFAULT_ADDR) {
    address = _address;
    channel = _channel;
  };

  uint8_t address;
  int channel;
  I2C * i2c;

};


#endif