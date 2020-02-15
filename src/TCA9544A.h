#ifndef __TCA9544A_H
#define __TCA9544A_H

#include "main.h"

#define TCA9544_DEFAULT_ADDR     0x70 // 1110000
// CHANNEL VALUES
#define TCA9544A_CH0 0b00000100
#define TCA9544A_CH1 0b00000101
#define TCA9544A_CH2 0b00000110
#define TCA9544A_CH3 0b00000111


class TCA9544A {
  public:
  
  TCA9544A(I2C *_i2c, uint8_t _address = TCA9544_DEFAULT_ADDR) {
    address = _address << 1;
    i2c = _i2c;
  };

  uint8_t address;
  int channel;
  I2C * i2c;
  char read_buffer[1];
  char write_buffer[1];

  void enableChan(int channel) {

    switch (channel) {
      case 0:
        write_buffer[0] = TCA9544A_CH0;
        i2c->write(address, write_buffer, 1);
        break;
      case 1:
        write_buffer[0] = TCA9544A_CH1;
        i2c->write(address, write_buffer, 1);
        break;
      case 2:
        write_buffer[0] = TCA9544A_CH2;
        i2c->write(address, write_buffer, 1);
        break;
      case 3:
        write_buffer[0] = TCA9544A_CH3;
        i2c->write(address, write_buffer, 1);
        break;
    }
  }

  char currentChan() {
    i2c->read(address, read_buffer, 1);
    return read_buffer[0];
  }
};



#endif