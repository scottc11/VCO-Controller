#include "TouchChannel.h"

void TouchChannel::init() {

  for (int i = 0; i < CALIBRATION_LENGTH; i++) {                 // copy default pre-calibrated dac voltage values into class object member
    dacVoltageValues[i] = DAC_VOLTAGE_VALUES[i];
  }

  this->generateDacVoltageMap();

  touch->init();

  this->initIOExpander();

  if (!touch->isConnected()) { this->updateOctaveLeds(3); return; }
  touch->calibrate();
  touch->clearInterupt();
  dac->init();

  pb_dac->init();
  calibratePitchBend();
  updatePitchBendDAC(0);

  // intialize inheritance variables
  numLoopSteps = DEFAULT_CHANNEL_LOOP_STEPS;
  loopMultiplier = 1;
  timeQuantizationMode = QUANT_NONE;
  currStep = 0;
  currTick = 0;
  currPosition = 0;
  setLoopTotalSteps();
  setLoopTotalPPQN();

  toggleMode();

  // initialize default variables
  currNoteIndex = 0;
  currOctave = 0;
  touched = 0;
  prevTouched = 0;
  enableQuantizer = false;
  mode = MONO;
  uiMode = DEFAULT_UI;

  this->setOctave(currOctave);
}


void TouchChannel::initIOExpander() {
  io->init();
  io->setBlinkFrequency(SX1509::FAST);
  io->pinMode(CHANNEL_IO_MODE_PIN, SX1509::INPUT, true);
  io->enableInterupt(CHANNEL_IO_MODE_PIN, SX1509::RISING);

  io->pinMode(CHANNEL_IO_TOGGLE_PIN_1, SX1509::INPUT);
  io->pinMode(CHANNEL_IO_TOGGLE_PIN_2, SX1509::INPUT);
  io->enableInterupt(CHANNEL_IO_TOGGLE_PIN_1, SX1509::RISE_FALL);
  io->enableInterupt(CHANNEL_IO_TOGGLE_PIN_2, SX1509::RISE_FALL);

  // initialize IO Led Driver pins
  for (int i = 0; i < 8; i++)
  {
    io->pinMode(chanLedPins[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(chanLedPins[i], 127);
    io->digitalWrite(chanLedPins[i], 1);
  }

  for (int i = 0; i < 4; i++)
  {
    io->pinMode(octaveLedPins[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(octaveLedPins[i], 255);
    io->digitalWrite(octaveLedPins[i], 1);
  }
}


/** ------------------------------------------------------------------------
 *         POLL    POLL    POLL    POLL    POLL    POLL    POLL    POLL    
---------------------------------------------------------------------------- */
// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  
  if (!freezeChannel) { // don't do anything if freeze enabled
    
    // Timer polling --> flag if timer is active, and if it is start counting to 3 seconds
    // after 3 seconds, call a function which takes the currTouched variable and applies it to the activeDegreeLimit.
    // then disable timer poll flag.

    if (uiMode == LOOP_LENGTH_UI) {
      handleLoopLengthUI();
    }

    if (touchDetected) {
      handleTouch();
      touchDetected = false;
    }


    if (modeChangeDetected) {
      this->toggleMode();
      modeChangeDetected = false;
    }

    if (degrees->hasChanged[channel]) {
      handleDegreeChange();
    }

    if (tickerFlag) {                                                        // every PPQN, read ADCs and update

      triggerNote(currNoteIndex, currOctave, PITCH_BEND);                    // HANDLE PITCH BEND

      if ((mode == QUANTIZE || mode == QUANTIZE_LOOP) && enableQuantizer)    // HANDLE CV QUANTIZATION
      {
        currCVInputValue = cvInput.read_u16();
        if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER)
        {
          handleCVInput(currCVInputValue);
          prevCVInputValue = currCVInputValue;
        }
      }

      
      if ((mode == MONO_LOOP || mode == QUANTIZE_LOOP) && enableLoop)        // HANDLE SEQUENCE
      {
        handleQueuedEvent(currPosition);
      }

      tickerFlag = false;
    }
  }
}
// ------------------------------------------------------------------------


void TouchChannel::handleQueuedEvent(int position) {
  if (events[position].active) {
    if (events[position].triggered == false) {
      events[prevEventIndex].triggered = false;
      events[position].triggered = true;
      prevEventIndex = position;
      switch (mode) {
        case MONO_LOOP:
          triggerNote(prevNoteIndex, currOctave, OFF);
          triggerNote(events[position].noteIndex, currOctave, ON);
          break;
        case QUANTIZE_LOOP:
          setActiveDegrees(events[position].activeNotes);
          break;
      }
    }
  } else {
    if (events[prevEventIndex].triggered) {
      events[prevEventIndex].triggered = false;
      triggerNote(prevNoteIndex, currOctave, OFF);
    }
  }

  // always handle pitch bend value
  triggerNote(currNoteIndex, currOctave, PITCH_BEND);

}



/** ------------------------------------------------------------------------
 *         LOOP UI METHODS
---------------------------------------------------------------------------- */

void TouchChannel::enableLoopLengthUI() {
  uiMode = LOOP_LENGTH_UI;
  updateLoopLengthUI();
}

void TouchChannel::disableLoopLengthUI() {
  uiMode = DEFAULT_UI;
  setAllLeds(LOW);
  if (mode == MONO || mode == MONO_LOOP) {
    updateOctaveLeds(currOctave);
    triggerNote(currNoteIndex, currOctave, PREV);
  } else {
    updateActiveDegreeLeds(); // TODO: additionally set active note to BLINK_ON
  }
}

void TouchChannel::updateLoopLengthUI() {
  setAllLeds(LOW);
  updateLoopMultiplierLeds();
  for (int i = 0; i < numLoopSteps; i++) {
    if (i == currStep) {
      setLed(i, BLINK_ON, true);
    } else {
      setLed(i, HIGH, true);
    }
  }
}

void TouchChannel::handleLoopLengthUI() {
  // take current clock step and flash the corrosponding channel LED and Octave LED
  if (currTick == 0) {
    int modulo = currStep % numLoopSteps;
    
    if (modulo != 0) {           // setting the previous LED back to normal
      setLed(modulo - 1, HIGH, true);
    } else {                     // when modulo rolls past 7 and back to 0
      setLed(numLoopSteps - 1, HIGH, true);
    }

    setLed(modulo, BLINK_ON, true);
    
    for (int i = 0; i < loopMultiplier; i++) {
      if (currStep < (numLoopSteps * (i + 1)) && currStep >= (numLoopSteps * i)) {
        setOctaveLed(i, BLINK_ON, true);
      } else {
        setOctaveLed(i, HIGH, true);
      }
    }
  }
}


void TouchChannel::clearLoop() {
  if (this->loopContainsEvents) {
    this->clearEventLoop();
    setMode(prevMode);
  }
}

void TouchChannel::setLoopLength(int value) {
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
  recordEnabled = true;
  if (mode == MONO) {
    setMode(MONO_LOOP);
  } else if (mode == QUANTIZE) {
    setMode(QUANTIZE_LOOP);
  }
}

void TouchChannel::disableLoopMode() {

  // a nice feature here would be to only have the LEDs be red when REC is held down, and flash the green LEDs 
  // when a channel contains loop events, but REC is NOT held down. You would only be able to add new events to 
  // the loop when REC is held down (ie. when channel leds are RED)

  // ADDITIONALLY, this would be a good place to count the amount of steps which have passed while the REC button has
  // been held down, and if this value is greater than the current loop length, update the loop length to accomodate.
  // the new loop length would just increase the multiplier by one

  if (loopContainsEvents) {   // if a touch event was recorded, remain in loop mode
    return;
  } else {             // if no touch event recorded, revert to previous mode
    setMode(prevMode);
  }
  recordEnabled = false;
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
  
  // when currTick exceeds PPQN, reset to 0
  if (currTick >= PPQN) {
    currTick = 0;
    this->stepClock();
  }
  if (currPosition >= totalPPQN) {
    currPosition = 0;
    currStep = 0;
  }
  tickerFlag = true;
}

void TouchChannel::stepClock() {
  currStep += 1;

  if (currStep >= totalSteps) {  // when currStep eqauls number of steps in loop, reset currStep and currPosition to 0
    currStep = 0;
  }
}

// NOTE: you probably don't want to reset the 'tick' value, as it would make it very dificult to line up with the global clock;
void TouchChannel::resetClock() {
  currTick = 0;
  currPosition = 0;
  currStep = 0;
}

/** -------------------------------------------------------------------------------------------
 *         TOUCH EVENT METHODS           TOUCH EVENT METHODS            TOUCH EVENT METHODS
----------------------------------------------------------------------------------------------- */

// NOTE: you need a way to trigger events after a series of touches have happened, and the channel is now not being touched

void TouchChannel::handleTouch() {
  touched = touch->touched();
  if (touched != prevTouched) {
    for (int i=0; i<8; i++) {
      // if it *is* touched and *wasnt* touched before, alert!
      if (touch->getBitStatus(touched, i) && !touch->getBitStatus(prevTouched, i)) {
        if (uiMode == DEFAULT_UI) {
          switch (mode) {
            case MONO:
              triggerNote(i, currOctave, ON);
              break;
            case QUANTIZE:
              /**
               * TODO: start timer for setting max active degrees
               * RESET will now reset activeDegreeLimit to its max value of 8
              */
              setActiveDegrees(bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
              break;
            case QUANTIZE_LOOP:
              // every touch detected, take a snapshot of all active degree values and apply them to a EventNode
              setActiveDegrees(bitWrite(activeDegrees, i, !bitRead(activeDegrees, i)));
              createChordEvent(currPosition, activeDegrees);
              break;
            case MONO_LOOP:
              createEvent(currPosition, i);
              triggerNote(i, currOctave, ON);
              break;
          }
        }
        else { // LOOP_LENGTH_UI mode
          setLoopLength(i + 1); // loop length is not zero indexed
        }
      }
      // if it *was* touched and now *isnt*, alert!
      if (!touch->getBitStatus(touched, i) && touch->getBitStatus(prevTouched, i)) {
        
        if (uiMode == DEFAULT_UI) {
          switch (mode) {
            case MONO:
              triggerNote(i, currOctave, OFF);
              break;
            case QUANTIZE:
              // set end time
              // if (endTime - startTime > gestureThreshold) do something fancy
              // 
              break;
            case QUANTIZE_LOOP:
              break;
            case MONO_LOOP:
              // triggerNote(i, currOctave, OFF);
              // enableLoop = true;
              break;
          }
        }
      }
    }
    prevTouched = touched;
  }
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------

/**
 * TOGGLE MODE
 * 
 * still needs to be written to handle 3-stage toggle switch.
**/
void TouchChannel::toggleMode() {
  if (io->digitalRead(CHANNEL_IO_MODE_PIN) == HIGH) {
    if (mode == MONO || mode == MONO_LOOP) {
      setMode(QUANTIZE);
    }
    else {
      setMode(MONO);
    }
  }
  uint8_t state = io->readBankA();
  state = (state & 0b11000000) >> 6;
  if (state == 2 || state == 3) {
    pbEnabled = !pbEnabled;
  } else {
    pbEnabled = false;
  }
}

void TouchChannel::setMode(Mode targetMode) {
  prevMode = mode;
  switch (targetMode) {
    case MONO:
      enableLoop = false;
      enableQuantizer = false;
      mode = MONO;
      setAllLeds(LOW);               // I think this is just a "start from a clean slate" kinda thing
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, ON);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case MONO_LOOP:
      enableLoop = true;
      enableQuantizer = false;
      mode = MONO_LOOP;
      setAllLeds(LOW);
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, ON);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE:
      if (!quantizerHasBeenInitialized) { initQuantizerMode(); }
      enableLoop = false;
      enableQuantizer = true;
      mode = QUANTIZE;
      setAllLeds(LOW);
      updateActiveDegreeLeds();
      updateOctaveLeds(activeOctaves);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE_LOOP:
      enableLoop = true;
      enableQuantizer = true;
      mode = QUANTIZE_LOOP;
      setAllLeds(LOW);
      updateActiveDegreeLeds();
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
  }
}

/**
 * take a number between 0 - 3 and apply to currOctave
**/
void TouchChannel::setOctave(int value) {
  
  currOctave = value;

  switch (mode) {
    case MONO:
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, ON);
      break;
    case MONO_LOOP:
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      setActiveOctaves(value);
      break;
    case QUANTIZE_LOOP:
      setActiveOctaves(value);
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
      io->writeBankB(0x00);
      break;
    case LOW:
      io->writeBankB(0xFF);
      break;
  }
}

void TouchChannel::setLed(int index, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {

    switch (state) {
      case LOW:
        ledStates &= ~(1 << index);
        io->setOnTime(chanLedPins[index], 0);
        io->digitalWrite(chanLedPins[index], 1);
        break;
      case HIGH:
        ledStates |= 1 << index;
        io->setOnTime(chanLedPins[index], 0);
        io->digitalWrite(chanLedPins[index], 0);
        break;
      case BLINK_ON:
        ledStates |= 1 << index;
        io->blinkLED(chanLedPins[index], 1, 2, 127, 0);
        break;
      case BLINK_OFF:
        io->setOnTime(chanLedPins[index], 0);
        break;
      case DIM:
        io->setPWM(chanLedPins[index], 10);
        break;
    }
  }
}


void TouchChannel::setOctaveLed(int octave, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {
    switch (state) {
      case LOW:
        io->setOnTime(octaveLedPins[octave], 0);     // reset any blinking state
        io->digitalWrite(octaveLedPins[octave], 1);
        break;
      case HIGH:
        io->setOnTime(octaveLedPins[octave], 0);     // reset any blinking state
        io->digitalWrite(octaveLedPins[octave], 0);
        break;
      case BLINK_ON:
        io->blinkLED(octaveLedPins[octave], 1, 1, 255, 0);
        break;
      case DIM:
        io->analogWrite(octaveLedPins[octave], 70);
        break;
    }
  }
}

void TouchChannel::updateLeds(uint8_t touched) {
  
}

void TouchChannel::updateOctaveLeds(int octave) {
  if (mode == MONO || mode == MONO_LOOP) {
    for (int i = 0; i < OCTAVE_COUNT; i++) {
      if (i == octave) {
        setOctaveLed(i, HIGH);
      } else {
        setOctaveLed(i, LOW);
      }
    }
  } else {
    numActiveOctaves = 0;
    for (int i = 0; i < OCTAVE_COUNT; i++) {
      if (bitRead(activeOctaves, i)) {
        setOctaveLed(i, HIGH);
        activeOctaveValues[numActiveOctaves].octave = i;
        numActiveOctaves += 1;
      } else {
        setOctaveLed(i, LOW);
      }
    }
    
  }
}

void TouchChannel::updateLoopMultiplierLeds() {
  for (int i = 0; i < 4; i++) {
    if (i < loopMultiplier) {  // loopMultiplier is not zero indexed
      setOctaveLed(i, HIGH, true);
    } else {
      setOctaveLed(i, LOW, true);
    }
  }
}


/** ------------------------------------------------------------------------
 *        TRIGGER NOTE
---------------------------------------------------------------------------- */
 
void TouchChannel::triggerNote(int index, int octave, NoteState state, bool blinkLED /* false */) {
  switch (state) {
    case ON:
      if (mode == MONO || mode == MONO_LOOP) {
        setLed(currNoteIndex, LOW);  // set the 'previous' active note led LOW
        setLed(index, HIGH);         // new active note HIGH
      }
      if (blinkLED) setLed(index, BLINK_ON);
      
      prevOctave = currOctave;       // the following two lines of code used to be outside the switch block
      prevNoteIndex = currNoteIndex;
      currNoteIndex = index;
      currOctave = octave;
      gateOut.write(HIGH);
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case SUSTAIN:
      prevOctave = currOctave;       // you might need to remove all this setter.
      prevNoteIndex = currNoteIndex; // you might need to remove all this setter.
      currNoteIndex = index;
      currOctave = octave;
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
    case PITCH_BEND:
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      break;
  }
}




int TouchChannel::calculateDACNoteValue(int index, int octave)
{
  calculatePitchBend();
  uint16_t offset;
  uint16_t cv;

  if (mode == MONO_LOOP || mode == QUANTIZE_LOOP) {   // IF LOOPER MODE APPLIED
    offset = events[currPosition].pitchBend;          // retreive pbNoteOffset and cvOffset values from sequence struct array
    cv = events[currPosition].cvOutput;
  } else {
    offset = pbNoteOffset;                            // business as usual
    cv = cvOffset;
  }

  updatePitchBendDAC(cv);

  return dacVoltageMap[ index + DAC_OCTAVE_MAP[octave] ][ degrees->switchStates[index] ] + offset;
}



int TouchChannel::calculateMIDINoteValue(int index, int octave) {
  return MIDI_NOTE_MAP[index][degrees->switchStates[index]] + MIDI_OCTAVE_MAP[octave];
}




/**
 * FREEZE
 * takes boolean to either freeze or unfreeze
 * NOTE: a good way to freeze everything would be to just change the current mode to FREEZE, and then everything in the POLL fn would not execute.
*/ 
void TouchChannel::freeze(bool freeze) {
  this->freezeChannel = freeze;
}

void TouchChannel::reset() {
  // you should probably get the currently queued event, see if it has been triggered yet, and disable it if it has been triggered
  switch (mode) {
    case MONO:
      resetClock();
      break;
    case QUANTIZE_LOOP:
      resetClock();
      break;
    case MONO_LOOP:
      resetClock();
      break;
  }
}

void TouchChannel::calibratePitchBend() {

  // apply op-amp gain via digi pot
  digiPot->setWiper(digiPotChan, 255); // max gain
  wait_us(1000); // wait for things to settle

  // NOTE: this calibration process is currently flawed, because in the off chance there is an erratic
  // sensor ready in the positive or negative direction, the min / max values used to determine the debounce 
  // value would be to far apart, giving a poor debounce. Additionally, pbZero would also not be very accurate due to these
  // reading. I actually think some DSP smoothing is necessary here, to remove the "noise". Or perhaps just adding debounce caps
  // on the hardware will help this problem.

  // populate calibration array
  for (int i = 0; i < PB_CALIBRATION_RANGE; i++)
  {
    pbCalibration[i] = pbInput.read_u16();
    wait_us(1000);
  }

  // find min/max value from calibration results
  int max = arr_max(pbCalibration, PB_CALIBRATION_RANGE);
  int min = arr_min(pbCalibration, PB_CALIBRATION_RANGE);
  pbDebounce = (max - min);

  // zero the sensor
  pbZero = arr_average(pbCalibration, PB_CALIBRATION_RANGE);

  int minMaxOffset = 10000;
  pbMax = pbZero + minMaxOffset < 65000 ? pbZero + minMaxOffset : 65000;
  pbMin = pbZero - minMaxOffset < 500 ? pbZero - minMaxOffset: 500;

}


/**
 * apply the pitch bend by mapping the ADC value to a value between PB Range value and the current note being outputted
*/
void TouchChannel::calculatePitchBend() {
  
  currPitchBend = pbInput.read_u16();
  if (currPitchBend > pbZero + pbDebounce || currPitchBend < pbZero - pbDebounce)
  {
    float voOutputRange = dacSemitone * pbNoteOffsetRange;
    float rawOutputRange = (32767 / 8) * pbOutputRange;
    if (currPitchBend > pbZero && currPitchBend < pbMax)
    {
      pbNoteOffset = pbEnabled ? ((voOutputRange / (pbMax - pbZero)) * (currPitchBend - pbZero)) * -1 : 0; // inverted
      cvOffset = ((rawOutputRange / (pbMax - pbZero)) * (currPitchBend - pbZero)) * 1;                     // non-inverted
    }
    else if (currPitchBend < pbZero && currPitchBend > pbMin)
    {
      pbNoteOffset = pbEnabled ? ((voOutputRange / (pbMin - pbZero)) * (currPitchBend - pbZero)) * 1 : 0; // non-inverted
      cvOffset = ((rawOutputRange / (pbMin - pbZero)) * (currPitchBend - pbZero)) * -1;                   // inverted
    }
    
    // IF record is enabled, record ALL pitch bend values into the sequencer struct array
    if (recordEnabled) {
      events[currPosition].pitchBend = pbNoteOffset;
      events[currPosition].cvOutput = cvOffset;
    }
  }
}



void TouchChannel::updatePitchBendDAC(uint16_t value)
{
  int zero = 32767;
  pb_dac->write(pb_dac_chan, zero + value);
}