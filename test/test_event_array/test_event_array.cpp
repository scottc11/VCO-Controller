#include <unity.h>
#include <iostream>
#include "EventLinkedList.h"

using namespace std;

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void print(int value) {
  cout << "value: " << value << endl;
}

int numSteps = 8;
int ppqn = 24;
int step;

void testy() {
  EventNode events[192];
  step = 0;

  if (events[step].triggered == false) {
    events[step].triggered = true;
    // triggerEvent(events[step].noteIndex) 
  } else {
    // nothing?
  }

}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(testy);
    UNITY_END();
    return 0;
}