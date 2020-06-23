#include "GlobalControl.h"

/**
 * TODO:
 * clear button for loop mode
 * reset button for loop mode
 * freeze button
*/

void GlobalControl::init() {
  touchCtrl->init();
  touchOctAB->init();
  touchOctCD->init();

  if (!touchOctAB->isConnected()) {
    channels[2]->ctrlLed.write(HIGH);
  }

  channels[0]->ctrlLed.write(HIGH);
  selectChannel(0);
}


void GlobalControl::poll() {
  if (touchDetected) {
    handleTouchEvent();
    touchDetected = false;
  }
  handleOctaveTouched();
  // if (octaveTouchDetected) {
    
  //   octaveTouchDetected = false;
  // }
}



/**
 * CHANNEL SELECT
*/
void GlobalControl::selectChannel(int channel) {
  for (int i = 0; i < 4; i++) {
    if (i != channel) {
      channels[i]->ctrlLed.write(LOW);
      channels[i]->isSelected = false;
    }
  }
  
  selectedChannel = channel;
  channels[selectedChannel]->isSelected = true;
  channels[selectedChannel]->ctrlLed.write(HIGH);
}

/**
 * HANDLE TOUCH EVENT
*/
void GlobalControl::handleTouchEvent() {
  currTouched = touchCtrl->touched();
  if (currTouched != prevTouched) {
    for (int i=0; i<8; i++) {
      if (touchCtrl->padIsTouched(i, currTouched, prevTouched)) {
        handleTouch(i);
      }
      if (touchCtrl->padWasTouched(i, currTouched, prevTouched)) {
        handleRelease(i);
      }
    }
    prevTouched = currTouched;
  }
}

void GlobalControl::handleOctaveTouched() {
  // put both touch ICs data into a 16 bit int
  uint8_t touchedAB = touchOctAB->touched();
  uint8_t touchedCD = touchOctCD->touched();
  currOctavesTouched = two8sTo16(touchedCD, touchedAB);
  if (currOctavesTouched != prevOctavesTouched) {
    for (int i=0; i<16; i++) {
      if (touchCtrl->padIsTouched(i, currOctavesTouched, prevOctavesTouched)) {
        switch (currTouched) {
          case 0b00000001: // loop length is currenttly being touched
            setChannelLoopMultiplier(i);
            break;
          case 0b00000000:
            setChannelOctave(i);
            break;
        }
      }
      if (touchCtrl->padWasTouched(i, currOctavesTouched, prevOctavesTouched)) {
        
      }
    }
    prevOctavesTouched = currOctavesTouched;
  }
}


void GlobalControl::setChannelLoopMultiplier(int pad) {
  switch (pad) {
    case 0:  channels[2]->setLoopMultiplier(1); break;
    case 1:  channels[2]->setLoopMultiplier(2); break;
    case 2:  channels[2]->setLoopMultiplier(3); break;
    case 3:  channels[2]->setLoopMultiplier(4); break;
    case 4:  channels[3]->setLoopMultiplier(1); break;
    case 5:  channels[3]->setLoopMultiplier(2); break;
    case 6:  channels[3]->setLoopMultiplier(3); break;
    case 7:  channels[3]->setLoopMultiplier(4); break;
    case 8:  channels[0]->setLoopMultiplier(1); break;
    case 9:  channels[0]->setLoopMultiplier(2); break;
    case 10: channels[0]->setLoopMultiplier(3); break;
    case 11: channels[0]->setLoopMultiplier(4); break;
    case 12: channels[1]->setLoopMultiplier(1); break;
    case 13: channels[1]->setLoopMultiplier(2); break;
    case 14: channels[1]->setLoopMultiplier(3); break;
    case 15: channels[1]->setLoopMultiplier(4); break;
  }
}

void GlobalControl::setChannelOctave(int pad) {
  switch (pad) {
    case 0:  channels[2]->setOctave(0); break;
    case 1:  channels[2]->setOctave(1); break;
    case 2:  channels[2]->setOctave(2); break;
    case 3:  channels[2]->setOctave(3); break;
    case 4:  channels[3]->setOctave(0); break;
    case 5:  channels[3]->setOctave(1); break;
    case 6:  channels[3]->setOctave(2); break;
    case 7:  channels[3]->setOctave(3); break;
    case 8:  channels[0]->setOctave(0); break;
    case 9:  channels[0]->setOctave(1); break;
    case 10: channels[0]->setOctave(2); break;
    case 11: channels[0]->setOctave(3); break;
    case 12: channels[1]->setOctave(0); break;
    case 13: channels[1]->setOctave(1); break;
    case 14: channels[1]->setOctave(2); break;
    case 15: channels[1]->setOctave(3); break;
  }
}

/**
 * HANDLE TOUCH TOUCHED
 * 
*/
void GlobalControl::handleTouch(int pad) {
  
  switch (pad) {
    case FREEZE:
      if (currTouched == _FREEZE) {
        handleFreeze(true);
      }
      break;
    case RESET:
      if (currTouched == CLEAR_LOOP) {
        handleClearLoop();
      } else {
        handleClockReset();
      }
      break;
    case LOOP_LENGTH:
      channels[0]->enableLoopLengthUI();
      channels[1]->enableLoopLengthUI();
      channels[2]->enableLoopLengthUI();
      channels[3]->enableLoopLengthUI();
      break;
    case RECORD:
      channels[0]->enableLoopMode();
      channels[1]->enableLoopMode();
      channels[2]->enableLoopMode();
      channels[3]->enableLoopMode();
      break;
    case CTRL_A:
      selectChannel(0);
      break;
    case CTRL_B:
      selectChannel(1);
      break;
    case CTRL_C:
      selectChannel(2);
      break;
    case CTRL_D:
      selectChannel(3);
      break;
  }
}

/**
 * HANDLE TOUCH RELEASE
*/
void GlobalControl::handleRelease(int pad) {
  switch (pad) {
    case FREEZE:
      if (currTouched == 0x00) {
        handleFreeze(false);
      }
      break;
    case RESET:
      break;
    case LOOP_LENGTH:
      channels[0]->disableLoopLengthUI();
      channels[1]->disableLoopLengthUI();
      channels[2]->disableLoopLengthUI();
      channels[3]->disableLoopLengthUI();
      break;
    case RECORD:
      channels[0]->disableLoopMode();
      channels[1]->disableLoopMode();
      channels[2]->disableLoopMode();
      channels[3]->disableLoopMode();
      break;
    case CTRL_A:
      break;
    case CTRL_B:
      break;
    case CTRL_C:
      break;
    case CTRL_D:
      break;
  }
}

/**
 * HANDLE FREEZE
*/
void GlobalControl::handleFreeze(bool enable) {
  // freeze all channels
  channels[0]->freeze(enable);
  channels[1]->freeze(enable);
  channels[2]->freeze(enable);
  channels[3]->freeze(enable);
}


/**
 * HANDLE RESET
*/
void GlobalControl::handleClockReset() {
  // reset all channels
  channels[0]->reset();
  channels[1]->reset();
  channels[2]->reset();
  channels[3]->reset();
}

/**
 * HANDLE RESET
*/
void GlobalControl::handleClearLoop() {
  channels[selectedChannel]->clearEventLoop();
}


void GlobalControl::setCalibration() {
  channels[selectedChannel]->enableCalibrationMode();
}