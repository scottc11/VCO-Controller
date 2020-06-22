#include "TouchChannel.h"
#include "PitchFrequencies.h"
// const int DAC_NOTE_MAP[8][3] = {
//   { 0, 1097, 2193 },
//   { 2193, 3290, 4387 },
//   { 4369, 5461, 6553 },
//   { 5461, 6553, 7646 },
//   { 7646, 8738, 9830 },
//   { 9830, 10922, 12015 },
//   { 12015, 13160, 14256 },
//   { 13160, 14256, 15291 }
// };


void TouchChannel::calibrateVCO() {
  
  float avgFreq;
  float threshold = 0.1;
  
  
  // wait till MAX_FREQ_SAMPLES samples has been obtained
  if (readyToCalibrate) {
    
    avgFreq = this->calculateAverageFreq();
    int currVal = dacVoltageValues[calibrationIndex];             // the value to be adjusted
    
    if (avgFreq > PITCH_FREQ[calibrationIndex] + threshold) {
      currVal -= 10;
      dacVoltageValues[calibrationIndex] = currVal;
    }
    
    else if (avgFreq < PITCH_FREQ[calibrationIndex] - threshold) {
      currVal += 10;
      dacVoltageValues[calibrationIndex] = currVal;
    }

    // output new voltage and reset calibration process
    dac->write(dacChannel, currVal);
    readyToCalibrate = false;
    freqSampleIndex = 0;
  }

  // if avgFreq is close enough to desired freq, break and move on to next pitch value
  if (avgFreq < PITCH_FREQ[calibrationIndex] + threshold && avgFreq > PITCH_FREQ[calibrationIndex] - threshold) {
    // increase note index by 1
    // reset sample counter to zero
    if (calibrationIndex > 10) {
      calibrationIndex = 0;
      calibrationFinished = true;
    } else {
      calibrationIndex += 1;
    }
    readyToCalibrate = false;
    freqSampleIndex = 0;
  }
}

float TouchChannel::calculateAverageFreq() {
  float sum = 0;
  for (int i = 0; i < MAX_FREQ_SAMPLES; i++) {
    sum += this->freqSamples[i];
  }
  return sum / MAX_FREQ_SAMPLES;
}

void TouchChannel::sampleVCOFrequency() {
  currVCOInputVal = slewCvInput.read_u16();

  // NEGATIVE
  if (currVCOInputVal >= (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal < (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && slopeIsPositive) {   // maybe plus threshold zero crossing
    // pulse.write(0);
    slopeIsPositive = false;
  }
    // POSITIVE
  else if (currVCOInputVal <= (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal > (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && !slopeIsPositive) {  // maybe negative threshold zero crossing
    // pulse.write(1);
    vcoPeriod = numSamplesTaken;     // how many samples have occurred between positive zero crossings
    vcoFrequency = 8000 / vcoPeriod; // sample rate divided by period of input signal
    freqSamples[freqSampleIndex] = vcoFrequency;
    numSamplesTaken = 0;
    
    if ( freqSampleIndex > MAX_FREQ_SAMPLES - 1 ) {
      readyToCalibrate = true;
      freqSampleIndex = 0;
    } else {
      freqSampleIndex += 1;
    }
    slopeIsPositive = true;
  }
  
  prevVCOInputVal = currVCOInputVal;
  numSamplesTaken++;
}