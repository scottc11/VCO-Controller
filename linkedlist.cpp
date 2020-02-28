// compile -->  g++ -o linkedlist linkedlist.cpp
// run     -->  ./linkedlist

#include <iostream>
#include <list>

typedef struct EventNode {
  uint16_t noteIndex;             // note noteIndex between 0 and 7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;

int main()
{
  
  std::list<EventNode> mylist;

  EventNode newEvent;
  newEvent.noteIndex = 1;
  newEvent.startPos = 12;
  newEvent.triggered = false;
  mylist.push_back(newEvent);

  newEvent.noteIndex = 2;
  newEvent.startPos = 18;
  newEvent.triggered = false;
  mylist.push_back(newEvent);

  newEvent.noteIndex = 3;
  newEvent.startPos = 44;
  newEvent.triggered = false;
  mylist.push_back(newEvent);


  std::cout << "mylist contains:" << std::endl;
  
  for(std::list<EventNode>::iterator it = mylist.begin(); it != mylist.end(); it++) {
    std::cout << "item: " << (it)->noteIndex << " str: " << (it)->startPos << std::endl;
  }

  mylist.clear();

  return 0;
}
