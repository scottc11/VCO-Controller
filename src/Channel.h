#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "main.h"
#include "CAP1208.h"
#include "ChannelEventList.h"
#include "BeatClock.h"
#include "TCA9544A.h"

class Channel {
  public:

  Channel(int _channel, PinName tchInt, PinName _gate, I2C *touchI2C, ShiftRegister *_reg, MIDI *_midi, BeatClock *_clock, TCA9544A *mux) : touchInterupt(tchInt), events(_gate, _reg, _midi) {
    channel = _channel;
    touch.init(touchI2C, mux, channel);
    clock = _clock;
  }
  
  int channel;  // index for identifying channel number (ie, channel 0, channel 1, etc.)
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
        // reg.writeByte(touched); // toggle channel LEDs
        prevTouched = touched;
      }

      touchEvent = 0;
    }
  }

};


#endif