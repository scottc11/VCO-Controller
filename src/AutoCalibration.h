#ifndef __AUTO_CALIB_H
#define __AUTO_CALIB_H

#include <mbed.h>

// Amplitude     -> defined as the farthest distance the wave gets from its center/zero crossing.
// Period        -> length of one cycle of the curve
// Zero Crossing -> the common point which the wave passes its mid-point

// It brackets the zero-crossing event and uses increasingly smaller time steps to pinpoint when the zero crossing has occurred

// class AutoCalibration {
// public:

//   const int sample_rate_us = 125;  // 8000hz is equal to 125us (microseconds)

//   const int numFreqSamples = 100;
//   int sampleIndex = 0;        // incrementing value to place frequency into array
//   float averageFreq;            
//   volatile int period;        // equal to the number of samples taken between zero crossings
//   volatile float frequency;
//   volatile float data[numFreqSamples];

//   int numSamplesTaken = 0;

//   int currValue = 0;
//   int prevValue = 0;
//   int zero_crossing = 32767;
//   int threshold = 500;
//   int isPositive = false;

//   AnalogIn input;

//   AutoCalibration();

//   void calibrate() {
//     currValue = input.read_u16();      

//     // NEGATIVE
//     if (currValue >= (zero_crossing + threshold) && prevValue < (zero_crossing + threshold) && isPositive) {   // maybe plus threshold zero crossing
//       // pulse.write(0);
//       isPositive = false;
//     }
//      // POSITIVE
//     else if (currValue <= (zero_crossing - threshold) && prevValue > (zero_crossing - threshold) && !isPositive) {  // maybe negative threshold zero crossing
//       // pulse.write(1);
//       period = numSamplesTaken;
//       frequency = 8000 / period;
//       data[sampleIndex] = frequency;
//       numSamplesTaken = 0;
//       sampleIndex = sampleIndex >= numFreqSamples - 1 ? 0 : sampleIndex + 1;
//       isPositive = true;
//     }
    
//     prevValue = currValue;
//     numSamplesTaken++;
//   }

// };


#endif