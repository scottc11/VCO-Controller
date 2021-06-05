#include "TouchChannel.h"

void TouchChannel::init() {

  for (int i = 0; i < CALIBRATION_LENGTH; i++) {                 // copy default pre-calibrated dac voltage values into class object member
    dacVoltageValues[i] = DAC_VOLTAGE_VALUES[i];
  }

  this->generateDacVoltageMap();

  this->initIOExpander();

  touchPads->init();
  touchPads->attachCallbackTouched(callback(this, &TouchChannel::onTouch));
  touchPads->attachCallbackReleased(callback(this, &TouchChannel::onRelease));
  touchPads->enable();

  dac->init();

  bender.init();
  
  setPitchBendRange(1); // default to a whole tone

  initSequencer(); // must be done after pb calibration

  handleIOInterupt();

  // initialize default variables
  currNoteIndex = 0;
  currOctave = 0;
  touched = 0;
  prevTouched = 0;
  enableQuantizer = false;
  mode = MONO;
  uiMode = DEFAULT_UI;
  setModeLed(LOW);
  this->setOctave(currOctave);
  setGate(LOW);
}


void TouchChannel::initIOExpander() {
  io->init();
  io->setBlinkFrequency(SX1509::FAST);
  io->pinMode(CHANNEL_IO_MODE_PIN, SX1509::INPUT, true);
  io->enableInterupt(CHANNEL_IO_MODE_PIN, SX1509::RISING);

  io->pinMode(CHANNEL_LED_MUX_SEL, SX1509::OUTPUT);
  io->enablePullup(CHANNEL_LED_MUX_SEL);
  io->digitalWrite(CHANNEL_LED_MUX_SEL, 1);
  io->ledConfig(CHANNEL_MODE_LED);
  io->ledConfig(CHANNEL_GATE_LED);

  // initialize IO Led Driver pins
  for (int i = 0; i < 8; i++)
  {
    io->pinMode(CHAN_LED_PINS[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(CHAN_LED_PINS[i], 127);
    io->digitalWrite(CHAN_LED_PINS[i], 1);
  }

  for (int i = 0; i < 4; i++)
  {
    io->pinMode(OCTAVE_LED_PINS[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(OCTAVE_LED_PINS[i], 255);
    io->digitalWrite(OCTAVE_LED_PINS[i], 1);
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

    // if (uiMode == LOOP_LENGTH_UI) {
    //   handleLoopLengthUI();
    // }

    // if (modeChangeDetected) {
    //   this->handleIOInterupt();
    //   modeChangeDetected = false;
    // }

    touchPads->poll();

    if (tickerFlag) {                                                        // every PPQN, read ADCs and update

      bender.poll();

      // triggerNote(currNoteIndex, currOctave, BEND_PITCH);                    // HANDLE PITCH BEND

      // if ((mode == QUANTIZE || mode == QUANTIZE_LOOP) && enableQuantizer)    // HANDLE CV QUANTIZATION
      // {
      //   currCVInputValue = cvInput.read_u16();
      //   if (gateState == HIGH) setGate(LOW);   // We only want trigger events in quantizer mode, so if the gate gets set HIGH, make sure to set it back to low the very next tick

      //   if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER)
      //   {
      //     handleCVInput(currCVInputValue);
      //     prevCVInputValue = currCVInputValue;
      //   }
      // }

      
      // if ((mode == MONO_LOOP || mode == QUANTIZE_LOOP) && enableLoop)        // HANDLE SEQUENCE
      // {
      //   handleSequence(currPosition);
      // }

      tickerFlag = false;
    }
  }
}
// ------------------------------------------------------------------------


void TouchChannel::clearLoop() {
  if (this->sequenceContainsEvents) {
    this->clearEventSequence();
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

  if (sequenceContainsEvents) {   // if a touch event was recorded, remain in loop mode
    recordEnabled = false;
    return;
  } else {             // if no touch event recorded, revert to previous mode
    recordEnabled = false;
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

void TouchChannel::onTouch(uint8_t pad) {
  
  if (pad < 8) {
    pad = CHAN_TOUCH_PADS[pad];
    if (uiMode == DEFAULT_UI)
    {
      switch (mode)
      {
      case MONO:
        triggerNote(pad, currOctave, ON);
        break;
      case QUANTIZE:
        /**
       * TODO: start timer for setting max active degrees
       * RESET will now reset activeDegreeLimit to its max value of 8
      */
        setActiveDegrees(bitWrite(activeDegrees, pad, !bitRead(activeDegrees, pad)));
        break;
      case QUANTIZE_LOOP:
        // every touch detected, take a snapshot of all active degree values and apply them to a EventNode
        setActiveDegrees(bitWrite(activeDegrees, pad, !bitRead(activeDegrees, pad)));
        createChordEvent(currPosition, activeDegrees);
        break;
      case MONO_LOOP:
        clearExistingNodes = true;
        createEvent(currPosition, pad, HIGH);
        triggerNote(pad, currOctave, ON);
        break;
      }
    }
    else
    { // LOOP_LENGTH_UI mode
      switch (uiMode)
      {
      case LOOP_LENGTH_UI:
        setLoopLength(pad + 1); // loop length is not zero indexed
        break;
      case PB_RANGE_UI:
        setPitchBendRange(pad);
        updatePitchBendRangeUI();
        break;
      }
    }
  } else {
    pad = CHAN_TOUCH_PADS[pad];
    setOctave(pad);
  }
}

void TouchChannel::onRelease(uint8_t pad) {
  
  if (uiMode == DEFAULT_UI)
  {
    if (pad < 8) {
      pad = CHAN_TOUCH_PADS[pad];
      switch (mode)
      {
      case MONO:
        triggerNote(pad, currOctave, OFF);
        break;
      case QUANTIZE:
        // set end time
        // if (endTime - startTime > gestureThreshold) do something fancy
        break;
      case QUANTIZE_LOOP:
        break;
      case MONO_LOOP:
        createEvent(currPosition, pad, LOW);
        triggerNote(pad, currOctave, OFF);
        clearExistingNodes = false;
        // create note OFF event
        // enableLoop = true;
        break;
      }
    } else {
      setGate(LOW);
    }
  }
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------

/**
 * TOGGLE MODE
 * 
 * still needs to be written to handle 3-stage toggle switch.
**/
void TouchChannel::handleIOInterupt() {
  if (io->digitalRead(CHANNEL_IO_MODE_PIN) == HIGH) {
    if (mode == MONO || mode == MONO_LOOP) {
      setMode(QUANTIZE);
    }
    else {
      setMode(MONO);
    }
  }
}

void TouchChannel::setMode(ChannelMode targetMode)
{
  prevMode = mode;
  switch (targetMode) {
    case MONO:
      enableLoop = false;
      enableQuantizer = false;
      mode = MONO;
      setAllLeds(LOW);               // I think this is just a "start from a clean slate" kinda thing
      setAllLeds(DIM_HIGH);
      updateOctaveLeds(currOctave);
      setModeLed(LOW);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case MONO_LOOP:
      enableLoop = true;
      enableQuantizer = false;
      mode = MONO_LOOP;
      setAllLeds(LOW);
      setAllLeds(DIM_LOW);
      updateOctaveLeds(currOctave);
      setModeLed(HIGH);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      if (!quantizerHasBeenInitialized) { initQuantizerMode(); }
      enableLoop = false;
      enableQuantizer = true;
      mode = QUANTIZE;
      setAllLeds(LOW);
      setAllLeds(DIM_HIGH);
      updateActiveDegreeLeds();
      updateOctaveLeds(activeOctaves);
      setModeLed(LOW);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE_LOOP:
      enableLoop = true;
      enableQuantizer = true;
      mode = QUANTIZE_LOOP;
      setAllLeds(LOW);
      setAllLeds(DIM_LOW);
      updateActiveDegreeLeds();
      setModeLed(HIGH);
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

void TouchChannel::updateDegrees() {
  switch (mode) {
    case MONO:
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      break;
    case MONO_LOOP:
      break;
  }
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
    case DIM_LOW:
      for (int i = 0; i < 8; i++) setLed(i, DIM_LOW);
      break;
    case DIM_MEDIUM:
      for (int i = 0; i < 8; i++) setLed(i, DIM_MEDIUM);
      break;
    case DIM_HIGH:
      for (int i = 0; i < 8; i++) setLed(i, DIM_HIGH);
      break;
  }
}

void TouchChannel::setLed(int index, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {

    switch (state) {
      case LOW:
        ledStates &= ~(1 << index);
        io->setOnTime(CHAN_LED_PINS[index], 0);
        io->digitalWrite(CHAN_LED_PINS[index], 1);
        break;
      case HIGH:
        ledStates |= 1 << index;
        io->setOnTime(CHAN_LED_PINS[index], 0);
        io->digitalWrite(CHAN_LED_PINS[index], 0);
        break;
      case BLINK_ON:
        ledStates |= 1 << index;
        io->blinkLED(CHAN_LED_PINS[index], 1, 2, 127, 0);
        break;
      case BLINK_OFF:
        io->setOnTime(CHAN_LED_PINS[index], 0);
        break;
      case DIM_LOW:
        io->setPWM(CHAN_LED_PINS[index], 10);
        break;
      case DIM_MEDIUM:
        io->setPWM(CHAN_LED_PINS[index], 30);
        break;
      case DIM_HIGH:
        io->setPWM(CHAN_LED_PINS[index], 70);
        break;
    }
  }
}


void TouchChannel::setOctaveLed(int octave, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {  // this is how you keep sequences going whilst "blocking" the sequence from changing any LEDs when a UI mode is active
    switch (state) {
      case LOW:
        io->setOnTime(OCTAVE_LED_PINS[octave], 0);     // reset any blinking state
        io->digitalWrite(OCTAVE_LED_PINS[octave], 1);
        break;
      case HIGH:
        io->setOnTime(OCTAVE_LED_PINS[octave], 0);     // reset any blinking state
        io->digitalWrite(OCTAVE_LED_PINS[octave], 0);
        break;
      case BLINK_ON:
        io->blinkLED(OCTAVE_LED_PINS[octave], 1, 1, 255, 0);
        break;
      case DIM_LOW:
        io->analogWrite(OCTAVE_LED_PINS[octave], 70);
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
      setGate(HIGH);
      setGlobalGate(HIGH);
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case SUSTAIN:
      prevOctave = currOctave;       // you might need to remove all this setter.
      prevNoteIndex = currNoteIndex; // you might need to remove all this setter.
      currNoteIndex = index;
      currOctave = octave;
      setLed(index, HIGH);
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case OFF:
      setGate(LOW);
      setGlobalGate(LOW);
      midi->sendNoteOff(channel, calculateMIDINoteValue(index, octave), 100);
      // wait_us(1);
      break;
    case PREV:
      setLed(index, HIGH);
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case BEND_PITCH:
      dac->write(dacChannel, calculateDACNoteValue(index, octave));
      break;
  }
}




int TouchChannel::calculateDACNoteValue(int index, int octave)
{
  return dacVoltageMap[index + DAC_OCTAVE_MAP[octave]][degrees->switchStates[index]] + (bender.mode == PITCH_BEND ? pbNoteOffset : 0);
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


/**
 * this function takes a 1D array and converts it into a 2D array formatted as [[0, 1, 2], ...]
 * take the first 12 values from dacVoltageValues. find the difference dacVoltageValues[i]
*/
void TouchChannel::generateDacVoltageMap()
{
  int octaveIndexes[4] = {0, 12, 24, 36};
  int multiplier = 1;

  for (int oct = 0; oct < 4; oct++)
  {
    int index = octaveIndexes[oct];
    int limit = 8 * multiplier;
    for (int i = limit - 8; i < limit; i++)
    {
      dacVoltageMap[i][0] = dacVoltageValues[index];
      dacVoltageMap[i][1] = dacVoltageValues[index + 1];
      dacVoltageMap[i][2] = dacVoltageValues[index + 2];
      index += 2;
    }
    multiplier += 1;
  }
}

void TouchChannel::setGlobalGate(bool state) {
  globalGateOut->write(state);
}

void TouchChannel::setGate(bool state)
{
  gateState = state;
  gateOut.write(state);
  setGateLed(state ? HIGH : LOW);
}

void TouchChannel::setModeLed(LedState state) {
  if (state == HIGH)
  {
    io->analogWrite(CHANNEL_MODE_LED, 20);
  }
  else
  {
    io->analogWrite(CHANNEL_MODE_LED, 0);
  }
}
void TouchChannel::setGateLed(LedState state) {
  if (state == HIGH)
  {
    io->analogWrite(CHANNEL_GATE_LED, 255);
  }
  else
  {
    io->analogWrite(CHANNEL_GATE_LED, 0);
  }
}

/**
 * apply the pitch bend by mapping the ADC value to a value between PB Range value and the current note being outputted
*/
void TouchChannel::benderActiveCallback()
{
  // if (mode == MONO_LOOP || mode == QUANTIZE_LOOP)
  // {
  //   if (currBend > adcZero + this->debounceRange || currBend < adcZero - this->debounceRange)
  //   {
  //     if (recordEnabled) // record pitch bend and use new value
  //     {
  //       createPitchBendEvent(currPosition, currBend);
  //       setPitchBendOffset(events[currPosition].pitchBend);
  //     }
  //     else
  //     {
  //       setPitchBendOffset(currBend);
  //     }
  //   }
  //   else
  //   {
  //     setPitchBendOffset(events[currPosition].pitchBend);
  //   }
  // }
  // else
  // {
  //   setPitchBendOffset(currBend);
  // }
}

void TouchChannel::benderIdleCallback() {
  
}

/**
 * Set the pitch bend range to be applied to 1v/o output
 * NOTE: There are 12 notes, but only 8 possible PB range options, meaning there are preset values for each PB range option via PB_RANGE_MAP global
 * value: num with range 0..7
*/
void TouchChannel::setPitchBendRange(int touchedIndex)
{
  // pbRangeIndex = touchedIndex;
  // pbOffsetRange = dacSemitone * PB_RANGE_MAP[pbRangeIndex]; // map 0..7 ranged value to preset pitch bend ranges
}

void TouchChannel::setPitchBendOffset(uint16_t value)
{
  if (bender.isIdle()) // may be able to move this line into handlevalue()
  {
    if (value > bender.zeroBend && value < bender.maxBend)
    {
      pbNoteOffset = ((pbOffsetRange / (bender.maxBend - bender.zeroBend)) * (value - bender.zeroBend)) * -1; // inverted
    }
    else if (value < bender.zeroBend && value > bender.minBend)
    {
      pbNoteOffset = ((pbOffsetRange / (bender.minBend - bender.zeroBend)) * (value - bender.zeroBend)) * 1; // non-inverted
    }
  }
  else
  {
    pbNoteOffset = 0;
  }
}

int TouchChannel::setBenderMode(int targetMode /*0*/)
{
  if (bender.mode < 3)
  {
    bender.mode += 1;
  }
  else
  {
    bender.mode = 0;
  }
  return bender.mode;
}