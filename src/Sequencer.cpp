#include "TouchChannel.h"

void TouchChannel::clearEventSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_STEPS; i++)
    {
        events[i].active = false;
        events[i].pitchBend = 0;
        events[i].cvOutput = 0;
    }
    sequenceContainsEvents = false; // after deactivating all events in list, set this flag to false
};

void TouchChannel::clearPitchBendSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_STEPS; i++)
    {
        events[i].pitchBend = 0;
        events[i].cvOutput = 0;
    }
};

void TouchChannel::createEvent(int position, int noteIndex)
{

    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    events[position].noteIndex = noteIndex;
    events[position].active = true;
    events[position].triggered = false;
};

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