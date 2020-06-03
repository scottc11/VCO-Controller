#include "TouchChannel.h"

void TouchChannel::init() {
  touch->init();
  leds->initialize();
  if (!touch->isConnected()) { this->setOctaveLed(3); return; }
  touch->calibrate();
  touch->clearInterupt();
  dac->init();

  // intialize inheritance variables
  numLoopSteps = DEFAULT_CHANNEL_LOOP_STEPS;
  loopMultiplier = 1;
  timeQuantizationMode = QUANT_NONE;
  currStep = 0;
  currTick = 0;
  currPosition = 0;

  // initialize default variables
  currNoteIndex = 0;
  currOctave = 0;
  counter = 0;
  touched = 0;
  prevTouched = 0;
  enableQuantizer = false;
  mode = MONO;

  this->handleOctaveChange(currOctave);
  this->initQuantizer();
}

// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  
  if (mode == LOOP_LENGTH_UI) {
    // set all leds based on loop length * multiplier
    // toggleLoopLengthUI()
    // revertUI()
  }

  if (touchDetected) {
    handleTouch();
    touchDetected = false;
  }

  currModeBtnState = modeBtn.read();
  if (currModeBtnState != prevModeBtnState) {
    if (currModeBtnState == LOW) {
      this->toggleMode();
    }
    prevModeBtnState = currModeBtnState;
  }

  // if (degrees->hasChanged[channel]) {
  //   handleDegreeChange();
  // }

  if ((mode == QUANTIZE || mode == QUANTIZE_LOOP) && enableQuantizer) {
    // this->flashNoteLed(currNoteIndex);
    currCVInputValue = cvInput.read_u16();
    if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER ) {
      handleCVInput(currCVInputValue);
      prevCVInputValue = currCVInputValue;
    }
  }

  // if ((mode == MONO_LOOP || mode == QUANTIZE_LOOP) && queuedEvent && enableLoop ) {
  //   handleQueuedEvent(currPosition);
  // }
}

void TouchChannel::flashNoteLed(int index) {
  if (currPosition % 6 == 0) {
    this->toggleLed(currNoteIndex);
  }
}

void TouchChannel::handleQueuedEvent(int position) {
  // if (queuedEvent->triggered == false ) {
  //   if (position == queuedEvent->startPos) {
  //     switch (mode) {
  //       case MONO_LOOP:
  //         triggerNote(queuedEvent->noteIndex, currOctave, ON);
  //         break;
  //       case QUANTIZE_LOOP:
  //         setActiveDegrees(queuedEvent->activeNotes);
  //         break;
  //     }
  //     queuedEvent->triggered = true;
  //   }
  // }
  // else {
  //   if (position == queuedEvent->endPos) {
  //     if (mode == MONO_LOOP) triggerNote(queuedEvent->noteIndex, currOctave, OFF);
  //     queuedEvent->triggered = false;
  //     if (queuedEvent->next != NULL) {
  //       queuedEvent = queuedEvent->next;
  //     } else {
  //       queuedEvent = head;
  //     }
  //   }
  // }
}


void TouchChannel::enableLoopLengthUI() {
  prevMode = mode;
  this->mode = LOOP_LENGTH_UI;
  updateLoopLengthUI();
}

void TouchChannel::disableLoopLengthUI() {
  this->mode = prevMode;
  setAllLeds(LOW);
  triggerNote(currNoteIndex, currOctave, PREV);
}

void TouchChannel::updateLoopLengthUI() {
  setAllLeds(LOW);
  for (int i = 0; i < numLoopSteps; i++) {
    setLed(i, HIGH);
  }
}

void TouchChannel::setLoopLength(int num) {
  numLoopSteps = num;
  updateLoopLengthUI();
};

/**
 * CALCULATE LOOP LENGTH
*/
void TouchChannel::calculateLoopLength() {
  loopLength = numLoopSteps * PPQN;
}

/**
 * ADVANCE LOOP POSITION
 * advance the channels loop position by 1 'tick', a 'tick' being a single Pulse Per Quarter Note or "PPQN"
*/

