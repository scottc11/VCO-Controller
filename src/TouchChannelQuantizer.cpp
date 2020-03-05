#include "TouchChannel.h"

void TouchChannel::handleCVInput(int value) {
  // 65,536 / 4 == 16,384
  // 16,384 / 8 == 2,048

  int clippedValue = 0;
  int octave = 0;

  // determin which octave the CV value will get mapped to
  // for (int i=0; i < 4; i++) {
  //   if (value < 16384) {
  //     clippedValue = value;
  //     octave = 0;
  //     break;
  //   }
  //   if (value > CV_OCTAVES[i] && value < CV_OCTAVES[i] + 16384) {
  //     octave = i + 1;
  //     clippedValue = value - CV_OCTAVES[i];
  //     break;
  //   }
  // }

  // for (int i=0; i < 8; i++) {
  //   if (clippedValue < CV_INPUT_MAP[i]) {
  //     if (prevNoteIndex != i) {
  //       this->triggerNote(prevNoteIndex, prevOctave, OFF);
  //       this->triggerNote(i, octave, ON);
  //       this->setOctaveLed(octave);
  //     }
  //     break;
  //   }
  // }
}

// when a channels degree is touched, toggle the active/inactive status of the touched degree
void TouchChannel::setActiveDegrees(int degree) {
  activeDegrees = bitWrite(activeDegrees, degree, !bitRead(activeDegrees, degree));
  this->updateLeds(activeDegrees);
}