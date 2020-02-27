#include "TouchChannel.h"

// ========================================================================================================================
// LOOPER MODE FUNCTIONS
// ========================================================================================================================

bool TouchChannel::hasEventInQueue() {
  return queued ? true : false;
}

void TouchChannel::handleQueuedEvent(int position) {
  if (queued->triggered == false ) {
    if (position == queued->startPos) {
      triggerNote(queued->index, currOctave, ON);
      queued->triggered = true;
    }
  }
  else {
    if (position == queued->endPos) {
      triggerNote(queued->index, currOctave, OFF);
      queued->triggered = false;
      if (queued->next != NULL) {
        queued = queued->next;
      } else {
        queued = head;
      }
    }
  }
}

int TouchChannel::quantizePosition(int position) {
  
  int pos = position < PPQN ? 0 : PPQN * currStep;

  if (currTick < 6) { // set position to first tick in step
    return pos;
  }
  if (currTick >= 6 && currTick < 18 ) {
    return pos + 12;
  }
  if (currTick >= 18) {
    return pos + 24;
  }
}

void TouchChannel::createEvent(int position, int noteIndex) {
  newEvent = new EventNode;
  newEvent->index = noteIndex;
  int startPosition = quantizePosition(position);
  newEvent->startPos = position;
  newEvent->triggered = false;
  newEvent->next = NULL;
}

void TouchChannel::addEvent(int position) {
  
  if (position == newEvent->startPos) {
    newEvent->endPos = position + EVENT_END_BUFFER;
  } else {
    newEvent->endPos = position;
  }

  if (head == NULL) { // initialize the list
    head = newEvent;
    queued = head;
  }
  // algorithm
  else {
    // iterate over all existing events in list, placing the new event in the list relative to those events
    EventNode *iteration = head;
    EventNode *prev = head;

    while(iteration != NULL) {
      
      // OCCURS BEFORE ITERATION->START
      if (newEvent->startPos < iteration->startPos) {
        
        // does the new event end before the iteration starts?
        if (newEvent->endPos <= iteration->startPos) {
          
          // If iteration is head, reset head to new event
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
        
        // newEvent overlaps the current iteration start? delete this iteration and set iteration to iteration->next
        else {
          if (iteration == head) { // if iteration is head, set head to the newEvent
            head = newEvent;
          } else {
            prev->next = newEvent; // right here
          }
          
          EventNode *temp = iteration;
          iteration = iteration->next;    // repoint to next iteration in loop
          delete temp;                    // delete current iteration
          queued = iteration ? iteration : head;
          continue;                       // continue the while loop for further placement of newEvent
        }
      }
      
      // OCCURS AFTER ITERATION->START
      else {
        
        // if the newEvent begins after the end of current iteration, move on to next iteration
        if (newEvent->startPos > iteration->endPos) {
          if (iteration->next) {
            prev = iteration;
            iteration = iteration->next;
            continue;
          }
          iteration->next = newEvent;
          break;
        }
        
        // if newEvent starts before iteration ends, update end position of current iteration and place after
        else {
          iteration->endPos = newEvent->startPos - 2;
          prev = iteration;
          iteration = iteration->next;
          continue;
        }
      }
    } // END OF WHILE LOOP
  }
  newEvent = NULL;

}