#include "ChannelEventList.h"


void ChannelEventList::init() {
  io->init();
  io->setDirection(MCP23017_PORTA, 0x00);           // set all of the PORTA pins to output
  io->setDirection(MCP23017_PORTB, 0b00001111);     // set PORTB pins 0-3 as input, 4-7 as output
  io->setPullUp(MCP23017_PORTB, 0b00001111);        // activate PORTB pin pull-ups for toggle switches
  io->setInputPolarity(MCP23017_PORTB, 0b00001111); // invert PORTB pins input polarity for toggle switches
  io->setInterupt(MCP23017_PORTB, 0b00001111);

  for (int i = 0; i < 8; i++) {
    this->setLed(i);
    wait_ms(100);
  }

  this->setLed(0);
  this->setOctaveLed(octave);
}

// HANDLE ALL INTERUPT FLAGS
void ChannelEventList::poll() {
  if (switchHasChanged) {
    counter += 1;
    volatile int change = io->digitalRead(MCP23017_PORTB);
    if (counter % 2 == 0) {
      updateLeds(0x00);
    } else {
      updateLeds(0xFF);
    }
    switchHasChanged = false;
  }
}

void ChannelEventList::setLed(int led_index) {
  io->digitalWrite(MCP23017_PORTA, leds[led_index]);
}

void ChannelEventList::updateLeds(uint8_t touched) {
  io->digitalWrite(MCP23017_PORTA, touched);
}

void ChannelEventList::setOctaveLed(int led_index) {
  io->digitalWrite(MCP23017_PORTB, 0b11110000);
}

void ChannelEventList::createEvent(int position, int noteIndex) {
  newEvent = new EventNode;
  newEvent->index = noteIndex;
  newEvent->startPos = position;
  newEvent->triggered = false;
  newEvent->next = NULL;
}

void ChannelEventList::addEvent(int position) {
  newEvent->endPos = position;
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
      gateOut.write(HIGH);
      // reg->setBit(queued->index);
      midi->sendNoteOn(1, MIDI_NOTE_MAP[queued->index], 100);
      // send midi note
      queued->triggered = true;
    }
  }
  else if (position == queued->endPos) {
    gateOut.write(LOW);
    // reg->clearBit(queued->index);
    midi->sendNoteOff(1, MIDI_NOTE_MAP[queued->index], 100);
    queued->triggered = false;
    if (queued->next != NULL) {
      queued = queued->next;
    } else {
      queued = head;
    }
  }
}



// ensuing, succeeding