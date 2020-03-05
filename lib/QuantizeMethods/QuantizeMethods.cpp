
int quantize8th(int pos, int step, int numSteps, int ppqn) {
  
  int baseline = step * ppqn;
  int total = numSteps * ppqn;
  int value;
  if (pos < baseline + 6) {
    value = baseline;
  }
  else if (pos >= baseline + 6 && pos < baseline + 12) {
    value = baseline + 12;
  }
  else if (pos >= baseline + 12 && pos < baseline + 18) {
    value = baseline + 12;
  }
  else if (pos >= baseline + 18 && pos < baseline + 24) {
    value = baseline + 24; // note: 24 will actually be the 'start' of the next quarter note
  }
  else {
    value = baseline;
  }
  
  return value != total ? value : 0;
}