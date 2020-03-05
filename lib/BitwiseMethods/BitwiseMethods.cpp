#include "BitwiseMethods.h"

int bitSet(int value, int bit) {
  return (value |= (1UL << bit));
}

int bitClear(int value, int bit) {
  return (value &= ~(1UL << bit));
}

int bitWrite(int byte, int bit, int value) {
  return (value ? bitSet(byte, bit) : bitClear(byte, bit));
}

int bitRead(int byte, int bit) {
  return (byte >> bit) & 0x01;
}