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
        queuedEvent++;
      } else {
        queuedEvent = eventList.begin();
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
  newEvent.exists = true;
}

void TouchChannel::addEventToList(int endPosition) {
  
  if (endPosition == newEvent.startPos) {
    newEvent.endPos = endPosition + EVENT_END_BUFFER;
  } else {
    newEvent.endPos = endPosition;
  }

  if (eventList.empty()) { // initialize the list
    eventList.push_back(newEvent);
    queuedEvent = eventList.begin();
  }

  // algorithm
  else {
    // iterate over all existing events in list, placing the new event in the list relative to those events
    for (list<EventNode>::iterator it = eventList.begin(); it != eventList.end(); it++) {
      
      // OCCURS BEFORE ITERATOR->START
      if (newEvent.startPos < it->startPos) {
        // new event ends before the current iterations begins
        if (newEvent.endPos < it->startPos) {
          // If new event occurs before the first event in list, place at front of list
          if (it == eventList.begin()) {
            eventList.push_front(newEvent);
            break;
          }
          // place inbetween the previous iteration and current iteration
          else {
            eventList.insert(it, newEvent);
            break;
          }
        }
        // does newEvent end after the current iteration starts?
        else if (newEvent.endPos >= it->startPos) {
          
          if (next(it)->exists) { // if there is another event after this one
            queuedEvent = next(it);  // update the queued event, as it will be the next one to be triggered after addEventToList() finishes executing
            eventList.erase(it); // delete iteration and continue
            continue;
          }
          // delete this iteration and replace with new event
          else {
            queuedEvent = eventList.begin(); // set queued event to first in list
            eventList.insert(it, newEvent);
            eventList.erase(it);
            break;
          }
        }
      }

      // OCCURS AFTER ITERATOR->START
      else {
        // if the newEvent starts after the end of current iteration
        if (newEvent.startPos > it->endPos) {
          
          // if there is an event after the current iterator, continue through loop, else add new event to end of list
          if (next(it)->exists) {
            continue;
          } else {
            eventList.push_back(newEvent);
            break;
          }
        }

        // if newEvent starts before iteration ends, update end position of current iteration and continue
        else if (newEvent.startPos <= it->endPos) {
          it->endPos = newEvent.startPos - 1;
          if (next(it)->exists) {
            continue;
          } else {
            queuedEvent = eventList.begin(); // set queued event to first in list
            eventList.push_back(newEvent);   // add new event to end of list
            break;
          }
          continue;
        }
      }
    }
  }
}