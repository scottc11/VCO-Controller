#ifndef __MIDI_H
#define __MIDI_H

#include "main.h"

const int MIDI_NOTE_MAP[8] = { 60, 62, 64, 65, 67, 69, 71, 72 };

class MIDI {
  private:
    RawSerial serial;

  public:
    MIDI() : serial(MIDI_TX, MIDI_RX, MIDI_BAUD) {}
    
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
  
};


#endif