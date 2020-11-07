#ifndef __EVENT_LOOP_H
#define __EVENT_LOOP_H

#include "QuantizeMethods.h"
#include "BitwiseMethods.h"

typedef struct LoopNode {
  LoopNode() : pitchBend(0), cvOutput(0) {}
  uint8_t activeNotes;       // byte for holding active/inactive notes for a chord
  uint8_t noteIndex;         // note index between 0 and 7
  uint16_t pitchBend;        // how much pitch bend to apply to the currently outputed note
  uint16_t cvOutput;         // how much raw CV to apply to the CV Output (pitchbend DAC)
  bool triggered;            // has the LoopNode been triggered
  bool active;               // this will tell the loop whether to trigger an event or not
} LoopNode;

class EventLoop {

public:
  EventLoop() {
    // initialize;
  }

  LoopNode events[PPQN * MAX_LOOP_STEPS];
  QuantizeMode timeQuantizationMode;
  int prevEventIndex;                 // index for disabling the last "triggered" event in the loop
  bool loopContainsEvents;
  bool deleteEvents;
  bool enableLoop = false;            // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
  bool recordEnabled;                 // 
  volatile int numLoopSteps;
  volatile int currStep;              // the current 'step' of the loop (lowest value == 0)
  volatile int currPosition;          // the current position in the loop measured by PPQN (lowest value == 0)
  volatile int currTick;              // the current PPQN position of the step (0..PPQN) (lowest value == 0)
  int totalPPQN;                      // how many PPQN (in total) the loop contains
  int totalSteps;                     // how many Steps (in total) the loop contains
  int loopMultiplier;                 // number between 1 and 4 based on Octave Leds of channel
  

  void clearEventLoop() {
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_LOOP_STEPS; i++) {
      events[i].active = false;
      events[i].pitchBend = 0;
      events[i].cvOutput = 0;
    }
    loopContainsEvents = false; // after deactivating all events in list, set this flag to false
  };

  void clearPitchBend()
  {
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_LOOP_STEPS; i++)
    {
      events[i].pitchBend = 0;
      events[i].cvOutput = 0;
    }
    
  };

  void createEvent(int position, int noteIndex) {

    if (loopContainsEvents == false) {
      loopContainsEvents = true;
    }

    events[position].noteIndex = noteIndex;
    events[position].active = true;
    events[position].triggered = false;
  };
  
  void createChordEvent(int position, uint8_t notes) {
    
    if (loopContainsEvents == false) {
      loopContainsEvents = true;
    }

    events[position].activeNotes = notes;
    events[position].active = true;
    events[position].triggered = false;

  };

  void addEventToList(int endPosition){};
  void handleQueuedEvent(int position){};

  int getListLength(){return 1;};

};


#endif