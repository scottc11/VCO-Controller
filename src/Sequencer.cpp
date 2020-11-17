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
    if (clearExistingNodes)    // when a node is being created (touched degree has not yet been released), this flag gets set to true so that the sequence handler clears existing nodes
    {
        
    }

    if (events[position].active)
    {
        if (events[position].triggered == false)
        {
            events[prevEventIndex].triggered = false;
            events[position].triggered = true;
            prevEventIndex = position;
            switch (mode)
            {
            case MONO_LOOP:
                triggerNote(prevNoteIndex, currOctave, OFF);
                triggerNote(events[position].noteIndex, currOctave, ON);
                break;
            case QUANTIZE_LOOP:
                setActiveDegrees(events[position].activeNotes);
                break;
            }
        }
    }
    else
    {
        if (events[prevEventIndex].triggered)
        {
            events[prevEventIndex].triggered = false;
            triggerNote(prevNoteIndex, currOctave, OFF);
        }
    }

    // always handle pitch bend value
    triggerNote(currNoteIndex, currOctave, PITCH_BEND);
}

void TouchChannel::clearEventSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_STEPS; i++)
    {
        events[i].active = false;
        events[i].pitchBend = pbZero;
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

void TouchChannel::createEvent(int position, int noteIndex)
{

    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    events[position].noteIndex = noteIndex;
    events[position].active = true;
    events[position].triggered = false;
};

void TouchChannel::clearEvent(int position)
{
    events[position].active = false;
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