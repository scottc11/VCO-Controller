#include "TouchChannel.h"

void TouchChannel::init() {
  touch->init();
  leds->initialize();

  if (!touch->isConnected()) { this->updateOctaveLeds(3); return; }
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
  setLoopTotalSteps();
  setLoopTotalPPQN();

  // initialize default variables
  currNoteIndex = 0;
  currOctave = 0;
  counter = 0;
  touched = 0;
  prevTouched = 0;
  enableQuantizer = false;
  mode = MONO;

  this->setOctave(currOctave);
  this->initQuantizer();
}

// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  
  if (mode == LOOP_LENGTH_UI) {
    // take current clock step and flash the corrosponding channel LED and Octave LED
    if (currTick == 0) {
      int modulo = currStep % numLoopSteps;
      
      if (modulo != 0) {           // setting the previous LED back to normal
        setLed(modulo - 1, HIGH);
      } else {                     // when modulo rolls past 7 and back to 0
        setLed(numLoopSteps - 1, HIGH);
      }

      setLed(modulo, BLINK);
      
      for (int i = 0; i < loopMultiplier; i++) {
        if (currStep < (numLoopSteps * (i + 1)) && currStep >= (numLoopSteps * i)) {
          setOctaveLed(i, BLINK);
        } else {
          setOctaveLed(i, HIGH);
        }
      }
    }
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
    currCVInputValue = cvInput.read_u16();
    if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER ) {
      handleCVInput(currCVInputValue);
      prevCVInputValue = currCVInputValue;
    }
  }

  if ((mode == MONO_LOOP || mode == QUANTIZE_LOOP) && queuedEvent && enableLoop ) {
    handleQueuedEvent(currPosition);
  }
}

void TouchChannel::handleQueuedEvent(int position) {
  if (queuedEvent->triggered == false ) {
    if (position == queuedEvent->startPos) {
      switch (mode) {
        case MONO_LOOP:
          triggerNote(queuedEvent->noteIndex, currOctave, ON);
          break;
        case QUANTIZE_LOOP:
          setActiveDegrees(queuedEvent->activeNotes);
          break;
      }
      queuedEvent->triggered = true;
    }
  }
  else {
    if (position == queuedEvent->endPos) {
      disableQueuedEvent();
    }
  }
}

void TouchChannel::disableQueuedEvent() {
  if (queuedEvent->triggered == true) {  // check required as this fn gets called for safety sometimes

    queuedEvent->triggered = false;

    if (mode == MONO_LOOP) triggerNote(queuedEvent->noteIndex, currOctave, OFF);
    
    if (queuedEvent->next != NULL) {
      queuedEvent = queuedEvent->next;
    } else {
      queuedEvent = head;
    }
  }
}

/** ------------------------------------------------------------------------
 *         LOOP UI METHODS
---------------------------------------------------------------------------- */

void TouchChannel::enableLoopLengthUI() {
  prevMode = mode;
  this->mode = LOOP_LENGTH_UI;
  updateLoopLengthUI();
}

void TouchChannel::disableLoopLengthUI() {
  this->mode = prevMode;
  setAllLeds(LOW);
  if (mode == MONO || mode == MONO_LOOP) {
    updateOctaveLeds(currOctave);
    triggerNote(currNoteIndex, currOctave, PREV);
  } else {
    updateActiveDegreeLeds();
  }
}

void TouchChannel::updateLoopLengthUI() {
  setAllLeds(LOW);
  updateLoopMultiplierLeds();
  for (int i = 0; i < numLoopSteps; i++) {
    if (i == currStep) {
      setLed(i, BLINK);
    } else {
      setLed(i, HIGH);
    }
  }
}


void TouchChannel::clearLoop() {
  disableQueuedEvent();
  clearEventList();
}

void TouchChannel::setLoopLength(int value) {
  disableQueuedEvent();  // incase an event occurs AFTER the new loop length value
  numLoopSteps = value;
  setLoopTotalSteps();
  setLoopTotalPPQN();
  updateLoopLengthUI();
};

void TouchChannel::setLoopMultiplier(int value) {
  loopMultiplier = value;
  setLoopTotalSteps();
  setLoopTotalPPQN();
  updateLoopLengthUI();
}

void TouchChannel::setLoopTotalSteps() {
  totalSteps = numLoopSteps * loopMultiplier;
}

void TouchChannel::setLoopTotalPPQN() {
  totalPPQN = totalSteps * PPQN;
}

void TouchChannel::enableLoopMode() {
  setMode(MONO_LOOP);
}

void TouchChannel::disableLoopMode() {

  // a nice feature here would be to only have the LEDs be red when REC is held down, and flash the green LEDs 
  // when a channel contains loop events, but REC is NOT held down. You would only be able to add new events to 
  // the loop when REC is held down (ie. when channel leds are RED)

  // ADDITIONALLY, this would be a good place to count the amount of steps which have passed while the REC button has
  // been held down, and if this value is greater than the current loop length, update the loop length to accomodate.

  if (queuedEvent) {   // if an touch event was recorded, remain in looper mode
    return;
  } else {             // if no touch event recorded, revert to previous mode
    setMode(prevMode);
  }
}

/** ------------------------------------------------------------------------
 *         CLOCK METHODS
---------------------------------------------------------------------------- */

/**
 * ADVANCE LOOP POSITION
 * advance the channels loop position by 1 'tick', a 'tick' being a single Pulse Per Quarter Note or "PPQN"
*/

