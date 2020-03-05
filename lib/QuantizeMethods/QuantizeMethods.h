#ifndef __QUANTIZE_METHODS_H
#define __QUANTIZE_METHODS_H

#include <iostream>

enum QuantizeMode {
  QUANT_NONE,
  QUANT_8,
  QUANT_16,
  QUANT_32,
  QUANT_64,
};

int quantize(QuantizeMode mode, int pos, int step, int numSteps, int ppqn);

#endif