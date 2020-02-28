#include "TouchChannel.h"

// ========================================================================================================================
// LOOPER MODE FUNCTIONS
// ========================================================================================================================

void TouchChannel::handleQueuedEvent(int position) {
  if (queuedEvent->triggered == false ) {
    if (position == queuedEvent->startPos) {
      triggerNote(queuedEvent->index, currOctave, ON);
      queuedEvent->triggered = true;
    }
  }
  else {
    if (position == queuedEvent->endPos) {
      triggerNote(queuedEvent->index, currOctave, OFF);
      queuedEvent->triggered = false;
      if (next(queuedEvent)->exists) {
        queuedEvent = next(queuedEvent);
      } else {
        queuedEvent = events.begin();
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
  newEvent.index = noteIndex;
  newEvent.startPos = position;
  newEvent.triggered = false;
}

void TouchChannel::addEventToList(list<EventNode> *eventList, int endPosition) {
  
  if (endPosition == newEvent.startPos) {
    newEvent.endPos = endPosition + EVENT_END_BUFFER;
  } else {
    newEvent.endPos = endPosition;
  }

  if (eventList->empty()) { // initialize the list
    eventList->push_back(newEvent);
  }

  // algorithm
  else {
    // iterate over all existing events in list, placing the new event in the list relative to those events
    for (list<EventNode>::iterator it = eventList->begin(); it != eventList->end(); it++) {
      
      // OCCURS BEFORE ITERATOR->START
      if (newEvent.startPos < it->startPos) {
        // new event ends before the current iterations begins
        if (newEvent.endPos < it->startPos) {
          // If new event occurs before the first event in list, place at front of list
          if (it == eventList->begin()) {
            eventList->push_front(newEvent);
            break;
          }
          // place inbetween the previous iteration and current iteration
          else {
            eventList->insert(it, newEvent);
            break;
          }
        }
        // newEvent ends before the current iteration start? delete this iteration and replace with new event
        else if (newEvent.endPos >= it->startPos) {
          
          if (next(it)->startPos) { // if there is another event after this one
            eventList->erase(it);
            continue;
          } else {
            eventList->insert(it, newEvent);
            eventList->erase(it);
            break;
          }
        }
      }

      // OCCURS AFTER ITERATOR->START
      else {
        // if the newEvent starts after the end of current iteration
        if (newEvent.startPos > it->endPos) {
          
          // if there is an event after the current iterator, continue through loop, else add new event to end of list
          if (next(it)->startPos) {
            continue;
          } else {
            eventList->insert(next(it), newEvent);
            break;
          }
        }

        // if newEvent starts before iteration ends, update end position of current iteration and continue
        else if (newEvent.startPos <= it->endPos) {
          it->endPos = newEvent.startPos - 1;
          if (next(it)->startPos) {
            continue;
          } else {
            eventList->insert(next(it), newEvent);
            break;
          }
          continue;
        }
      }
    }
  }
}