void TouchChannel::tickClock() {
  currTick += 1;
  currPosition += 1;
  
  // if (currTick > 2) {
  //   if (isSelected) {
  //     ctrlLed.write(HIGH);
  //   } else {
  //     ctrlLed.write(LOW);
  //   }
  // }

  // when currTick exceeds PPQN, reset to 0
  if (currTick >= PPQN) {
    currTick = 0;
  }
  if (currPosition >= totalPPQN) {
    currPosition = 0;
  }
}

void TouchChannel::stepClock() {
  // if (isSelected) {
  //   ctrlLed.write(LOW);
  // } else {
  //   ctrlLed.write(HIGH);
  // }

  currTick = 0;
  currStep += 1;
  // when currStep eqauls number of steps in loop, reset currStep and currPosition to 0
  if (currStep >= totalSteps) {
    currStep = 0;
  }
}



/** ------------------------------------------------------------------------
 *         TOUCH EVENT METHODS
---------------------------------------------------------------------------- */

// NOTE: you need a way to trigger events after a series of touches have happened, and the channel is now not being touched

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
            setActiveDegrees(bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
            break;
          case QUANTIZE_LOOP:
            // every touch detected, take a snapshot of all active degree values and apply them to a EventNode
            enableLoop = false;
            setActiveDegrees(bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
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
 * TOGGLE MODE
**/
void TouchChannel::toggleMode() {
  if (mode == MONO) {
    setMode(QUANTIZE);
  } else {
    setMode(MONO);
  }
}

void TouchChannel::setMode(Mode targetMode) {
  switch (targetMode) {
    case MONO:
      enableLoop = false;
      enableQuantizer = false;
      mode = MONO;
      setAllLeds(LOW);
      triggerNote(currNoteIndex, currOctave, ON);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case MONO_LOOP:
      enableLoop = true;
      enableQuantizer = false;
      mode = MONO_LOOP;
      setAllLeds(LOW);
      triggerNote(prevNoteIndex, currOctave, ON);
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE:
      enableLoop = false;
      enableQuantizer = true;
      mode = QUANTIZE;
      setAllLeds(LOW);
      updateActiveDegreeLeds();
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE_LOOP:
      // delete previous loop objects for safety
      this->clearEventList();
      enableLoop = true;
      enableQuantizer = true;
      mode = QUANTIZE_LOOP;
      setAllLeds(LOW);
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
  }
}

/**
 * take a number between 0 - 3 and apply to currOctave
**/
void TouchChannel::setOctave(int value) {
  
  currOctave = value;
  updateOctaveLeds(currOctave);

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



/** ------------------------------------------------------------------------
 *         LED METHODS
---------------------------------------------------------------------------- */

void TouchChannel::setAllLeds(int state) {
  switch (state) {
    case HIGH:
      break;
    case LOW:
      leds->setAllOutputsOff();
      break;
  }
}

void TouchChannel::setLed(int index, LedState state) {
  // switch between red and green leds based on mode
  int (*pins_ptr)[8];
  pins_ptr = mode == MONO_LOOP || mode == QUANTIZE_LOOP ? &redLedPins : &greenLedPins;

  switch (state) {
    case LOW:
      ledStates &= ~(1 << index);
      leds->setLedOutput((*pins_ptr)[index], TLC59116::OFF);
      break;
    case HIGH:
      ledStates |= 1 << index;
      leds->setLedOutput((*pins_ptr)[index], TLC59116::ON);
      break;
    case BLINK:
      ledStates |= 1 << index;
      leds->setLedOutput((*pins_ptr)[index], TLC59116::PWM, 20);
      break;
  }
}

void TouchChannel::setOctaveLed(int octave, LedState state) {
  switch (state) {
    case LOW:
      octLeds->setLedOutput(octLedPins[octave], TLC59116::OFF);
      break;
    case HIGH:
      octLeds->setLedOutput(octLedPins[octave], TLC59116::ON);
      break;
    case BLINK:
      octLeds->setLedOutput(octLedPins[octave], TLC59116::PWM, 20);
      break;
  }
}

void TouchChannel::updateLeds(uint8_t touched) {
  leds->setLedOutput16(ledStates);
}

void TouchChannel::updateOctaveLeds(int octave) {
  for (int i = 0; i < 4; i++) {
    if (i == octave) {
      setOctaveLed(i, HIGH);
    } else {
      setOctaveLed(i, LOW);
    }
  }
}

void TouchChannel::updateLoopMultiplierLeds() {
  for (int i = 0; i < 4; i++) {
    if (i < loopMultiplier) {  // loopMultiplier is not zro indexed
      setOctaveLed(i, HIGH);
    } else {
      setOctaveLed(i, LOW);
    }
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

/** ------------------------------------------------------------------------
 *        TRIGGER NOTE
---------------------------------------------------------------------------- */
 
void TouchChannel::triggerNote(int index, int octave, NoteState state, bool dimLed /* false */) {
  switch (state) {
    case ON:
      if (mode == MONO || mode == MONO_LOOP) {
        setLed(prevNoteIndex, LOW);
        setLed(index, HIGH);
      }
      if (dimLed) setLed(index, BLINK);
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
  disableQueuedEvent(); // you should probably get the currently queued event, see if it has been triggered yet, and disable it if it has been triggered
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