/**
 * ============================================ 
 * ------------ QUANTIZATION ------------------
 * 
 * determin which degreese are active / inactive
 * apply the number of active degrees to numActiveDegrees variable
 * read the voltage on analog input pin
 * divide the maximum possible value from analog input pin by the number numActiveDegrees
 * map this value to an array[8]
 * map voltage input values (num between 0 and 1023)
*/

#include "TouchChannel.h"

const int CV_INPUT_MAP[8] = { 2048, 4096, 6144, 8192, 10240, 12288, 14366, 16384 };  // ADC values for each degree in one octave
const int CV_OCTAVES[4] = { 16384, 32768, 49152, 65536 };  // max ADC divided into 4 octaves

void TouchChannel::initQuantizer() {
  activeDegrees = 0xFF;
  this->numActiveDegrees = DEGREE_COUNT;
  for (int i = 0; i < this->numActiveDegrees; i++) {
    activeDegreeValues[i].noteIndex = i;
    activeDegreeValues[i].threshold = CV_INPUT_MAP[i];
  }
  
  if (this->mode == QUANTIZE || this->mode == QUANTIZE_LOOP) {
    this->updateLeds(activeDegrees);
  }
}


void TouchChannel::handleCVInput(int value) {
  // 65,536 / 4 == 16,384
  // 16,384 / 8 == 2,048

  int clippedValue = 0; // we want a number between 0 and 16384 for mapping to degrees. The octave is added afterwords via CV_OCTAVES
  int octave = 0;

  // determin which octave the CV value will get mapped to
  for (int i=0; i < 4; i++) {
    if (value < 16384) {
      clippedValue = value;
      octave = 0;
      break;
    }
    if (value > CV_OCTAVES[i] && value < CV_OCTAVES[i] + 16384) {
      octave = i + 1;
      clippedValue = value - CV_OCTAVES[i];
      break;
    }
  }

  // latch cv value to
  for (int i = 0; i < numActiveDegrees; i++) {
    if (clippedValue < activeDegreeValues[i].threshold) {
      if (prevNoteIndex != activeDegreeValues[i].noteIndex || prevOctave != octave) {
        this->triggerNote(prevNoteIndex, prevOctave, OFF);
        this->triggerNote(activeDegreeValues[i].noteIndex, octave, ON);
        this->updateOctaveLeds(octave);
      }
      break;
    }
  }
}

/**
 * when a channels degree is touched, toggle the active/inactive status of the 
 * touched degree by flipping the bit of the given index that was touched
*/
void TouchChannel::setActiveDegrees(int index) {
  activeDegrees = bitFlip(activeDegrees, index);
  
  this->updateActiveDegreeLeds();
  
  // apply the number of active degrees to numActiveDegrees variable
  numActiveDegrees = 0;
  for (int i = 0; i < 8; i++) {
    if (bitRead(activeDegrees, i)) {
      activeDegreeValues[numActiveDegrees].noteIndex = i;
      numActiveDegrees += 1;
    }
  }

  int min_threshold = 16384 / numActiveDegrees; // divide one CV octave by the number numActiveDegrees
  
  // for each active degree, generate a 'threshold' for latching CV input value
  for (int i = 0; i < numActiveDegrees; i++) {
    activeDegreeValues[i].threshold = min_threshold * (i + 1); // can't multiply by zero!
  }
  

}