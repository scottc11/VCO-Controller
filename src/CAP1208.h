#ifndef __CAP1208_H
#define __CAP1208_H

#include "main.h"
#include "TCA9544A.h"


#define CAP1208_MAIN_CONTROL 0x00

class CAP1208 {
  public:

  CAP1208() {
    address = 0x28 << 1;
    id = 0x6B;
  }
  


  I2C * i2c;
  TCA9544A * mux;
  int muxChannel;
  uint8_t address;
  uint8_t id;
  bool connected;
  char data_read[1];
  char data_write[2];

  void init(I2C *_i2c, TCA9544A *mux_ptr, int _muxChannel) {
    mux = mux_ptr;
    i2c = _i2c;
    
    mux->enableChan(0);

    // read product id to test connection
    data_write[0] = 0xFD;
    i2c->write(address, data_write, 1, true);
    i2c->read(address, data_read, 1);

    if (data_read[0] != id) {
      connected = false;
    } else {
      connected = true;
    }

    // speed up sampling time
    data_write[0] = 0x24; // AVERAGING AND SAMPLING CONFIGURATION REGISTER
    data_write[1] = 0b00000000;  // default: 0b00111001
    i2c->write(address, data_write, 2);

    // allow multiple touches
    data_write[0] = 0x2A;
    data_write[1] = 0x00;
    i2c->write(address, data_write, 2);

    // disable repeat rate for all channels
    data_write[0] = 0x28;
    data_write[1] = 0x00;
    i2c->write(address, data_write, 2);
    i2c->write(address, data_write, 1);

  }

  void disableInterupts() {
    // disable interupts
    data_write[0] = 0x27;
    data_write[1] = 0x00;
    i2c->write(address, data_write, 2);
    i2c->write(address, data_write, 1);
  }

  bool isConnected() {
    return connected;
  }

  void read(uint8_t reg) {
    data_write[0] = reg;
    i2c->write(address, data_write, 1, true);
    i2c->read(address, data_read, 1);
  }

  void getControlStatus() {
    mux->enableChan(0);
    // read main control status of CAP1208
    data_write[0] = 0x00; // main control
    i2c->write(address, data_write, 1, true);
    i2c->read(address, data_read, 1);
  }

  void getGeneralStatus() {
    mux->enableChan(0);
    // read general status of CAP1208
    data_write[0] = 0x02; // general status
    i2c->write(address, data_write, 1, true);
    i2c->read(address, data_read, 1);
  }

  void calibrate() {
    mux->enableChan(0);
    data_write[0] = 0x26; // CALIBRATION ACTIVATE AND STATUS REGISTER
    data_write[1] = 0xFF; // calibrate all inputs
    i2c->write(address, data_write, 2, true);
    i2c->read(address, data_read, 1);
  }

  uint8_t touched() {
    mux->enableChan(0);
    // for some reason we have to "clear" the INT bit everytime we read the sensors... 
    data_write[0] = CAP1208_MAIN_CONTROL;
    data_write[1] = CAP1208_MAIN_CONTROL & ~0x01;
    i2c->write(address, data_write, 2);
    
    // read input status of CAP1208
    data_write[0] = 0x03; // input status
    i2c->write(address, data_write, 1, true);
    i2c->read(address, data_read, 1);
    return data_read[0];
  }

  // bitNum starts at 0-7 for 8-bits
  // https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
  bool getBitStatus(int b, int bitNum) {
    return (b & (1 << bitNum));
  }

};


#endif