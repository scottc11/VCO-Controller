#ifndef __CAP1208_H
#define __CAP1208_H

#include "main.h"
#include "TCA9544A.h"

#define CAP1208_I2C_ADDR     0x50
#define CAP1208_PROD_ID      0x6B

// supports I2C speeds to 400kHz

class CAP1208 {
  public:

  CAP1208(I2C *i2c_ptr) {
    useMux = false;
    i2c = i2c_ptr;
  }

  CAP1208(I2C *i2c_ptr, TCA9544A *mux_ptr, int _muxChannel) {
    useMux = true;
    i2c = i2c_ptr;
    mux = mux_ptr;
    muxChannel = _muxChannel;
  }

  I2C * i2c;
  TCA9544A * mux;
  int muxChannel;
  bool useMux;
  bool connected;
  char data_read[1];
  char data_write[2];

  void init();
  void disableInterupts();
  bool isConnected();
  void read(uint8_t reg);
  uint8_t getControlStatus();
  uint8_t getGeneralStatus();
  void calibrate();
  void clearInterupt();
  uint8_t touched();
  bool getBitStatus(int b, int bitNum);

private:

  void i2cWrite(char reg, char data) {
    
    if (useMux) { mux->enableChan(muxChannel); }
    
    char buffer[2];
    buffer[0] = reg;
    buffer[1] = data;
    i2c->write(CAP1208_ADDR, buffer, 2);
  }

  char i2cRead(char reg) {
    
    if (useMux) { mux->enableChan(muxChannel); }

    char command[1];
    char buffer[1];
    command[0] = reg;
    i2c->write(CAP1208_I2C_ADDR, command, 1, true);
    i2c->read(CAP1208_I2C_ADDR, buffer, 1);
    return buffer[0];
  }

  enum Registers {
    MAIN_CTRL_REG = 0x00,
    GENERAL_STATUS_REG = 0x02,     // read general status of CAP1208
    INPUT_STATUS_REG = 0x03,
    PRODUCT_ID_REG = 0xFD,
    AVR_SMPL_CONF_REG = 0x24,      // default: 0b00111001
    CALIBRATE_REG = 0x26,          // CALIBRATION ACTIVATE AND STATUS
    MULT_TOUCH_CONF_REG = 0x2A,
    INT_ENABLE_REG = 0x27,
    REPEAT_RATE_ENABLE_REG = 0x28,
    CONF_2_REG = 0x44,             // default: 0x40
  };
};


#endif