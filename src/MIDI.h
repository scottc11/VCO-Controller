#ifndef __MIDI_H
#define __MIDI_H

#include "main.h"

class MIDI {
  private:
    RawSerial serial;

  public:
    MIDI() : serial(MIDI_TX, MIDI_RX, MIDI_BAUD) {}
    
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
  
};


#endif