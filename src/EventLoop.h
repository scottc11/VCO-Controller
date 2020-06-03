#ifndef __EVENT_LOOP_H
#define __EVENT_LOOP_H

#include "QuantizeMethods.h"
#include "BitwiseMethods.h"

typedef struct LoopNode {
  uint8_t activeNotes;       // byte for holding active/inactive notes for a chord
  uint8_t noteIndex;         // note index between 0 and 7
  bool triggered;            // has the LoopNode been triggered
} LoopNode;

class EventLoop {

public:
  EventLoop() {
    // initialize;
  }

  volatile LoopNode events[768];
  QuantizeMode timeQuantizationMode;
  bool enableLoop = false;            // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
  volatile int numLoopSteps;
  volatile int currStep;              // the current 'step' of the loop (lowest value == 1)
  volatile int currPosition;          // the current position in the loop measured by PPQN (lowest value == 1)
  volatile int currTick;              // the current PPQN position of the step (0..PPQN) (lowest value == 1)
  volatile int loopLength;            // how many PPQN (in total) the loop contains
  int loopMultiplier;                 // number between 1 and 4 based on Octave Leds of channel

  void clearEventList(){};
  void createEvent(int position, int noteIndex){};
  void createChordEvent(int position, uint8_t notes){};
  void addEventToList(int endPosition){};
  void handleQueuedEvent(int position){};

  int getListLength(){return 1;};

};


#endif