#ifndef __EVENT_LIST_H
#define __EVENT_LIST_H

#include "main.h"
#include "ShiftRegister.h"
#include "MCP23017.h"
#include "MIDI.h"

typedef struct EventNode {
  uint8_t index;             // note index :: one of 0..7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;

// Linked List
class ChannelEventList {
  private:
    EventNode* head;
    EventNode* newEvent;  // to be created and deleted everytime an user presses event create button
    EventNode* queued;    // the currently active / next / ensuing / succeeding event

  public:
    ChannelEventList(PinName gateOutPin, PinName intPin, MCP23017 *io_p, MIDI *midi_p) : gateOut(gateOutPin), switchInterupt(intPin, PullUp) {
      head=NULL;
      newEvent=NULL;
      queued=NULL;
      io = io_p;
      midi = midi_p;
      switchInterupt.fall(callback(this, &ChannelEventList::handleSwitchInterupt));
      counter = 0;
      octave = 0;
    }

    DigitalOut gateOut;
    InterruptIn switchInterupt;
    volatile bool switchHasChanged;
    int octave;
    int counter;
    MCP23017 *io;
    MIDI *midi;          // pointer to mbed midi instance
    int leds[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };

    void init();
    void handleSwitchInterupt() { switchHasChanged = true; }
    void poll();
    void setLed(int led_index);
    void updateLeds(uint8_t touched);
    void setOctaveLed(int led_index);
    void createEvent(int position, int noteIndex);
    void addEvent(int position);
    bool hasEventInQueue();
    void handleQueuedEvent(int position);
};

#endif