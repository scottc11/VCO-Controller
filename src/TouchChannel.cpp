#include "TouchChannel.h"

void TouchChannel::init() {
  touch->init();
  leds->initialize();
  
  if (!touch->isConnected()) {
    this->updateLeds(0xFF);
    return;
  }
  touch->calibrate();
  touch->clearInterupt();

  this->updateLeds(0x00);
  this->setOctaveLed(currOctave);

  dac->init();

  currNoteIndex = 0;
  currOctave = 0;
  // dac->write_u12(dacChannel, calculateDACNoteValue(currNoteIndex, currOctave));

  this->initQuantizer();

}

// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  // if (touchDetected) {
  //   handleTouch();
  //   touchDetected = false;
  // }

  handleTouch();

  // if (degrees->hasChanged[channel]) {
  //   handleDegreeChange();
  // }

  // if ((mode == QUANTIZE || mode == QUANTIZE_LOOP) && enableQuantizer) {
  //   this->flashNoteLed(currNoteIndex);
  //   currCVInputValue = cvInput.read_u16();
  //   if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER ) {
  //     handleCVInput(currCVInputValue);
  //     prevCVInputValue = currCVInputValue;
  //   }
  // }

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
      if (mode == MONO_LOOP) triggerNote(queuedEvent->noteIndex, currOctave, OFF);
      queuedEvent->triggered = false;
      if (queuedEvent->next != NULL) {
        queuedEvent = queuedEvent->next;
      } else {
        queuedEvent = head;
      }
    }
  }
}


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
 * HANDLE SWITCH INTERUPT
*/
void TouchChannel::handleSwitchInterupt() {
  wait_us(10); // debounce
  currSwitchStates = readSwitchStates();
  int modeSwitchState = currSwitchStates & 0b00000011;   // set first 6 bits to zero
  int octaveSwitchState = currSwitchStates & 0b00001100; // set first 4 bits, and last 2 bits to zero
  
  if (modeSwitchState != (prevSwitchStates & 0b00000011) ) {
    handleModeSwitch(modeSwitchState);
  }

  if (octaveSwitchState != (prevSwitchStates & 0b00001100) ) {
    handleOctaveSwitch(octaveSwitchState);
  }
  
  prevSwitchStates = currSwitchStates;
  switchHasChanged = false;
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
        }
      }
      // if it *was* touched and now *isnt*, alert!
      if (!touch->getBitStatus(touched, i) && touch->getBitStatus(prevTouched, i)) {
        
        switch (mode) {
          case MONO:
            triggerNote(i, currOctave, OFF);
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
    // reg.writeByte(touched); // toggle channel LEDs
    prevTouched = touched;
  }
}

/**
 * mode switch states determined by the last 2 bits of io's port B
**/
void TouchChannel::handleModeSwitch(int state) {
  
  switch (state) {
    case 0b00000011:
      enableLoop = false;
      mode = MONO;
      triggerNote(currNoteIndex, currOctave, ON);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case 0b00000010:
      // enableLoop = false;
      // mode = QUANTIZE;
      // triggerNote(prevNoteIndex, currOctave, OFF);
      
      // delete previous loop objects for safety
      this->clearEventList();
      enableLoop = true;
      mode = QUANTIZE_LOOP;
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
    case 0b00000001:
      enableLoop = true;
      mode = MONO_LOOP;
      triggerNote(prevNoteIndex, currOctave, OFF);
      break;
  }
}


/**
 * octave switch states determined by bits 5 and 6 of io's port B
**/
void TouchChannel::handleOctaveSwitch(int state) {
  // update the octave value
  switch (state) {
    case OCTAVE_UP:
      if (currOctave < 3) { currOctave += 1; }
      break;
    case OCTAVE_DOWN:
      if (currOctave > 0) { currOctave -= 1; }
      break;
  }
  setOctaveLed(currOctave);

  if (state == OCTAVE_UP || state == OCTAVE_DOWN) {  // only want this to happen once
    switch (mode) {
      case MONO:
        triggerNote(currNoteIndex, currOctave, ON);
        break;
      case QUANTIZE:
        break;
      case MONO_LOOP:
        break;
    }
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

int TouchChannel::readSwitchStates() {
  // return io->digitalRead(MCP23017_PORTB) & 0xF;
  return 0;
}

void TouchChannel::writeLed(int index, int state) {
  switch (state) {
    case HIGH:
      ledStates |= 1 << index;
      leds->setLedOutput(index, TLC59116::ON);
      break;
    case LOW:
      ledStates &= ~(1 << index);
      leds->setLedOutput(index, TLC59116::OFF);
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
  int state = 1 << (octave + 4);
  // io->digitalWrite(MCP23017_PORTB, state);
}

void TouchChannel::triggerNote(int index, int octave, NoteState state) {
  switch (state) {
    case ON:
      // if midiNoteState == ON, midi->sendNoteOff(prevNoteIndex, prevOctave)
      if (mode == MONO || mode == MONO_LOOP) {
        writeLed(prevNoteIndex, LOW);
        writeLed(index, HIGH);
      }
      currNoteIndex = index;
      currOctave = octave;
      gateOut.write(HIGH);
      // dac->write_u12(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case OFF:
      gateOut.write(LOW);
      midi->sendNoteOff(channel, calculateMIDINoteValue(index, octave), 100);
      wait_us(5);
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
