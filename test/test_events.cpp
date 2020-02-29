// #include <unity.h>
// #include <iostream>
// #include <iterator>
// #include <list>

// #include "EventList.cpp"

// using namespace std;

// void setUp(void) {
//   // set stuff up here
// }

// void tearDown(void) {
//   // clean stuff up here
// }

// void printList(EventList * obj) {
//  for(list<EventNode>::iterator it = obj->eventList.begin(); it != obj->eventList.end(); it++) {
//     cout << "index: " << (it)->index << " startPos: " << (it)->startPos << " endPos: " << (it)->endPos << endl;
//   }
// }


// void test_create_events(void) {
//   EventList mylist;
//   mylist.createEvent(12, 1);
//   mylist.addEventToList(15);
  
//   mylist.createEvent(18, 2);
//   mylist.addEventToList(30);

//   printList(&mylist);

//   TEST_ASSERT_EQUAL(2, mylist.eventList.size());
// }

// void test_insert_event_between_two_events(void) {
//   EventList mylist;

//   mylist.createEvent(12, 1);
//   mylist.addEventToList(15);
  
//   mylist.createEvent(30, 2);
//   mylist.addEventToList(40);

//   mylist.createEvent(18, 1);
//   mylist.addEventToList(25);
  
//   mylist.createEvent(50, 1);
//   mylist.addEventToList(60);

//   mylist.createEvent(42, 1);
//   mylist.addEventToList(48);

//   printList(&mylist);

//   mylist.queuedEvent = mylist.eventList.begin();
//   advance(mylist.queuedEvent, 1);
//   TEST_ASSERT_EQUAL_INT(18, mylist.queuedEvent->startPos);
//   mylist.queuedEvent = mylist.eventList.begin();
//   advance(mylist.queuedEvent, 3);
//   TEST_ASSERT_EQUAL_INT(42, mylist.queuedEvent->startPos);
//   TEST_ASSERT_EQUAL(5, mylist.eventList.size());
// }


// void test_insert_between_and_overlap(void) {
//   EventList mylist;

//   mylist.createEvent(12, 1);
//   mylist.addEventToList(15);

//   mylist.createEvent(18, 1);
//   mylist.addEventToList(25);

//   mylist.createEvent(42, 1);
//   mylist.addEventToList(48);

//   mylist.createEvent(14, 1);
//   mylist.addEventToList(30);

//   printList(&mylist);
//   TEST_ASSERT_EQUAL(3, mylist.eventList.size());
// }

// // new event start position overlaps the current iterations end position
// void test_insert_overlapping_end_event(void) {
//   EventList mylist;

//   mylist.createEvent(10, 1);
//   mylist.addEventToList(20);

//   mylist.createEvent(15, 1);
//   mylist.addEventToList(25);

//   mylist.createEvent(40, 1);
//   mylist.addEventToList(44);

//   printList(&mylist);

//   mylist.queuedEvent = mylist.eventList.begin();
//   TEST_ASSERT_EQUAL_INT(14, mylist.queuedEvent->endPos);

//   TEST_ASSERT_EQUAL(3, mylist.eventList.size());
// }

// // new event start position overlaps the current iterations end position, AND new event end position overlaps the next iterations start position
// void test_insert_double_overlapping_event(void) {
//   EventList mylist;

//   mylist.createEvent(10, 1);
//   mylist.addEventToList(20);

//   mylist.createEvent(40, 1);
//   mylist.addEventToList(60);

//   mylist.createEvent(15, 1);
//   mylist.addEventToList(45);

//   printList(&mylist);

//   mylist.queuedEvent = mylist.eventList.begin();
//   TEST_ASSERT_EQUAL_INT(14, mylist.queuedEvent->endPos);

//   TEST_ASSERT_EQUAL(2, mylist.eventList.size());
// }

// void test_insert_at_front(void) {
//   EventList mylist;

//   mylist.createEvent(15, 1);
//   mylist.addEventToList(25);

//   mylist.createEvent(40, 1);
//   mylist.addEventToList(60);

//   mylist.createEvent(10, 1);
//   mylist.addEventToList(12);

//   printList(&mylist);

//   mylist.queuedEvent = mylist.eventList.begin();
//   TEST_ASSERT_EQUAL_INT(10, mylist.queuedEvent->startPos);

//   TEST_ASSERT_EQUAL(3, mylist.eventList.size());
// }

// void test_insert_at_front_and_overlap(void) {
//   EventList mylist;

//   mylist.createEvent(40, 1);
//   mylist.addEventToList(60);

//   mylist.createEvent(15, 2);
//   mylist.addEventToList(25);

//   mylist.createEvent(30, 3);
//   mylist.addEventToList(35);

//   mylist.createEvent(36, 3);
//   mylist.addEventToList(39);

//   mylist.createEvent(10, 4);
//   mylist.addEventToList(37);

//   printList(&mylist);

//   mylist.queuedEvent = mylist.eventList.begin();
//   TEST_ASSERT_EQUAL_INT(10, mylist.queuedEvent->startPos);

//   TEST_ASSERT_EQUAL(2, mylist.eventList.size());
// }

// void test_handle_queued_event(void) {
//   EventList mylist;
  
//   mylist.createEvent(12, 1);
//   mylist.addEventToList(15);
  
//   // queuedEvent should be the first event in list
//   TEST_ASSERT_TRUE(mylist.queuedEvent == mylist.eventList.begin());

//   // next should return false
//   TEST_ASSERT_FALSE(next(mylist.queuedEvent)->exists);

//   mylist.handleQueuedEvent(12);
//   TEST_ASSERT_TRUE(mylist.queuedEvent->triggered);

//   mylist.handleQueuedEvent(15);
//   TEST_ASSERT_EQUAL(1, mylist.eventList.size());

//   printList(&mylist);
// }

// int main(int argc, char **argv) {
//     UNITY_BEGIN();
//     RUN_TEST(test_create_events);
//     RUN_TEST(test_insert_event_between_two_events);
//     RUN_TEST(test_insert_between_and_overlap);
//     RUN_TEST(test_insert_overlapping_end_event);
//     RUN_TEST(test_insert_double_overlapping_event);
//     RUN_TEST(test_insert_at_front);
//     RUN_TEST(test_insert_at_front_and_overlap);
    
//     RUN_TEST(test_handle_queued_event);

//     UNITY_END();
//     return 0;
// }