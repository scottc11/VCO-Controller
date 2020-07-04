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

#define CV_MAX     65535


void TouchChannel::initQuantizerMode() {
  this->activeDegrees = 0xFF;
  this->activeOctaves = 0xF;
  this->numActiveDegrees = DEGREE_COUNT;
  this->numActiveOctaves = 3; // there is a bug requiring this to be set in initialization, even though it should be set when turning on the octave leds via setActiveOctaves fn

  this->setActiveOctaves(3);

  this->quantizerHasBeenInitialized = true;
}



void TouchChannel::setActiveDegreeLimit(int value) {
  activeDegreeLimit = value;
}



void TouchChannel::handleCVInput(int value) {
  int adcValue = CV_MAX - value; // NOTE: CV voltage input is inverted, so everything needs to be flipped to make more sense
  int refinedValue = 0; // we want a number between 0 and CV_OCTAVE for mapping to degrees. The octave is added afterwords via CV_OCTAVES
  int octave = 0;

  // determin which octave the CV value will get mapped to
  for (int i=0; i < numActiveOctaves; i++) {
    if (adcValue < activeOctaveValues[i].threshold) {
      octave = activeOctaveValues[i].octave;
      refinedValue = i == 0 ? adcValue : adcValue - activeOctaveValues[i - 1].threshold; // remap adc value to a number between 0 and octaveThreshold
      break;
    }
  }

  // latch incoming ADC value to DAC value
  for (int i = 0; i < numActiveDegrees; i++) {
    if (refinedValue < activeDegreeValues[i].threshold) {                              // break from loop as soon as we can
      if (prevNoteIndex != activeDegreeValues[i].noteIndex || prevOctave != octave) {  // catch duplicate triggering of that same note.
        this->triggerNote(prevNoteIndex, prevOctave, OFF);
        if (bitRead(activeDegrees, prevNoteIndex)) { // if prevNote still active, its led needs to be set from dimmed back fully ON
          setLed(prevNoteIndex, HIGH);
        }
        this->triggerNote(activeDegreeValues[i].noteIndex, octave, ON, true);
        
        if (bitRead(activeOctaves, prevOctave)) {
          this->setOctaveLed(prevOctave, LedState::HIGH);
        }
        this->setOctaveLed(octave, LedState::BLINK);
      }
      break;
    }
  }
}

/**
 * when a channels degree is touched, toggle the active/inactive status of the 
 * touched degree by flipping the bit of the given index that was touched
*/
void TouchChannel::setActiveDegrees(int degrees) {
  // activeDegrees = bitFlip(activeDegrees, index); // this did not work for some reason....
  activeDegrees = degrees;
  
  this->updateActiveDegreeLeds();
  
  // determine the number of active degrees
  numActiveDegrees = 0;
  for (int i = 0; i < 8; i++) {
    if (bitRead(activeDegrees, i)) {
      activeDegreeValues[numActiveDegrees].noteIndex = i;
      numActiveDegrees += 1;
    }
  }
  
  int octaveThreshold = CV_MAX / numActiveOctaves;        // divide max ADC value by num octaves
  int min_threshold = octaveThreshold / numActiveDegrees; // then numActiveDegrees
  
  // for each active octave, generate an ADC threshold for mapping ADC values to DAC octave values
  for (int i = 0; i < numActiveOctaves; i++) {
    activeOctaveValues[i].threshold = octaveThreshold * (i + 1); // can't multiply by zero
  }
  

  // for each active degree, generate a ADC 'threshold' for latching CV input values
  for (int i = 0; i < numActiveDegrees; i++) {
    activeDegreeValues[i].threshold = min_threshold * (i + 1); // can't multiply by zero
  }
}

void TouchChannel::setActiveOctaves(int octave) {
  // take the newly touched octave, and either add it or remove it from the activeOctaves list
  int newActiveOctaves = bitFlip(activeOctaves, octave);
  
  if (newActiveOctaves != 0) { // one octave must always remain active.
    activeOctaves = newActiveOctaves;
    this->updateOctaveLeds(activeOctaves); // fn also sets numActiveOctaves
    this->setActiveDegrees(activeDegrees);
  }
  
}

void TouchChannel::updateActiveDegreeLeds() {
  for (int i = 0; i < 8; i++) {
    if (bitRead(activeDegrees, i)) {
      setLed(i, HIGH);
    } else {
      setLed(i, LOW);
    }
  }
}