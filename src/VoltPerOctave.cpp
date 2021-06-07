#include "VoltPerOctave.h"

void VoltPerOctave::init() {
    
    // copy default pre-calibrated dac voltage values into class object member
    for (int i = 0; i < DAC_1VO_ARR_SIZE; i++) {
        dacVoltageMap1D[i] = DAC_VOLTAGE_VALUES[i];
    }
    
    dac->init();
    generate2DVoltageMap();
    setPitchBendRange(1); // default to a whole tone
}

/**
 * Set the pitch bend range to be applied to 1v/o output
 * NOTE: There are 12 notes, but only 8 possible PB range options, meaning there are preset values for each PB range option via PB_RANGE_MAP global
 * @param value number beyween 0..7
*/
void VoltPerOctave::setPitchBendRange(int value)
{
    if (value < 8)
    {
        pbRangeIndex = value;
        maxPitchBend = dacSemitone * PB_RANGE_MAP[pbRangeIndex]; // multiply semitone DAC value by the max desired number of semitones to be bent
    }
}

// void TouchChannel::setPitchBendOffset(uint16_t value)
// {
//     if (bender.isIdle()) // may be able to move this line into handlevalue()
//     {
//         if (value > bender.zeroBend && value < bender.maxBend)
//         {
//             pbNoteOffset = ((pbOffsetRange / (bender.maxBend - bender.zeroBend)) * (value - bender.zeroBend)) * -1; // inverted
//         }
//         else if (value < bender.zeroBend && value > bender.minBend)
//         {
//             pbNoteOffset = ((pbOffsetRange / (bender.minBend - bender.zeroBend)) * (value - bender.zeroBend)) * 1; // non-inverted
//         }
//     }
//     else
//     {
//         pbNoteOffset = 0;
//     }
// }

/**
 * Scale an input value to a number between 0 and maxPitchBend
 * @param input ADC input
 * @param min range floor of input
 * @param max range ceiling of input
*/
uint16_t VoltPerOctave::calculatePitchBend(int input, int min, int max)
{
    currPitchBend = scaleIntToRange(input, min, max, minPitchBend, maxPitchBend);
    return currPitchBend;
}

/**
 * @param index target degree index value. range if 0..7
 * @param octave index for mapping target octave to DAC_OCTAVE_MAP. Range of 0..3 
 * @param degree switch state for corrosponding degree. Range of 0..2
 * @param pitchBend DAC value corrosponing to the amount of pitch bend to apply to output. positive or negative
*/
void VoltPerOctave::updateDAC(int index, int octave, int degree, uint16_t pitchBend)
{
    uint16_t value = dacVoltageMap2D[index + DAC_OCTAVE_MAP[octave]][degree] + pitchBend;
    dac->write(dacChannel, value);
}

/**
 * this function takes a 1D array and converts it into a 2D array formatted as [[0, 1, 2], ...]
 * take the first 12 values from dacVoltageValues. find the difference dacVoltageValues[i]
*/
void VoltPerOctave::generate2DVoltageMap()
{
    int octaveIndexes[4] = {0, 12, 24, 36};
    int multiplier = 1;

    for (int oct = 0; oct < 4; oct++)
    {
        int index = octaveIndexes[oct];
        int limit = 8 * multiplier;
        for (int i = limit - 8; i < limit; i++)
        {
            dacVoltageMap2D[i][0] = dacVoltageMap1D[index];
            dacVoltageMap2D[i][1] = dacVoltageMap1D[index + 1];
            dacVoltageMap2D[i][2] = dacVoltageMap1D[index + 2];
            index += 2;
        }
        multiplier += 1;
    }
}