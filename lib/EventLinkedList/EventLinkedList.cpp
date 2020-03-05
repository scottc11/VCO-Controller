#include <iostream>
#include "QuantizeMethods.cpp"

#define PPQN  24
#define EVENT_END_BUFFER 4

using namespace std;

typedef struct EventNode {
  uint8_t index;             // note index between 0 and 7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;


class EventLinkedList {
public:  
  enum QuantizeMode {
    NONE,
    QUANT_8,
    QUANT_16,
    QUANT_32,
    QUANT_64,
  };

  EventLinkedList() {
    head = NULL;
    newEvent = NULL;
    queuedEvent = NULL;
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
  int handleQuantization(int pos);
};


int EventLinkedList::length() {
  int32_t count = 0;
  EventNode *iteration = head;
  //loop until the end of the list is found
  while (iteration) {
    ++count;
    iteration = iteration->next;
  }
  return count;
}

void EventLinkedList::clearEventList() {
  EventNode *iteration;
  iteration = head;
  while (iteration) {
    EventNode *tmp = iteration;
    iteration = iteration->next;
    delete tmp;
  }
  head = NULL;
  queuedEvent = NULL;
}

void EventLinkedList::handleQueuedEvent(int position) {
  if (queuedEvent->triggered == false ) {
    if (position == queuedEvent->startPos) {
      // triggerNote(queuedEvent->index, currOctave, ON);
      queuedEvent->triggered = true;
    }
  }
  else {
    if (position == queuedEvent->endPos) {
      // triggerNote(queuedEvent->index, currOctave, OFF);
      queuedEvent->triggered = false;
      if (queuedEvent->next != NULL) {
        queuedEvent = queuedEvent->next;
      } else {
        queuedEvent = head;
      }
    }
  }
}

int EventLinkedList::handleQuantization(int pos) {
  switch (timeQuantizationMode) {
    case NONE:
      return pos;
    case QUANT_8:
      return quantize8th(pos, currStep, numLoopSteps, PPQN);
    case QUANT_16:
      return pos;
    case QUANT_32:
      return pos;
    case QUANT_64:
      return pos;
  }
}


void EventLinkedList::createEvent(int position, int noteIndex) {
  newEvent = new EventNode;
  newEvent->index = noteIndex;
  newEvent->startPos = handleQuantization(position);
  newEvent->triggered = false;
  newEvent->next = NULL;
}



void EventLinkedList::addEventToList(int endPosition) {
  
  if (endPosition < newEvent->startPos ) {
    newEvent->endPos = (numLoopSteps * PPQN) - 1;
  } else if (endPosition == newEvent->startPos) {
    newEvent->endPos = endPosition + EVENT_END_BUFFER > (numLoopSteps * PPQN) - 1 ? (numLoopSteps * PPQN) - 1 : endPosition + EVENT_END_BUFFER;
  } else {
    newEvent->endPos = endPosition;
  }

  if (head == NULL) { // initialize the list
    head = newEvent;
    queuedEvent = head;
  }

  // algorithm
  else {

    EventNode *iteration = head;
    EventNode *prev = head;

    // iterate over all existing events in list, placing the new event in the list relative to those events
    while (iteration != NULL) {
      
      // OCCURS BEFORE ITERATOR->START
      if (newEvent->startPos < iteration->startPos) {

        // new event ends before the current iterations begins
        if (newEvent->endPos < iteration->startPos) {

          // If new event occurs before the first event in list, place at front of list
          if (iteration == head) {
            newEvent->next = iteration;
            head = newEvent;
            break;
          }
          // place inbetween the previous iteration and current iteration
          else {
            prev->next = newEvent;
            newEvent->next = iteration;
            break;
          }
        }
        // does newEvent end after the current iteration starts?
        else if (newEvent->endPos >= iteration->startPos) {
          
          if (iteration == head) {
            if (head->next) {
              newEvent->next = head->next;
              iteration = newEvent->next;
              delete head;
              prev = newEvent;
              head = newEvent;
              continue;
            } else {
              newEvent->next = head->next;
              delete head;
              head = newEvent;
              break;
            } 
          }

          if (iteration->next) { // if there is another event after this one, delete and continue
            prev->next = newEvent;
            EventNode *temp = iteration;
            iteration = iteration->next; // repoint iteration to its next pointer,
            queuedEvent = iteration;     // update the queued event, as it will be the next one to be triggered after addEventToList() finishes executing
            delete temp;                 // delete prev iteration and continue
            continue;
          }
          // iteration is last event in list, delete iteration and replace with new event
          else {
            queuedEvent = head; // set queued event to first in list
            prev->next = newEvent; // previous needs to get assigned this event I think.
            delete iteration;
            break;
          }
        }
      }

      // OCCURS AFTER ITERATOR->START
      else {
        // if the newEvent starts after the end of current iteration
        if (newEvent->startPos > iteration->endPos) {
          
          // if there is an event after the current iterator, continue through loop
          if (iteration->next) {
            prev = iteration;
            iteration = iteration->next;
            continue;
          }
          // else add new event to end of list
          else {
            iteration->next = newEvent;
            break;
          }
        }

        // if newEvent starts before iteration ends, update end position of current iteration and continue
        else if (newEvent->startPos <= iteration->endPos) {
          iteration->endPos = newEvent->startPos - 1;
          if (iteration->next) {
            prev = iteration;
            iteration = iteration->next;
            continue;
          } else {
            queuedEvent = head; // set queued event to first in list
            iteration->next = newEvent; // add new event to end of list
            break;
          }
        }
      }
    }
  }
}