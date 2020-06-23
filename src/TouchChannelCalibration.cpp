#include "TouchChannel.h"
#include "PitchFrequencies.h"

void TouchChannel::calibrateVCO() {
  
  float avgFreq = this->calculateAverageFreq();
  float threshold = 0.1;
  
  
  // wait till MAX_FREQ_SAMPLES samples has been obtained
  if (readyToCalibrate) {

    avgFreq = this->calculateAverageFreq();                       // determine the new average frequency
    int currVal = dacVoltageValues[calibrationIndex];             // pre-calibrated value to be adjusted
    
    if (avgFreq > PITCH_FREQ[calibrationIndex] + threshold) {     // if overshoot
      currVal -= 10;
      dacVoltageValues[calibrationIndex] = currVal;
    }
    
    else if (avgFreq < PITCH_FREQ[calibrationIndex] - threshold) { // if undershoot
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
    
    readyToCalibrate = false;
    freqSampleIndex = 0;

    if (calibrationIndex < 12) {
      calibrationIndex += 1;          // increase note index by 1
    }
    else {                            // finished calibrating
      this->generateDacVoltageMap();  // set dac map to use new calibrated values
      calibrationIndex = 0;           // deactivate calibration mode
      calibrationFinished = true; 
    }
  }
}

/**
 * this function takes an 1D array and converts it into a 2D array formatted like [[0, 1, 2], ...]
*/ 
void TouchChannel::generateDacVoltageMap() {
  int index = 0;
  int octaveIndexes[4] = { 0, 12, 24, 32 };
  int multiplier = 1;

  for (int oct = 0; oct < 4; oct++) {
    int index = octaveIndexes[oct];
    int limit = 8 * multiplier;
    for (int i = limit - 8; i < limit; i++) {
      dacVoltageMap[i][0] = dacVoltageValues[index];
      dacVoltageMap[i][1] = dacVoltageValues[index + 1];
      dacVoltageMap[i][2] = dacVoltageValues[index + 2];
      index += 2;
    }
    multiplier += 1;
  }
}



// Since once frequency detection is not always accurate, take a running average of MAX_FREQ_SAMPLES
float TouchChannel::calculateAverageFreq() {
  float sum = 0;
  for (int i = 0; i < MAX_FREQ_SAMPLES; i++) {
    sum += this->freqSamples[i];
  }
  return sum / MAX_FREQ_SAMPLES;
}


/**
 * CALLBACK executing at desired VCO_SAMPLE_RATE_US
*/ 
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