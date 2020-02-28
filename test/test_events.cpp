#include <unity.h>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;

#define PPQN  24
#define EVENT_END_BUFFER 4

class EventNode {
public:
  EventNode() {
    // init..
  };
  ~EventNode() {};

  uint16_t index;             // note noteIndex between 0 and 7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
};

EventNode newEvent;

void setUp(void) {
  // set stuff up here
  
}

void tearDown(void) {
// clean stuff up here
}

void printList(list<EventNode> *_list) {
 for(list<EventNode>::iterator it = _list->begin(); it != _list->end(); it++) {
    cout << "index: " << (it)->index << " startPos: " << (it)->startPos << " endPos: " << (it)->endPos << endl;
  }
}

void createEvent(int position, int noteIndex) {
  newEvent.index = noteIndex;
  newEvent.startPos = position;
  newEvent.triggered = false;
}

void addEventToList(list<EventNode> *_list, int endPosition) {
  
  if (endPosition == newEvent.startPos) {
    newEvent.endPos = endPosition + EVENT_END_BUFFER;
  } else {
    newEvent.endPos = endPosition;
  }

  if (_list->empty()) { // initialize the list
    cout << "list was empty" << endl;
    _list->push_back(newEvent);
  }

  // algorithm
  else {
    // iterate over all existing events in list, placing the new event in the list relative to those events
    for (list<EventNode>::iterator it = _list->begin(); it != _list->end(); it++) {
      
      // OCCURS BEFORE ITERATOR->START
      if (newEvent.startPos < it->startPos) {
        // new event ends before the current iterations begins
        if (newEvent.endPos < it->startPos) {
          // If new event occurs before the first event in list, place at front of list
          if (it == _list->begin()) {
            _list->push_front(newEvent);
            break;
          }
          // place inbetween the previous iteration and current iteration
          else {
            _list->insert(it, newEvent);
            break;
          }
        }
        // newEvent overlaps the current iteration start? delete this iteration and replace with new event
        else if (newEvent.endPos >= it->startPos) {
          _list->insert(it, newEvent);
          _list->erase(it);
          break;
        }
      }

      else {
        if (newEvent.startPos > it->endPos) {
          if (next(it)->startPos) {
            continue;
          } else {
            _list->insert(next(it), newEvent);
            break;
          }
        }
      }
    }
  }
}




void test_create_events(void) {
  list<EventNode> mylist;
  createEvent(12, 1);
  addEventToList(&mylist, 15);
  
  createEvent(18, 2);
  addEventToList(&mylist, 30);

  printList(&mylist);

  TEST_ASSERT_EQUAL(2, mylist.size());
}

void test_insert_event_between_two_events(void) {
  list<EventNode> mylist;

  createEvent(12, 1);
  addEventToList(&mylist, 15);
  
  createEvent(30, 2);
  addEventToList(&mylist, 40);

  createEvent(18, 1);
  addEventToList(&mylist, 25);
  
  createEvent(50, 1);
  addEventToList(&mylist, 60);

  createEvent(42, 1);
  addEventToList(&mylist, 48);

  printList(&mylist);

  list<EventNode>::iterator it = mylist.begin();
  advance(it, 1);
  TEST_ASSERT_EQUAL_INT(18, it->startPos);
  TEST_ASSERT_EQUAL(5, mylist.size());
}

void test_insert_overlapping_event(void) {
  list<EventNode> mylist;

  TEST_ASSERT_EQUAL(2, mylist.size());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_create_events);
    RUN_TEST(test_insert_event_between_two_events);
    // RUN_TEST(test_insert_overlapping_event);
    UNITY_END();
    return 0;
}