void TouchChannel::tickClock() {
  currTick += 1;
  currPosition += 1;
  
  if (currTick > 2) {
    if (isSelected) {
      ctrlLed.write(HIGH);
    } else {
      ctrlLed.write(LOW);
    }
  }

  // when currTick exceeds PPQN, reset to 0
  if (currTick >= PPQN) {
    currTick = 0;
  }
}

void TouchChannel::stepClock() {
  currTick = 0;
  currStep += 1;
  // when currStep eqauls number of steps in loop, reset currStep and currPosition to 0
  if (currStep >= numLoopSteps) {
    currStep = 0;
    currPosition = 0;
    
    // signal end of loop via control led
    if (isSelected) { 
      ctrlLed.write(LOW);
    } else {
      ctrlLed.write(HIGH);
    }
  }
}


/**
 * 
 * NOTE: you need a way to trigger events after a series of touches have happened, and the channel is now not being touched
 * 
*/ 
void TouchChannel::handleTouch() {
  touched = touch->touched();
  if (touched != prevTouched) {
    for (int i=0; i<8; i++) {
      // if it *is* touched and *wasnt* touched before, alert!
      if (touch->getBitStatus(touched, i) && !touch->getBitStatus(prevTouched, i)) {

        switch (mode) {
          case MONO:
            triggerNote(i, currOctave, ON);
            break;
          case QUANTIZE:
            setActiveDegrees(i);
            break;
          case QUANTIZE_LOOP:
            // every touch detected, take a snapshot of all active degree values and apply them to a EventNode
            enableLoop = false;
            setActiveDegrees(i);
            createChordEvent(currPosition, activeDegrees);
            addEventToList(currPosition);
            break;
          case MONO_LOOP:
            enableLoop = false;
            createEvent(currPosition, i);
            addEventToList(currPosition);
            triggerNote(i, currOctave, ON);
            break;
          case LOOP_LENGTH_UI:
            setLoopLength(i + 1); // loop length is not zero indexed
            break;
        }
      }
      // if it *was* touched and now *isnt*, alert!
      if (!touch->getBitStatus(touched, i) && touch->getBitStatus(prevTouched, i)) {
        
        switch (mode) {
          case MONO:
            triggerNote(i, currOctave, OFF);
            break;
          case QUANTIZE:
            enableLoop = true;
            break;
          case QUANTIZE_LOOP:
            enableLoop = true;
            break;
          case MONO_LOOP:
            triggerNote(i, currOctave, OFF);
            enableLoop = true;
            break;
        }
      }
    }
    prevTouched = touched;
  }
}

/**
 * mode switch states determined by the last 2 bits of io's port B
**/
void TouchChannel::toggleMode() {
  enableQuantizer = false;
  modeCounter += 2; // to be changed to 1 when other modes implemented
  if (modeCounter > 3) { modeCounter = 0; }

  switch (modeCounter) {
    case MONO:
      enableLoop = false;
      mode = MONO;
      setAllLeds(LOW);
      triggerNote(currNoteIndex, currOctave, ON);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE:
      enableLoop = false;
      enableQuantizer = true;
      mode = QUANTIZE;
      setAllLeds(LOW);
      updateActiveDegreeLEDs();
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE_LOOP:
      // delete previous loop objects for safety
      // this->clearEventList();
      // enableLoop = true;
      // mode = QUANTIZE_LOOP;
      // triggerNote(prevNoteIndex, currOctave, OFF);
      break;
    case MONO_LOOP:
      // enableLoop = true;
      // mode = MONO_LOOP;
      // triggerNote(prevNoteIndex, currOctave, OFF);
      break;
  }
}


/**
 * take a number between 0 - 3 and apply to currOctave
**/
void TouchChannel::handleOctaveChange(int value) {
  
  currOctave = value;
  setOctaveLed(currOctave);

  switch (mode) {
    case MONO:
      triggerNote(currNoteIndex, currOctave, ON);
      break;
    case QUANTIZE:
      break;
    case MONO_LOOP:
      break;
  }

  prevOctave = currOctave;
}

