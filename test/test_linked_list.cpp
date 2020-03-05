#include <unity.h>
#include <iostream>
#include "EventLinkedList.cpp"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void printList(EventLinkedList * list) {
  EventNode *iterator;
  iterator = list->head;
  while (iterator) {
    cout << "index: " << iterator->index << " startPos: " << iterator->startPos << " endPos: " << iterator->endPos << endl;
    iterator = iterator->next;
  }
}


void test_create_events(void) {
  EventLinkedList list;
  
  list.createEvent(12, 1);
  list.addEventToList(15);

  list.createEvent(18, 2);
  list.addEventToList(30);

  list.createEvent(32, 3);
  list.addEventToList(36);

  list.createEvent(40, 4);
  list.addEventToList(46);

  printList(&list);

  TEST_ASSERT_EQUAL(4, list.length());
}

void test_insert_event_between_two_events(void) {
  EventLinkedList list;

  list.createEvent(12, 1);
  list.addEventToList(15);
  
  list.createEvent(30, 2);
  list.addEventToList(40);

  list.createEvent(18, 1);
  list.addEventToList(25);
  
  list.createEvent(50, 1);
  list.addEventToList(60);

  list.createEvent(42, 1);
  list.addEventToList(48);

  printList(&list);

  TEST_ASSERT_EQUAL(5, list.length());
}


void test_insert_between_and_overlap(void) {
  EventLinkedList list;

  list.createEvent(12, 1);
  list.addEventToList(15);

  list.createEvent(18, 1);
  list.addEventToList(25);

  list.createEvent(42, 1);
  list.addEventToList(48);

  list.createEvent(14, 1);
  list.addEventToList(30);

  printList(&list);
  TEST_ASSERT_EQUAL(3, list.length());
}

// new event start position overlaps the current iterations end position
void test_insert_overlapping_end_event(void) {
  EventLinkedList list;

  list.createEvent(10, 1);
  list.addEventToList(20);

  list.createEvent(15, 1);
  list.addEventToList(25);

  list.createEvent(40, 1);
  list.addEventToList(44);

  printList(&list);

  TEST_ASSERT_EQUAL_INT(14, list.head->endPos);

  TEST_ASSERT_EQUAL(3, list.length());
}

// new event start position overlaps the current iterations end position, AND new event end position overlaps the next iterations start position
void test_insert_double_overlapping_event(void) {
  EventLinkedList list;

  list.createEvent(10, 1);
  list.addEventToList(20);

  list.createEvent(40, 1);
  list.addEventToList(60);

  list.createEvent(15, 1);
  list.addEventToList(45);

  printList(&list);

  TEST_ASSERT_EQUAL_INT(14, list.head->endPos);

  TEST_ASSERT_EQUAL(2, list.length());
}

void test_insert_at_front(void) {
  EventLinkedList list;

  list.createEvent(15, 1);
  list.addEventToList(25);

  list.createEvent(40, 1);
  list.addEventToList(60);

  list.createEvent(10, 1);
  list.addEventToList(12);

  printList(&list);

  TEST_ASSERT_EQUAL_INT(10, list.head->startPos);

  TEST_ASSERT_EQUAL(3, list.length());
}

void test_insert_at_front_and_overlap(void) {
  EventLinkedList list;

  list.createEvent(40, 1);
  list.addEventToList(60);

  list.createEvent(15, 2);
  list.addEventToList(25);

  list.createEvent(30, 3);
  list.addEventToList(35);

  list.createEvent(36, 3);
  list.addEventToList(39);

  list.createEvent(10, 4);
  list.addEventToList(37);

  printList(&list);

  TEST_ASSERT_EQUAL_INT(10, list.head->startPos);
  TEST_ASSERT_EQUAL_INT(37, list.head->endPos);

  TEST_ASSERT_EQUAL(2, list.length());
}

void test_handle_queued_event(void) {
  EventLinkedList list;
  
  list.createEvent(12, 1);
  list.addEventToList(15);
  
  // queuedEvent should be the first event in list
  TEST_ASSERT_TRUE(list.queuedEvent == list.head);

  // next should return false
  TEST_ASSERT_FALSE(list.queuedEvent->next);

  list.createEvent(24, 1);
  list.addEventToList(30);

  TEST_ASSERT_TRUE(list.queuedEvent->next);
  TEST_ASSERT_TRUE(list.queuedEvent == list.head);    // queued event should still be head

  list.handleQueuedEvent(12);
  TEST_ASSERT_TRUE(list.queuedEvent->triggered);
  TEST_ASSERT_EQUAL(12, list.queuedEvent->startPos);

  list.handleQueuedEvent(15);
  TEST_ASSERT_EQUAL(24, list.queuedEvent->startPos);

  printList(&list);
}

void test_delete_all_events() {
  EventLinkedList list;
  
  list.createEvent(12, 1);
  list.addEventToList(15);
  
  list.createEvent(30, 2);
  list.addEventToList(40);

  list.createEvent(18, 1);
  list.addEventToList(25);
  
  list.createEvent(50, 1);
  list.addEventToList(60);

  list.createEvent(42, 1);
  list.addEventToList(48);

  printList(&list);

  TEST_ASSERT_EQUAL(5, list.length());

  list.clearEventList();

  TEST_ASSERT_EQUAL(0, list.length());
}

void test_loop_end_overlap() {
  EventLinkedList list;
  list.numLoopSteps = 8;
  list.createEvent(189, 1);
  list.addEventToList(189);
  list.createEvent(188, 1);
  list.addEventToList(20);

  printList(&list);
  TEST_ASSERT_EQUAL((list.numLoopSteps * PPQN) - 1, list.head->endPos);
}

void test_quantization_8th_notes() {
  EventLinkedList list;
  list.numLoopSteps = 8;
  list.timeQuantizationMode = EventLinkedList::QUANT_8;
  
  // test event quantizes beginning of step
  list.currStep = 1;
  list.createEvent(26, 1);
  list.addEventToList(88);
  TEST_ASSERT_EQUAL(24, list.head->startPos);

  // test event quantizes over to the next start of the step
  list.currStep = 3;
  list.createEvent(78, 1);
  list.addEventToList(88);
  TEST_ASSERT_EQUAL(84, list.head->next->startPos);

  // test event quantizes over to the next start of the step
  list.currStep = 5;
  list.createEvent(142, 1);
  list.addEventToList(88);
  TEST_ASSERT_EQUAL(144, list.head->next->next->startPos);

  list.currStep = 8;
  list.createEvent(191, 1);
  list.addEventToList(88);

  printList(&list);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_create_events);
    RUN_TEST(test_insert_event_between_two_events);
    RUN_TEST(test_insert_between_and_overlap);
    RUN_TEST(test_insert_overlapping_end_event);
    RUN_TEST(test_insert_double_overlapping_event);
    RUN_TEST(test_insert_at_front);
    RUN_TEST(test_insert_at_front_and_overlap);
    
    RUN_TEST(test_handle_queued_event);
    RUN_TEST(test_delete_all_events);
    RUN_TEST(test_loop_end_overlap);
    RUN_TEST(test_quantization_8th_notes);
    UNITY_END();
    return 0;
}