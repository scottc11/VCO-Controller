#include "QuantizeMethods.h"

// NOTE: You could probable change this function from using cases to just one long trickling set of if statements, 
// where certain if statements only gets executed for certain quantize modes

int quantize(QuantizeMode mode, int pos, int step, int numSteps, int ppqn) {
  
  int baseline = step * ppqn; // could potentially write (step * ppqn) - ppqn, and avoid having steps start at 0
  int total = numSteps * ppqn;
  int value;
  
  switch (mode) {
    case QUANT_NONE:
      value = pos;
      break;
    
    case QUANT_8:
      if (pos < baseline + 6) {
        value = baseline;
      }
      else if (pos >= baseline + 6 && pos < baseline + 18) {
        value = baseline + 12;
      }
      else if (pos >= baseline + 18 && pos < baseline + 24) {
        value = baseline + 24; // note: 24 will actually be the 'start' of the next quarter note
      }
      break;

    case QUANT_16:
      if (pos < baseline + 3) {
        value = baseline;
      }
      else if (pos >= baseline + 3 && pos < baseline + 9) {
        value = baseline + 6;
      }
      else if (pos >= baseline + 9 && pos < baseline + 15) {
        value = baseline + 12;
      }
      else if (pos >= baseline + 15 && pos < baseline + 21) {
        value = baseline + 18;
      }
      else if (pos >= baseline + 21 && pos < baseline + 24) {
        value = baseline + 24; // note: 24 will actually be the 'start' of the next quarter note
      }
      break;
    
    default:
      value = pos;
      break;
  }
  
  return value != total ? value : 0;
}
