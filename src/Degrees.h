#ifndef __DEGREES_H
#define __DEGREES_H

#include "main.h"
#include "MCP23017.h"

class Degrees {
  public:

    MCP23017 * io;
    InterruptIn ioInterupt;
    bool interuptDetected;
    bool hasChanged[4];
    uint16_t currState;
    uint16_t prevState;

    int switchStates[8];

    Degrees(PinName ioIntPin, MCP23017 *io_ptr) : ioInterupt(ioIntPin, PullUp) {
      io = io_ptr;
      interuptDetected = false;
      ioInterupt.fall(callback(this, &Degrees::handleInterupt));
    };

    void init() {

      io->init();
      io->setDirection(MCP23017_PORTA, 0xFF);           // set PORTA pins as inputs
      io->setDirection(MCP23017_PORTB, 0xFF);           // set PORTB pins as inputs
      io->setPullUp(MCP23017_PORTA, 0xFF);              // activate PORTA pin pull-ups for toggle switches
      io->setPullUp(MCP23017_PORTB, 0xFF);              // activate PORTB pin pull-ups for toggle switches
      io->setInputPolarity(MCP23017_PORTA, 0x00);       // invert PORTA pins input polarity for toggle switches
      io->setInputPolarity(MCP23017_PORTB, 0x00);       // invert PORTB pins input polarity for toggle switches
      io->setInterupt(MCP23017_PORTA, 0xFF);
      io->setInterupt(MCP23017_PORTB, 0xFF);
      
      updateDegreeStates(); // get current state of toggle switches
    };

    void handleInterupt() {
      interuptDetected = true;
    };

    void poll() {
      if (interuptDetected) {     // update switch states
        wait_us(5);
        updateDegreeStates();
        interuptDetected = false;
      }
    };

    void updateDegreeStates() {
      currState = io->digitalReadAB();
      if (currState != prevState) {
        // for notifiying external channels there was a change
        hasChanged[0] = true;
        hasChanged[1] = true;
        hasChanged[2] = true;
        hasChanged[3] = true;

        int switchIndex = 0;
        for (int i = 0; i < 16; i++) {   // iterate over all 16 bits
          if (i % 2 == 0) {             // only checking bits in pairs
            int bitA = io->getBitStatus(currState, i);
            int bitB = io->getBitStatus(currState, i + 1);
            int value = (bitB | bitA) >> i;
            switch (value) {
              case SWITCH_UP:
                switchStates[switchIndex] = 2;
                break;
              case SWITCH_NEUTRAL:
                switchStates[switchIndex] = 1;
                break;
              case SWITCH_DOWN:
                switchStates[switchIndex] = 0;
                break;
            }
            switchIndex += 1;
          }
        }
        prevState = currState;
      }
    };

  private:
    enum SwitchPosition {
      SWITCH_UP = 2,      // 0b00000010
      SWITCH_NEUTRAL = 3, // 0b00000011
      SWITCH_DOWN = 1,    // 0b00000001
    };

};



#endif