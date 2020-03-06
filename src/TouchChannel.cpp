#include "TouchChannel.h"

void TouchChannel::init() {
  touch->init();
  
  if (!touch->isConnected()) {
    this->updateLeds(0xFF);
    return;
  }
  touch->calibrate();
  touch->clearInterupt();

  io->init();
  io->setDirection(MCP23017_PORTA, 0x00);           // set all of the PORTA pins to output
  io->setDirection(MCP23017_PORTB, 0b00001111);     // set PORTB pins 0-3 as input, 4-7 as output
  io->setPullUp(MCP23017_PORTB, 0b00001111);        // activate PORTB pin pull-ups for toggle switches
  io->setInputPolarity(MCP23017_PORTB, 0b00000000); // invert PORTB pins input polarity for toggle switches
  io->setInterupt(MCP23017_PORTB, 0b00001111);

  handleSwitchInterupt();

  for (int i = 0; i < 8; i++) {
    this->updateLeds(leds[i]);
    wait_ms(25);
  }
  this->updateLeds(0x00);
  this->setOctaveLed(currOctave);

  dac->referenceMode(dacChannel, MCP4922::REF_UNBUFFERED);
  dac->gainMode(dacChannel, MCP4922::GAIN_1X);
  dac->powerMode(dacChannel, MCP4922::POWER_NORMAL);

  currNoteIndex = 0;
  currOctave = 0;
  dac->write_u12(dacChannel, calculateDACNoteValue(currNoteIndex, currOctave));

  this->initQuantizer();

}

// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  if (touchDetected) {
    handleTouch();
    touchDetected = false;
  }

  if (switchHasChanged) {
    handleSwitchInterupt();
  }

  if (degrees->hasChanged[channel]) {
    handleDegreeChange();
  }

  if (mode == QUANTIZE || mode == QUANTIZE_LOOP) {
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
            // no need to set any quantizer variables here, as they will be set later by the queued event loop
            enableLoop = false;
            setActiveDegrees(bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
            createChordEvent(currPosition, bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
            break;
          case MONO_LOOP:
            enableLoop = false;
            createEvent(currPosition, i);
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
            addEventToList(currPosition);
            enableLoop = true;
            break;
          case MONO_LOOP:
            addEventToList(currPosition);
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
  return io->digitalRead(MCP23017_PORTB) & 0xF;
}

void TouchChannel::writeLed(int index, int state) {
  switch (state) {
    case HIGH:
      ledStates |= 1 << index;
      io->digitalWrite(MCP23017_PORTA, ledStates);
      break;
    case LOW:
      ledStates &= ~(1 << index);
      io->digitalWrite(MCP23017_PORTA, ledStates);
      break;
  }
  
}

void TouchChannel::updateLeds(uint8_t touched) {
  io->digitalWrite(MCP23017_PORTA, touched);
}

void TouchChannel::setOctaveLed(int octave) {
  int state = 1 << (octave + 4);
  io->digitalWrite(MCP23017_PORTB, state);
}

void TouchChannel::triggerNote(int index, int octave, NoteState state) {
  switch (state) {
    case ON:
      // if mideNoteState == ON, midi->sendNoteOff(prevNoteIndex, prevOctave)
      if (mode == MONO || mode == MONO_LOOP) {
        writeLed(prevNoteIndex, LOW);
        writeLed(index, HIGH);
      }
      currNoteIndex = index;
      currOctave = octave;
      gateOut.write(HIGH);
      dac->write_u12(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case OFF:
      switch (mode) {
        case MONO_LOOP:
          writeLed(index, LOW);
          break;
        default:
          break;
      }
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
      // turn quantizer off
      break;
    case MONO_LOOP:
      if (freeze == true) {
        enableLoop = false;
      } else {
        enableLoop = true;
      }
      break;
  }
}

void TouchChannel::reset() {
  switch (mode) {
    case MONO:
      break;
    case QUANTIZE:
      break;
    case MONO_LOOP:
      // NOTE: you probably don't want to reset the 'tick' value, as it would make it very dificult to line up with the global clock;
      currPosition = 0;
      currStep = 1;
      break;
  }
}
