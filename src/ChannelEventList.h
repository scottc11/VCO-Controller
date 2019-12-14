#ifndef __EVENT_LIST_H
#define __EVENT_LIST_H

#include "main.h"

typedef struct EventNode {
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
    ChannelEventList(PinName gateOutPin) : gateOut(gateOutPin) {
      head=NULL;
      newEvent=NULL;
      queued=NULL;
    }

    DigitalOut gateOut;

    void createEvent(int position);
    void addEvent(int position);
    bool hasEventInQueue();
    void handleQueuedEvent(int position);
};

#endif