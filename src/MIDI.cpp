#include "MIDI.h"


void MIDI::sendNoteOn(uint8_t _channel, uint8_t _note, uint8_t _velocity) {
  // you need to send three bytes for a note on message.
  
  uint8_t status = 0x90 | _channel; // byte 1 -> midi function
  uint8_t note = _note;             // byte 2 -> note (0 - 127)
  uint8_t velocity = _velocity;     // byte 3 -> velocity (default to 64)

  serial.putc(status);
  serial.putc(note);
  serial.putc(velocity);
}

void MIDI::sendNoteOff(uint8_t _channel, uint8_t _note, uint8_t _velocity) {
  uint8_t status = 0x80 | _channel;
  uint8_t note = _note;
  uint8_t velocity = _velocity;

  serial.putc(status);
  serial.putc(note);
  serial.putc(velocity);
}