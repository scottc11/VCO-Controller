#include "TouchChannel.h"
#include "PitchFrequencies.h"

void TouchChannel::enableCalibrationMode() {
  this->setAllLeds(HIGH);
  wait_ms(500);
  this->setAllLeds(LOW);
  wait_ms(500);
  this->setAllLeds(HIGH);
  wait_ms(500);
  this->setAllLeds(LOW);
  wait_ms(500);
  this->setLed(0, HIGH);

  dac->write(dacChannel, dacVoltageValues[0]);  // start at bottom most note.

  ticker->attach_us(callback(this, &TouchChannel::sampleVCOFrequency), VCO_SAMPLE_RATE_US);

}

void TouchChannel::disableCalibrationMode() {
  ticker->detach();
  this->setAllLeds(HIGH);
  wait_ms(500);
  this->setAllLeds(LOW);
  wait_ms(500);
  this->setAllLeds(HIGH);
  wait_ms(500);
  this->setAllLeds(LOW);
  wait_ms(500);
  calNoteIndex = 0;           // deactivate calibration mode
  calibrationFinished = true;
  this->setMode(MONO);
}

void TouchChannel::calibrateVCO() {

  // wait till MAX_FREQ_SAMPLES has been obtained
  if (readyToCalibrate) {
    
    prevAvgFreq = avgFreq;
    float threshold = 0.1;
    avgFreq = this->calculateAverageFreq();     // determine the new average frequency
    
    // if avgFreq is close enough to desired freq
    if ((avgFreq <= PITCH_FREQ[calNoteIndex] + threshold && avgFreq >= PITCH_FREQ[calNoteIndex] - threshold) || calibrationAttemps > MAX_CALIB_ATTEMPTS) {
      
      // move to next pitch to be calibrated
      if (calNoteIndex < 59) {
        setLed(CALIBRATION_LED_MAP[calNoteIndex], LOW);
        adjustment = DEFAULT_VOLTAGE_ADJMNT;     // reset to default
        calibrationAttemps = 0;
        calNoteIndex += 1;    // increase note index by 1
        setLed(CALIBRATION_LED_MAP[calNoteIndex], HIGH);
      }
      // finished calibrating
      else {
        this->generateDacVoltageMap();  // set dac map to use new calibrated values
        this->disableCalibrationMode();
      }

    } else {
      // every time avgFreq over/undershoots the desired frequency, decrement the 'adjustment' value by half.
      int currVal = dacVoltageValues[calNoteIndex];             // pre-calibrated value to be adjusted
      
      if (avgFreq > PITCH_FREQ[calNoteIndex] + threshold) {     // if overshoot
        if (prevAvgFreq < PITCH_FREQ[calNoteIndex] - threshold) {
          overshoot = true;
          adjustment = (adjustment / 2) + 1; // so it never becomes zero
        }
        currVal -= adjustment;
        dacVoltageValues[calNoteIndex] = currVal;
      }
      
      else if (avgFreq < PITCH_FREQ[calNoteIndex] - threshold) { // if undershoot
        if (prevAvgFreq > PITCH_FREQ[calNoteIndex] + threshold) {
          overshoot = false;
          adjustment = (adjustment / 2) + 1; // so it never becomes zero
        }
        currVal += adjustment;
        dacVoltageValues[calNoteIndex] = currVal;
      }
    }
    // output new voltage and reset calibration process
    dac->write(dacChannel, dacVoltageValues[calNoteIndex]);
    wait_ms(50); // give time for new voltage to 'settle'
    freqSampleIndex = 0;
    readyToCalibrate = false;
    calibrationAttemps += 1;
  }
}

/**
 * this function takes an 1D array and converts it into a 2D array formatted like [[0, 1, 2], ...]
 * take the first 12 values from dacVoltageValues. find the difference dacVoltageValues[i]
*/ 
void TouchChannel::generateDacVoltageMap() {
  int index = 0;
  int octaveIndexes[4] = { 0, 12, 24, 36 };
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
// NOTE: the first sample in freqSamples[] will always be scewed since the numSamplesTaken (since the first positive zero crossing)
// could be any value between 1 and the average sample time (between positive zero crossings)
float TouchChannel::calculateAverageFreq() {
  float sum = 0;
  for (int i = 1; i < MAX_FREQ_SAMPLES; i++) {
    sum += this->freqSamples[i];
  }
  return sum / (MAX_FREQ_SAMPLES - 1);
}


/**
 * CALLBACK executing at desired VCO_SAMPLE_RATE_US
*/ 
void TouchChannel::sampleVCOFrequency() {
  if (!readyToCalibrate) {
    currVCOInputVal = slewCvInput.read_u16();

    // NEGATIVE
    if (currVCOInputVal >= (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal < (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && slopeIsPositive) {   // maybe plus threshold zero crossing
      slopeIsPositive = false;
    }
    // POSITIVE
    else if (currVCOInputVal <= (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal > (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && !slopeIsPositive) {  // maybe negative threshold zero crossing
      vcoPeriod = numSamplesTaken;     // how many samples have occurred between positive zero crossings
      vcoFrequency = 8000.00 / vcoPeriod; // sample rate divided by period of input signal
      freqSamples[freqSampleIndex] = vcoFrequency;
      numSamplesTaken = 0;
      
      if ( freqSampleIndex < MAX_FREQ_SAMPLES ) {
        freqSampleIndex += 1;
      } else {
        readyToCalibrate = true;
        freqSampleIndex = 0;
      }
      slopeIsPositive = true;
    }
    
    prevVCOInputVal = currVCOInputVal;
    numSamplesTaken++;
  }
}