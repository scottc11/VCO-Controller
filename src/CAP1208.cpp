#include "CAP1208.h"


void CAP1208::init() {
  
  // read product id to test connection
  if (i2cRead(PRODUCT_ID_REG) != CAP1208_PROD_ID) {
    connected = false;
  } else {
    connected = true;
  }

  i2cWrite(AVR_SMPL_CONF_REG, 0x00);        // speed up sampling time
  i2cWrite(MULT_TOUCH_CONF_REG, 0x00);      // allow multiple touches
  i2cWrite(INT_ENABLE_REG, 0xFF);           // enable interupts
  i2cWrite(REPEAT_RATE_ENABLE_REG, 0x00);   // disable repeat rate for all channels
  i2cWrite(CONF_2_REG, 0x60);               // disable BLK_PWR_CTRL power saving feature

  clearInterupt();
}

void CAP1208::disableInterupts() {
  i2cWrite(INT_ENABLE_REG, 0x00);
}

bool CAP1208::isConnected() {
  return connected;
}

uint8_t CAP1208::getControlStatus() {
  return i2cRead(MAIN_CTRL_REG);
}

uint8_t CAP1208::getGeneralStatus() {
  return i2cRead(GENERAL_STATUS_REG);
}

void CAP1208::calibrate() {
  i2cWrite(CALIBRATE_REG, 0xFF);
}

// for some reason we have to "clear" the INT bit everytime we read the sensors... 
void CAP1208::clearInterupt() {
  i2cWrite(MAIN_CTRL_REG, MAIN_CTRL_REG & ~0x01);
}

// read input status of CAP1208
uint8_t CAP1208::touched() {
  this->clearInterupt();
  uint8_t data = i2cRead(INPUT_STATUS_REG);
  return data;
}

// if it *is* touched and *wasnt* touched before, alert!
bool CAP1208::padIsTouched(int pad, int currTouched, int prevTouched) {
  return (getBitStatus(currTouched, pad) && !getBitStatus(prevTouched, pad));
}

// if it *was* touched and now *isnt*, alert!
bool CAP1208::padWasTouched(int pad, int currTouched, int prevTouched) {
  return (!getBitStatus(currTouched, pad) && getBitStatus(prevTouched, pad));
}

// bitNum starts at 0-7 for 8-bits
// https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
bool CAP1208::getBitStatus(int b, int bitNum) {
  return (b & (1 << bitNum));
}