
#include <iterator>
#include <list>

#define PPQN  24
#define EVENT_END_BUFFER 4

using namespace std;

class EventNode {
public:
  EventNode() {};
  ~EventNode() {};

  uint16_t index;             // note noteIndex between 0 and 7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  bool exists;               // this is purely for the std::iterator use for determining if an object exists with next(), prev(), etc.
};


class EventList {
public:
  EventNode newEvent;                     // instead of creating a new object everytime a new event gets created, just modify this
  list<EventNode> eventList;              // std::list for holding event nodes
  list<EventNode>::iterator queuedEvent;
  volatile int numLoopSteps;
  volatile int currStep;                  // the current 'step' of the loop (lowest value == 1)
  volatile int currPosition;              // the current position in the loop measured by PPQN (lowest value == 1)
  volatile int currTick;                  // the current PPQN position of the step (0..PPQN) (lowest value == 1)
  volatile int loopLength;                // how many PPQN (in total) the loop contains

  void createEvent(int position, int noteIndex);
  void addEventToList(int endPosition);
  void handleQueuedEvent(int position);
};




void EventList::handleQueuedEvent(int position) {
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
      if (next(queuedEvent)->exists) {
        queuedEvent = next(queuedEvent);
      } else {
        queuedEvent = eventList.begin();
      }
    }
  }
}



void EventList::createEvent(int position, int noteIndex) {
  newEvent.index = noteIndex;
  newEvent.startPos = position;
  newEvent.triggered = false;
  newEvent.exists = true;
}



void EventList::addEventToList(int endPosition) {
  
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
            eventList.insert(next(it), newEvent);
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
            eventList.insert(next(it), newEvent); // add new event to end of list
            break;
          }
        }
      }
    }
  }
}