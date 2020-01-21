#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "main.h"
#include "CAP1208.h"
#include "ChannelEventList.h"
#include "BeatClock.h"

class Channel {
  public:

  Channel(PinName tchInt, PinName _gate, I2C *touchI2C, ShiftRegister *_reg, MIDI *_midi, BeatClock *_clock) : touchInterupt(tchInt), events(_gate, _reg, _midi) {
    touch.init(touchI2C);
    clock = _clock;
  }

  CAP1208 touch;
  ChannelEventList events;
  BeatClock *clock;
  uint8_t touched;
  uint8_t prevTouched;
  bool ETL;

  InterruptIn touchInterupt;
  int touchEvent;

  void handleInterupts() {
    if (touchEvent == 1) {
      touched = touch.touched();
      if (touched != prevTouched) {
        for (int i=0; i<8; i++) {
          // if it *is* touched and *wasnt* touched before, alert!
          if (touch.getBitStatus(touched, i) && !touch.getBitStatus(prevTouched, i)) {
            ETL = false; // deactivate event triggering loop
            events.createEvent(clock->position, i);
          }
          // if it *was* touched and now *isnt*, alert!
          if (!touch.getBitStatus(touched, i) && touch.getBitStatus(prevTouched, i)) {
            events.addEvent(clock->position);
            ETL = true; // activate event triggering loop
          }
        }
        reg.writeByte(touched); // toggle channel LEDs
        prevTouched = touched;
      }

      touchEvent = 0;
    }
  }

};


#endif