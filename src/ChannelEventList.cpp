#include "ChannelEventList.h"

DigitalOut channelGate(PA_10);

void ChannelEventList::createEvent(int position) {
  temp = new EventNode;
  temp->startPos = position;
  temp->triggered = false;
  temp->next = NULL;
}

void ChannelEventList::addEvent(int position) {
  temp->endPos = position;
  if (head == NULL) {
    head = temp;
  } else {
    head->next = temp;
  }
  temp = NULL;
}


bool ChannelEventList::hasEventInQueue() {
  if (queued) {
    return true;
  } else {
    return false;
  }
}

void ChannelEventList::handleQueuedEvent(int position) {
  if (queued->triggered == false ) {
    if (position == queued->startPos) {
      channelGate.write(HIGH);
      // send midi note
      queued->triggered = true;
    }
  }
  else if (position == queued->endPos) {
    channelGate.write(LOW);
    queued->triggered = false;
    if (queued->next != NULL) {
      queued = queued->next;
    } else {
      queued = head;
    }
  }
}



// ensuing, succeeding