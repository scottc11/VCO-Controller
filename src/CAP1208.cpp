#include "CAP1208.h"


void CAP1208::init(I2C *_i2c, TCA9544A *mux_ptr, int _muxChannel) {
  mux = mux_ptr;
  muxChannel = _muxChannel;
  i2c = _i2c;
  

  mux->enableChan(muxChannel);

  // read product id to test connection
  if (i2cRead(PRODUCT_ID_REG) != CAP1208_PROD_ID) {
    connected = false;
  } else {
    connected = true;
  }

  // speed up sampling time
  data_write[0] = AVR_SMPL_CONF_REG; // AVERAGING AND SAMPLING CONFIGURATION REGISTER
  data_write[1] = 0b00000000;  // default: 0b00111001
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);

  // allow multiple touches
  data_write[0] = MULT_TOUCH_CONF_REG;
  data_write[1] = 0x00;
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);

  // enable interupts
  data_write[0] = INT_ENABLE_REG;
  data_write[1] = 0xFF;
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);

  // disable repeat rate for all channels
  data_write[0] = REPEAT_RATE_ENABLE_REG;
  data_write[1] = 0x00;
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);

  // disable BLK_PWR_CTRL power saving feature
  data_write[0] = CONF_2_REG;
  data_write[1] = 0x60;
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);

  clearInterupt();
}

void CAP1208::disableInterupts() {
  i2cWrite(INT_ENABLE_REG, 0x00);
}

bool CAP1208::isConnected() {
  return connected;
}

void CAP1208::read(uint8_t reg) {
  data_write[0] = reg;
  i2c->write(CAP1208_I2C_ADDR, data_write, 1, true);
  i2c->read(CAP1208_I2C_ADDR, data_read, 1);
}

uint8_t CAP1208::getControlStatus() {
  mux->enableChan(muxChannel);
  // read main control status of CAP1208
  data_write[0] = MAIN_CTRL_REG; // main control
  i2c->write(CAP1208_I2C_ADDR, data_write, 1, true);
  i2c->read(CAP1208_I2C_ADDR, data_read, 1);
  return data_read[0];
}

uint8_t CAP1208::getGeneralStatus() {
  mux->enableChan(muxChannel);
  
  data_write[0] = GENERAL_STATUS_REG; // general status
  i2c->write(CAP1208_I2C_ADDR, data_write, 1, true);
  i2c->read(CAP1208_I2C_ADDR, data_read, 1);
  return data_read[0];
}

void CAP1208::calibrate() {
  mux->enableChan(muxChannel);
  data_write[0] = CALIBRATE_REG;
  data_write[1] = 0xFF; // calibrate all inputs
  i2c->write(CAP1208_I2C_ADDR, data_write, 2, true);
  i2c->read(CAP1208_I2C_ADDR, data_read, 1);
}

void CAP1208::clearInterupt() {
  data_write[0] = MAIN_CTRL_REG;
  data_write[1] = MAIN_CTRL_REG & ~0x01;
  i2c->write(CAP1208_I2C_ADDR, data_write, 2);
}

uint8_t CAP1208::touched() {
  mux->enableChan(muxChannel);
  // for some reason we have to "clear" the INT bit everytime we read the sensors... 
  clearInterupt();
  
  // read input status of CAP1208
  data_write[0] = 0x03; // input status
  i2c->write(CAP1208_I2C_ADDR, data_write, 1, true);
  i2c->read(CAP1208_I2C_ADDR, data_read, 1);
  return data_read[0];
}

// bitNum starts at 0-7 for 8-bits
// https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
bool CAP1208::getBitStatus(int b, int bitNum) {
  return (b & (1 << bitNum));
}