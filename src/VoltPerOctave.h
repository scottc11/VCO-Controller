#ifndef __VOLT_PER_OCTAVE_H
#define __VOLT_PER_OCTAVE_H

#include "main.h"
#include "DAC8554.h"
#include "Algorithms.h"

const int PB_RANGE_MAP[8] = {1, 2, 3, 4, 5, 7, 10, 12};
class VoltPerOctave
{
public:
    DAC8554 *dac;                                 // pointer to 16 bit DAC driver
    DAC8554::Channels dacChannel;                 // DAC channel
    
    uint16_t currOutput;                          // value being output to the DAC
    int currNoteIndex;

    uint16_t dacSemitone = 938;                 //
    uint16_t dacVoltageMap[DAC_1VO_ARR_SIZE];   // pre/post calibrated 16-bit DAC values

    // PITCH BEND
    int pbRangeIndex = 4;         // an index value which gets mapped to PB_RANGE_MAP
    int pbNoteOffset;             // the amount of pitch bend to apply to the 1v/o DAC output. Can be positive/negative centered @ 0

    uint16_t maxPitchBend;        // must be a float!
    uint16_t minPitchBend = 0;    // should always be 0
    uint16_t currPitchBend;       // the amount of pitch bend to apply to the 1v/o DAC output. Can be positive/negative centered @ 0

    VoltPerOctave(){};

    void init();
    void updateDAC(int index, uint16_t pitchBend);
    void setPitchBendRange(int value);
    void setPitchBend(uint16_t value);
    void bend(uint16_t value);
    uint16_t calculatePitchBend(int input, int min, int max);
    void resetVoltageMap();
};

#endif