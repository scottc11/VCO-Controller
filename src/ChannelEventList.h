#ifndef __EVENT_LIST_H
#define __EVENT_LIST_H

#include "main.h"
#include "ShiftRegister.h"

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
    ChannelEventList(PinName gateOutPin, ShiftRegister *reg_p) : gateOut(gateOutPin) {
      head=NULL;
      newEvent=NULL;
      queued=NULL;
      reg = reg_p;
    }

    DigitalOut gateOut;
    ShiftRegister *reg;

    void createEvent(int position, int noteIndex);
    void addEvent(int position);
    bool hasEventInQueue();
    void handleQueuedEvent(int position);
};

#endif