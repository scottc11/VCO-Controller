#ifndef __EVENT_LIST_H
#define __EVENT_LIST_H

#include "main.h"

typedef struct EventNode {
  uint32_t duration;       // the duration of the EventNode in microseconds
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  uint8_t step;           // which step the EventNode occurs within clocks loop
  bool triggered;          // has the EventNode been triggered
  struct EventNode *next;      // pointer to the 'next' EventNode to occur (linked list)
} EventNode;


class ChannelEventList {
  private:
    EventNode* head;
    EventNode* temp;
    EventNode* queued;

  public:
    ChannelEventList() {
      head=NULL;
      temp=NULL;
      queued=NULL;
    }

    void createEvent(int position);
    void addEvent(int position);
    bool hasEventInQueue();
    void handleQueuedEvent(int position);
};

#endif