void TouchChannel::handleDegreeChange() {
  switch (mode) {
    case MONO:
      triggerNote(currNoteIndex, currOctave, ON);
      break;
    case QUANTIZE:
      break;
    case MONO_LOOP:
      break;
  }
  degrees->hasChanged[channel] = false;
}

void TouchChannel::setAllLeds(int state) {
  switch (state) {
    case HIGH:
      break;
    case LOW:
      leds->setAllOutputsOff();
      break;
  }
}

void TouchChannel::setLed(int index, int state) {
  switch (state) {
    case HIGH:
      ledStates |= 1 << index;
      leds->setLedOutput(greenLedPins[index], TLC59116::ON);
      break;
    case LOW:
      ledStates &= ~(1 << index);
      leds->setLedOutput(greenLedPins[index], TLC59116::OFF);
      break;
  }
  
}

void TouchChannel::toggleLed(int index) {
  int toggles = activeDegrees;
  toggles ^= 1UL << index;
  // io->digitalWrite(MCP23017_PORTA, toggles);
}

void TouchChannel::updateLeds(uint8_t touched) {
  leds->setLedOutput16(ledStates);
}

void TouchChannel::setOctaveLed(int octave) {
  for (int i = 0; i < 4; i++) {
    if (i == octave) {
      octLeds->setLedOutput(octLedPins[i], TLC59116::ON);
    } else {
      octLeds->setLedOutput(octLedPins[i], TLC59116::OFF);
    }
  }
}

void TouchChannel::triggerNote(int index, int octave, NoteState state) {
  switch (state) {
    case ON:
      // if midiNoteState == ON, midi->sendNoteOff(prevNoteIndex, prevOctave)
      if (mode == MONO || mode == MONO_LOOP) {
        setLed(prevNoteIndex, LOW);
        setLed(index, HIGH);
      }
      currNoteIndex = index;
      currOctave = octave;
      gateOut.write(HIGH);
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case OFF:
      gateOut.write(LOW);
      midi->sendNoteOff(channel, calculateMIDINoteValue(index, octave), 100);
      wait_us(1);
      break;
    case PREV:
      setLed(index, HIGH);
      break;
  }
  prevOctave = octave;
  prevNoteIndex = index;
}

int TouchChannel::calculateDACNoteValue(int index, int octave) {
  return DAC_NOTE_MAP[index][degrees->switchStates[index]] + DAC_OCTAVE_MAP[octave];
}

int TouchChannel::calculateMIDINoteValue(int index, int octave) {
  return MIDI_NOTE_MAP[index][degrees->switchStates[index]] + MIDI_OCTAVE_MAP[octave];
}


/**
 * FREEZE
 * takes boolean to either freeze or unfreeze
*/ 
void TouchChannel::freeze(bool freeze) {
  // hold all gates in their current state
  switch (mode) {
    case MONO:
      break;
    case QUANTIZE:
      enableQuantizer = freeze ? false : true;
      break;
    case QUANTIZE_LOOP:
      // turn quantizer off
      enableLoop = freeze ? false : true;
      enableQuantizer = freeze ? false : true;
      break;
    case MONO_LOOP:
      enableLoop = freeze ? false : true;
      break;
  }
}

void TouchChannel::reset() {
  switch (mode) {
    case MONO:
      break;
    case QUANTIZE_LOOP:
      currPosition = 0;
      currStep = 0;
      break;
    case MONO_LOOP:
      // NOTE: you probably don't want to reset the 'tick' value, as it would make it very dificult to line up with the global clock;
      currPosition = 0;
      currStep = 0;
      break;
  }
}

void TouchChannel::updateActiveDegreeLEDs() {
  for (int i = 0; i < 8; i++) {
    if (bitRead(activeDegrees, i)) {
      leds->setLedOutput(redLedPins[i], TLC59116::ON);
    } else {
      leds->setLedOutput(redLedPins[i], TLC59116::OFF);
    }
  }
}