#ifndef __EVENT_LINKED_LIST_H
#define __EVENT_LINKED_LIST_H

#include <iostream>
#include "QuantizeMethods.h"


#define PPQN  24
#define EVENT_END_BUFFER 4

using namespace std;

typedef struct EventNode {
  int index;             // note index between 0 and 7
  int startPos;         // the point in time in which the EventNode occured
  int endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;


class EventLinkedList {
public:
  EventLinkedList() {
    head = NULL;
    newEvent = NULL;
    queuedEvent = NULL;
    timeQuantizationMode = QUANT_NONE;
  }

  EventNode* head;
  EventNode* newEvent;       // to be created and deleted everytime a user presses event create button
  EventNode* queuedEvent;    // the currently active / next / ensuing / succeeding event
  QuantizeMode timeQuantizationMode;
  volatile int numLoopSteps;
  volatile int currStep;                  // the current 'step' of the loop (lowest value == 1)
  volatile int currPosition;              // the current position in the loop measured by PPQN (lowest value == 1)
  volatile int currTick;                  // the current PPQN position of the step (0..PPQN) (lowest value == 1)
  volatile int loopLength;                // how many PPQN (in total) the loop contains

  void clearEventList();
  int length();
  void createEvent(int position, int noteIndex);
  void addEventToList(int endPosition);
  void handleQueuedEvent(int position);
};

#endif