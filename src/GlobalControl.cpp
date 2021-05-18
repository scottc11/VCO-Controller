#include "GlobalControl.h"

void GlobalControl::init() {

  metronome->init();

  metronome->attachTickCallback(callback(this, &GlobalControl::tickChannels));

  selectChannel(0);  // select a default channel
}

void GlobalControl::tickChannels() {
  channels[0]->tickClock();
  channels[1]->tickClock();
  channels[2]->tickClock();
  channels[3]->tickClock();
}


void GlobalControl::poll() {
  // if (touchDetected) {
  //   handleTouchEvent();
  //   touchDetected = false;
  // }

  if (timer.read() > 2) {
    calibrateChannel(selectedChannel);
    timer.stop();
    timer.reset();
  }
  
}



/**
 * CHANNEL SELECT
*/
void GlobalControl::selectChannel(int channel) {
  for (int i = 0; i < 4; i++) {
    if (i != channel) {
      channels[i]->isSelected = false;
    }
  }
  
  selectedChannel = channel;
  channels[selectedChannel]->isSelected = true;
}

/**
 * HANDLE TOUCH EVENT
*/
void GlobalControl::handleTouchEvent() {
  
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

/**
 * HANDLE TOUCH TOUCHED
 * 
*/
void GlobalControl::handleTouch(int pad) {
  
  if (handleGesture()) {
    return;
  }
  
  switch (pad) {
    case FREEZE:
      handleFreeze(true);
      break;
    case RESET:
      break;
    case CALIBRATE:
      timer.start();
      break;
    case PB_RANGE:
      channels[0]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[1]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[2]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[3]->enableUIMode(TouchChannel::PB_RANGE_UI);
      break;
    case LOOP_LENGTH:
      channels[0]->enableUIMode(TouchChannel::LOOP_LENGTH_UI);
      channels[1]->enableUIMode(TouchChannel::LOOP_LENGTH_UI);
      channels[2]->enableUIMode(TouchChannel::LOOP_LENGTH_UI);
      channels[3]->enableUIMode(TouchChannel::LOOP_LENGTH_UI);
      break;
    case RECORD:
      if (!recordEnabled) {
        // rec_led.write(1);
        channels[0]->enableLoopMode();
        channels[1]->enableLoopMode();
        channels[2]->enableLoopMode();
        channels[3]->enableLoopMode();
        recordEnabled = true;
      } else {
        // rec_led.write(0);
        channels[0]->disableLoopMode();
        channels[1]->disableLoopMode();
        channels[2]->disableLoopMode();
        channels[3]->disableLoopMode();
        recordEnabled = false;
      }
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
      handleFreeze(false);
      break;
    case RESET:
      break;
    case CALIBRATE:
      timer.stop();
      timer.reset();
      break;
    case PB_RANGE:
      channels[0]->disableUIMode();
      channels[1]->disableUIMode();
      channels[2]->disableUIMode();
      channels[3]->disableUIMode();
      break;
    case LOOP_LENGTH:
      channels[0]->disableUIMode();
      channels[1]->disableUIMode();
      channels[2]->disableUIMode();
      channels[3]->disableUIMode();
      break;
    case RECORD:
      // channels[0]->disableLoopMode();
      // channels[1]->disableLoopMode();
      // channels[2]->disableLoopMode();
      // channels[3]->disableLoopMode();
      break;
    case CTRL_A:
      timer.stop();
      break;
    case CTRL_B:
      break;
    case CTRL_C:
      break;
    case CTRL_D:
      break;
  }
}

bool GlobalControl::handleGesture() {
  // switch (currTouched) {
  //   case RESET_CALIBRATION:
  //     // TODO: there should be some kind of UI signaling successful clear
  //     saveCalibrationToFlash(true);   // reset calibration to default values
  //     loadCalibrationDataFromFlash(); // then load the 'new' values into all the channel instances
  //     return true;
  //   case RESET_LOOP_A:
  //     channels[0]->reset();
  //     return true;
  //   case RESET_LOOP_B:
  //     channels[1]->reset();
  //     return true;
  //   case RESET_LOOP_C:
  //     channels[2]->reset();
  //     return true;
  //   case RESET_LOOP_D:
  //     channels[3]->reset();
  //     return true;
  //   case CLEAR_CH_A_LOOP:
  //     channels[0]->clearLoop();
  //     return true;
  //   case CLEAR_CH_B_LOOP:
  //     channels[1]->clearLoop();
  //     return true;
  //   case CLEAR_CH_C_LOOP:
  //     channels[2]->clearLoop();
  //     return true;
  //   case CLEAR_CH_D_LOOP:
  //     channels[3]->clearLoop();
  //     return true;
  //   case CLEAR_CH_A_PB:
  //     channels[0]->clearPitchBendSequence();
  //     return true;
  //   case CLEAR_CH_B_PB:
  //     channels[1]->clearPitchBendSequence();
  //     return true;
  //   case CLEAR_CH_C_PB:
  //     channels[2]->clearPitchBendSequence();
  //     return true;
  //   case CLEAR_CH_D_PB:
  //     channels[3]->clearPitchBendSequence();
  //     return true;
  //   case CLEAR_SEQ_ALL:
  //     channels[0]->clearLoop();
  //     channels[1]->clearLoop();
  //     channels[2]->clearLoop();
  //     channels[3]->clearLoop();
  //     return true;
  //   default:
  //     return false;
  // }
  return false;
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


void GlobalControl::calibrateChannel(int chan) {
  this->mode = Mode::CALIBRATING;
  calibrator.setChannel(channels[chan]);
  calibrator.enableCalibrationMode();
}

/**
 * every time we calibrate a channel, we need to save all 4 channels data due to the fact 
 * that we must delete/clear an entire sector of data first.
*/ 
void GlobalControl::saveCalibrationToFlash(bool reset /* false */)
{
  
  char16_t buffer[CALIBRATION_LENGTH * 4]; // create array of 16 bit chars to hold ALL 4 channels data

  for (int chan = 0; chan < 4; chan++) {                                                   // iterate through each channel
    for (int i = 0; i < CALIBRATION_LENGTH; i++)
    {
      int index = i + CALIBRATION_LENGTH * chan;                                           // determine falshData index position based on channel
      buffer[index] = reset ? DAC_VOLTAGE_VALUES[i] : channels[chan]->dacVoltageValues[i]; // either reset values to default, or using existing values saved to class
    }
  }
  FlashIAP flash;
  flash.init();
  flash.erase(flashAddr, flash.get_sector_size(flashAddr));     // must erase all data before a write
  flash.program(buffer, flashAddr, 512);                        // number of bytes = CALIBRATION_LENGTH * number of channels * 16 bits / 8 bits
  flash.deinit();
}

void GlobalControl::loadCalibrationDataFromFlash() {
  volatile uint16_t buffer[CALIBRATION_LENGTH * 4];
  FlashIAP flash;
  flash.init();
  flash.read((void *)buffer, flashAddr, 512);
  flash.deinit();
  for (int chan = 0; chan < 4; chan++) {
    for (int i = 0; i < CALIBRATION_LENGTH; i++)
    {
      int index = i + CALIBRATION_LENGTH * chan; // determine falshData index position based on channel
      channels[chan]->dacVoltageValues[i] = buffer[index];
    }
    channels[chan]->generateDacVoltageMap(); // must call this to map, once again, the above values to the 2 dimensional array
  }

}

void GlobalControl::enablePitchBendRangeUI() {
  // disable all channels LED control
  
}
void GlobalControl::disablePitchBendRangeUI() {
  // re-enable channel LED control after
}