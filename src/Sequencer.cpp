#include "TouchChannel.h"

void TouchChannel::initSequencer()
{
    numLoopSteps = DEFAULT_CHANNEL_LOOP_STEPS;
    loopMultiplier = 1;
    timeQuantizationMode = QUANT_NONE;
    currStep = 0;
    currTick = 0;
    currPosition = 0;
    setLoopTotalSteps();
    setLoopTotalPPQN();
    clearEventSequence(); // initialize values in sequence array
}

/**
 * Sequence Handler which gets called during polling / every clocking event
*/ 
void TouchChannel::handleSequence(int position)
{

    if (events[position].active) {
        
        if (clearExistingNodes) // when a node is being created (touched degree has not yet been released), this flag gets set to true so that the sequence handler clears existing nodes
        {
            // if previous event overlaps with new event
            if (events[prevNodePosition].gate == HIGH) {
                int newPosition = position == 0 ? totalPPQN : position - 1;
                createEvent(newPosition - 1, events[prevNodePosition].noteIndex, LOW); // create a copy of event with gate == LOW @ currPos - 1
            }
            // if new event overlaps succeeding events, overwrite those events
            if (events[position].active) {
                clearEvent(position);
            }
        }
        else
        {
            if (events[position].gate == HIGH)
            {
                prevNodePosition = position;                             // store position into variable
                triggerNote(events[position].noteIndex, currOctave, ON); // trigger note ON
            }
            else
            {
                // cleanup: if this 'active' LOW node does not match the last active HIGH node, delete it
                if (events[prevNodePosition].noteIndex != events[position].noteIndex) {
                    clearEvent(position);
                } else {
                    prevNodePosition = position;                              // store position into variable
                    triggerNote(events[position].noteIndex, currOctave, OFF); // trigger note OFF
                }
            }
        }

    }

    // if (events[position].active)
    // {
    //     if (events[position].triggered == false)
    //     {
    //         events[prevEventIndex].triggered = false;
    //         events[position].triggered = true;
    //         prevEventIndex = position;
            
    //         switch (mode)
    //         {
    //         case MONO_LOOP:
    //             triggerNote(prevNoteIndex, currOctave, OFF);
    //             triggerNote(events[position].noteIndex, currOctave, ON);
    //             break;
    //         case QUANTIZE_LOOP:
    //             setActiveDegrees(events[position].activeNotes);
    //             break;
    //         }
    //     }
    // }
    // else
    // {
    //     if (events[prevEventIndex].triggered)
    //     {
    //         events[prevEventIndex].triggered = false;
    //         triggerNote(prevNoteIndex, currOctave, OFF);
    //     }
    // }

    // always handle pitch bend value
    triggerNote(currNoteIndex, currOctave, PITCH_BEND);
}

void TouchChannel::clearEventSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_STEPS; i++)
    {
        clearEvent(i);
    }
    sequenceContainsEvents = false; // after deactivating all events in list, set this flag to false
};

void TouchChannel::clearPitchBendSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_STEPS; i++)
    {
        events[i].pitchBend = pbZero;
    }
};

void TouchChannel::createEvent(int position, int noteIndex, bool gate)
{

    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    events[position].noteIndex = noteIndex;
    events[position].gate = gate;
    events[position].active = true;
    events[position].triggered = false;
};

void TouchChannel::clearEvent(int position)
{
    events[position].noteIndex = NULL_NOTE_INDEX;
    events[position].active = false;
    events[position].gate = LOW;
    events[position].pitchBend = pbZero;
    events[position].triggered = false;
}

void TouchChannel::createChordEvent(int position, uint8_t notes)
{

    if (sequenceContainsEvents == false)
    {
        sequenceContainsEvents = true;
    }

    events[position].activeNotes = notes;
    events[position].active = true;
    events[position].triggered = false;
};