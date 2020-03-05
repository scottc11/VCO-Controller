#ifndef __TOUCH_CHANNEL_H
#define __TOUCH_CHANNEL_H

#include "main.h"
#include "Metronome.h"
#include "Degrees.h"
#include "ShiftRegister.h"
#include "MCP23017.h"
#include "MCP4922.h"
#include "CAP1208.h"
#include "TCA9544A.h"
#include "MIDI.h"
#include "QuantizeMethods.h"
#include "BitwiseMethods.h"


typedef struct EventNode {
  uint8_t index;             // note index between 0 and 7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;


class TouchChannel {
  private:  
    enum SWITCH_STATES {
      // octave switch
      OCTAVE_UP = 0b00001000,
      OCTAVE_DOWN = 0b00000100,
    };

    enum NoteState {
      ON,
      OFF,
      SUSTAIN,
    };

    enum Mode {
      MONOPHONIC,
      QUANTIZER,
      LOOPER,
    };

  public:  
    int channel;                    // 0 based index to represent channel
    bool isSelected;
    Mode mode;                      // which mode channel is currently in
    QuantizeMode timeQuantizationMode;
    DigitalOut gateOut;             // gate output pin
    DigitalOut ctrlLed;             // via global controls
    Metronome *metronome;
    MIDI *midi;                     // pointer to mbed midi instance
    CAP1208 *touch;                 // i2c touch IC
    MCP4922 *dac;                   // pointer to dual channel digital-analog-converter
    MCP4922::_DAC dacChannel;       // which dac to address
    MCP23017 *io;                   // for leds and switches
    Degrees *degrees;
    InterruptIn touchInterupt;
    InterruptIn ioInterupt;          // gpio interupt pin
    AnalogIn cvInput;                // CV Input Pin
    volatile bool switchHasChanged;  // toggle switches interupt flag
    volatile bool touchDetected;
    
    // quantizer variables
    int activeDegrees;               // 8 bits to determine which scale degrees are presently active/inactive (active = 1, inactive= 0)
    int numActiveDegrees;            // number of degrees which are active (to quantize voltage input)
    int activeDegreeValues[8];       // array to hold currently active scale degree values to output to DAC (ex. {136.5, 341.25, 682.50, 819.0, 0, 0, 0, 0} )
    int voltageInputMap[8];          // holds values between 0 and 1023 in order to map analogRead(voltage_input_pin) to the active_degree_values array

    // looper variables
    EventNode* head;
    EventNode* newEvent;             // to be created and deleted everytime a user presses event create button
    EventNode* queuedEvent;          // the currently active / next / ensuing / succeeding event
    bool enableLoop = false;         // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
    volatile int numLoopSteps;
    volatile int currStep;           // the current 'step' of the loop (lowest value == 1)
    volatile int currPosition;       // the current position in the loop measured by PPQN (lowest value == 1)
    volatile int currTick;           // the current PPQN position of the step (0..PPQN) (lowest value == 1)
    volatile int loopLength;         // how many PPQN (in total) the loop contains

    uint8_t ledStates;
    unsigned int currCVInputValue; // 16 bit value (0..65,536)
    unsigned int prevCVInputValue; // 16 bit value (0..65,536)
    int touched;                 // variable for holding the currently touched degrees
    int prevTouched;             // variable for holding the previously touched degrees
    int currSwitchStates;        // value to hold the current octave and mode switch states
    int prevSwitchStates;        // value to hold the previous octave and mode switch states
    int currOctave;              // current octave value between 0..3
    int prevOctave;              // previous octave value
    int counter;
    int currNoteIndex;
    int prevNoteIndex;
    
    int leds[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };

    TouchChannel(
        int _channel,
        PinName gateOutPin,
        PinName ioIntPin,
        PinName tchIntPin,
        PinName ctrlLedPin,
        PinName cvInputPin,
        CAP1208 *touch_ptr,
        Degrees *degrees_ptr,
        MCP23017 *io_p,
        MIDI *midi_p,
        Metronome *_clock,
        MCP4922 *dac_ptr,
        MCP4922::_DAC _dacChannel
      ) : 
      gateOut(gateOutPin),
      ioInterupt(ioIntPin, PullUp),
      touchInterupt(tchIntPin, PullUp),
      ctrlLed(ctrlLedPin),
      cvInput(cvInputPin) {
      
      head = NULL;
      newEvent = NULL;
      queuedEvent = NULL;

      touch = touch_ptr;
      degrees = degrees_ptr;
      metronome = _clock;
      dac = dac_ptr;
      dacChannel = _dacChannel;
      io = io_p;
      midi = midi_p;
      touchInterupt.fall(callback(this, &TouchChannel::handleTouchInterupt));
      ioInterupt.fall(callback(this, &TouchChannel::handleioInterupt));
      counter = 0;
      currOctave = 0;
      prevOctave = 0;
      numLoopSteps = DEFAULT_CHANNEL_LOOP_STEPS;
      timeQuantizationMode = QUANT_16;
      currStep = 0;
      currTick = 0;
      currPosition = 0;
      touched = 0;
      prevTouched = 0;
      channel = _channel;
    };

    void init();
    void handleioInterupt() { switchHasChanged = true; }
    void handleTouchInterupt() { touchDetected = true; }
    void poll();
    int readSwitchStates();
    void writeLed(int index, int state);
    void updateLeds(uint8_t touched);
    void setOctaveLed(int octave);
    void setNumLoopSteps(int num);
    void handleCVInput(int value);
    void handleTouch();
    void handleDegreeChange();
    void handleSwitchInterupt();
    void handleModeSwitch(int state);
    void handleOctaveSwitch(int state);
    void tickClock();
    void stepClock();
    int quantizePosition(int position);
    void calculateLoopLength();
    int calculateMIDINoteValue(int index, int octave);
    int calculateDACNoteValue(int index, int octave);
    void triggerNote(int index, int octave, NoteState state);
    void freeze(bool enable);
    void reset();

    // EVENT LOOP FUNCTIONS
    void clearEventList();
    int length();
    void createEvent(int position, int noteIndex);
    void addEventToList(int endPosition);
    void handleQueuedEvent(int position);

    // QUANTIZER FUNCTIONS
    void setActiveDegrees(int degree);
};

#endif