#ifndef __TOUCH_CHANNEL_H
#define __TOUCH_CHANNEL_H

#include "main.h"
#include "Metronome.h"
#include "Degrees.h"
#include "DAC8554.h"
#include "CAP1208.h"
#include "MPR121.h"
#include "TCA9544A.h"
#include "SX1509.h"
#include "MIDI.h"
#include "QuantizeMethods.h"
#include "BitwiseMethods.h"
#include "ArrayMethods.h"

#define CHANNEL_IO_MODE_PIN 9
#define CHANNEL_LED_MUX_SEL 8
#define CHANNEL_MODE_LED 10
#define CHANNEL_GATE_LED 11
#define NULL_NOTE_INDEX 99  // used to identify a 'null' or 'deleted' sequence event
#define PB_CALIBRATION_RANGE 64
const int PB_RANGE_MAP[8] = { 1, 2, 3, 4, 5, 7, 10, 12 };

static const int OCTAVE_LED_PINS[4] = { 3, 2, 1, 0 };               // io pin map for octave LEDs
static const int CHAN_LED_PINS[8] = { 15, 14, 13, 12, 7, 6, 5, 4 }; // io pin map for channel LEDs
static const int CHAN_TOUCH_PADS[12] = { 7, 6, 5, 4, 3, 2, 1, 0, 3, 2, 1, 0 };

typedef struct QuantDegree {
  int threshold;
  int noteIndex;
} QuantDegree;

typedef struct QuantOctave {
  int threshold;
  int octave;
} QuantOctave;

typedef struct SequenceNode {
  uint8_t activeNotes; // byte for holding active/inactive notes for a chord
  uint8_t noteIndex;   // note index between 0 and 7 NOTE: you could tag on some extra data in the bottom most bits, like gate on / off for example
  uint16_t pitchBend;  // raw ADC value from pitch bend
  bool gate;           // set gate HIGH or LOW
  bool active;         // this will tell the loop whether to trigger an event or not
} SequenceNode;

class TouchChannel {
  private:
    enum SWITCH_STATES {
      OCTAVE_UP = 0b00001000,
      OCTAVE_DOWN = 0b00000100,
    };

    enum NoteState {
      ON,
      OFF,
      SUSTAIN,
      PREV,
      PITCH_BEND
    };

    enum LedColor {
      RED,
      GREEN
    };

  public:
    enum LedState : int
    {
      LOW = 0,
      HIGH = 1,
      BLINK_ON = 2,
      BLINK_OFF = 3,
      DIM_LOW = 4,
      DIM_MEDIUM = 5,
      DIM_HIGH = 6
    };

    enum Mode {
      MONO = 0,
      MONO_LOOP = 1,
      QUANTIZE = 2,
      QUANTIZE_LOOP = 3,
    };

    enum UIMode { // not yet implemented
      DEFAULT_UI,
      LOOP_LENGTH_UI,
      PB_RANGE_UI
    };

    int channel;                    // 0 based index to represent channel
    bool isSelected;
    bool gateState;                 // the current state of the gate output pin
    Mode mode;                      // which mode channel is currently in
    Mode prevMode;                  // used for reverting to previous mode when toggling between UI modes
    UIMode uiMode;                  // for settings and alt LED uis
    DigitalOut gateOut;             // gate output pin
    DigitalOut *globalGateOut;      // 
    Timer *timer;                   // timer for handling duration based touch events
    Ticker *ticker;                 // for handling time based callbacks
    EventQueue *queue;              // event queue for executing touch events
    MIDI *midi;                     // pointer to mbed midi instance
    CAP1208 *touch;                 // i2c touch IC
    MPR121 *touchPads;
    DAC8554 *dac;                   // pointer to 1vo DAC
    DAC8554::Channels dacChannel;   // which dac to address
    DAC8554 *pb_dac;                // pointer to Pitch Bends DAC
    DAC8554::Channels pb_dac_chan;  // which dac to address
    SX1509 *io;                     // IO Expander
    Degrees *degrees;
    InterruptIn ioInterupt;         // for SC1509 3-stage toggle switch + tactile mode button
    AnalogIn cvInput;               // CV input pin for quantizer mode
    AnalogIn pbInput;               // CV input for Pitch Bend

    volatile bool tickerFlag;        // each time the clock gets ticked, this flag gets set to true - then false in polling loop
    volatile bool switchHasChanged;  // toggle switches interupt flag
    volatile bool touchDetected;
    volatile bool modeChangeDetected;

    // SEQUENCER variables
    SequenceNode events[PPQN * MAX_SEQ_STEPS];
    QuantizeMode timeQuantizationMode;
    int prevEventIndex; // index for disabling the last "triggered" event in the loop
    bool sequenceContainsEvents;
    bool clearExistingNodes;   
    bool deleteEvents;
    bool enableLoop = false;   // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
    bool recordEnabled;        //
    int prevNodePosition;      // represents the last node in the sequence which got triggered (either HIGH or LOW)
    volatile int numLoopSteps; // how many steps the sequence contains (before applying the multiplier)
    volatile int currStep;     // the current 'step' of the loop (lowest value == 0)
    volatile int currPosition; // the current position in the in the entire sequence (measured by PPQN)
    volatile int currTick;     // the current PPQN position of the step (0..PPQN) (lowest value == 0)
    int totalPPQN;             // how many PPQN the sequence currently contains (equal to totalSteps * PPQN)
    int totalSteps;            // how many Steps the sequence contains (in total ie. numLoopSteps * loopMultiplier)
    int loopMultiplier;        // number between 1 and 4 based on Octave Leds of channel

    // quantizer variables
    bool quantizerHasBeenInitialized;
    bool enableQuantizer;                 // by default set to true, only ever changes with a 'freeze' event
    int activeDegrees;                    // 8 bits to determine which scale degrees are presently active/inactive (active = 1, inactive= 0)
    int activeOctaves;                    // 4-bits to represent which octaves external CV will get mapped to (active = 1, inactive= 0)
    int numActiveDegrees;                 // number of degrees which are active (to quantize voltage input)
    int numActiveOctaves;                 // number of active octaves for mapping CV to
    int activeDegreeLimit;                // the max number of degrees allowed to be enabled at one time.
    QuantDegree activeDegreeValues[8];    // array which holds noteIndex values and their associated DAC/1vo values
    QuantOctave activeOctaveValues[OCTAVE_COUNT];

    // Pitch Bend
    int currPitchBend;                       // 16 bit value (0..65,536)
    int prevPitchBend;                       // 16 bit value (0..65,536)
    int pbOffsetIndex = 4;                   // an index value which gets mapped to PB_RANGE_MAP
    float pbOffsetRange;                     // must be a float!
    float cvOffsetRange = 32767;             // +- 8.1v DAC range (must be a float!)
    int pbNoteOffset;                        // the amount of pitch bend to apply to the 1v/o DAC output. Can be positive/negative centered @ 0
    int cvOffset;                            // the amount of Control Voltage to apply Pitch Bend DAC
    int pbCalibration[PB_CALIBRATION_RANGE]; // an array which gets populated during initialization phase to determine a debounce value + zeroing
    uint16_t pbZero;                         // the average ADC value when pitch bend is idle
    uint16_t pbMax;                          // the minimum value the ADC can achieve when Pitch Bend fully pulled
    uint16_t pbMin;                          // the maximum value the ADC can achieve when Pitch Bend fully pressed
    int pbDebounce;                          // for debouncing the ADC when Pitch Bend is idle
    bool pbEnabled;                          // for toggleing on/off the pitch bend effect to JUST the 1vo output
    

    float dacSemitone = 938.0;               // must be a float, as it gets divided down to a num between 0..1
    uint16_t dacVoltageMap[32][3];
    uint16_t dacVoltageValues[CALIBRATION_LENGTH];               // pre/post calibrated 16-bit DAC values

    int redLedPins[8] = { 14, 12, 10, 8, 6, 4, 2, 0 };    // hardcoded values to be passed to the 16 chan LED driver
    int greenLedPins[8] = { 15, 13, 11, 9, 7, 5, 3, 1 };  // hardcoded values to be passed to the 16 chan LED driver
    
    uint16_t ledStates;                   // 16 bits to represent each bi-color led  | 0-Red | 0-Green | 1-Red | 1-Green | 2-Red | 2-Green | etc...
    
    unsigned int currCVInputValue;        // 16 bit value (0..65,536)
    unsigned int prevCVInputValue;        // 16 bit value (0..65,536)

    int touched;                          // variable for holding the currently touched degrees
    int prevTouched;                      // variable for holding the previously touched degrees
    int currOctave;                       // current octave value between 0..3
    int prevOctave;                       // previous octave value

    int currNoteIndex;
    int prevNoteIndex;

    int currModeBtnState;        // ** to be refractored into MomentaryButton class
    int prevModeBtnState;        // ** to be refractored into MomentaryButton class
    
    bool freezeChannel;          //

    TouchChannel(
        int _channel,
        Timer *timer_ptr,
        Ticker *ticker_ptr,
        EventQueue *queue_ptr,
        DigitalOut *globalGateOut_ptr,
        PinName gateOutPin,
        PinName ioIntPin,
        PinName cvInputPin,
        PinName pbInputPin,
        MPR121 *touch_ptr,
        SX1509 *io_ptr,
        Degrees *degrees_ptr,
        MIDI *midi_p,
        DAC8554 *dac_ptr,
        DAC8554::Channels _dacChannel,
        DAC8554 *pb_dac_ptr,
        DAC8554::Channels pb_dac_channel
        ) : gateOut(gateOutPin), ioInterupt(ioIntPin, PullUp), cvInput(cvInputPin), pbInput(pbInputPin)
    {
      globalGateOut = globalGateOut_ptr;
      timer = timer_ptr;
      ticker = ticker_ptr;
      queue = queue_ptr;
      touchPads = touch_ptr;
      io = io_ptr;
      degrees = degrees_ptr;
      dac = dac_ptr;
      dacChannel = _dacChannel;
      pb_dac = pb_dac_ptr;
      pb_dac_chan = pb_dac_channel;
      midi = midi_p;
      ioInterupt.fall(callback(this, &TouchChannel::ioInteruptFn));
      channel = _channel;
    };

    void init();
    void poll();
    void onTouch(uint8_t pad);
    void onRelease(uint8_t pad);

    void ioInteruptFn() { modeChangeDetected = true; }

    void initIOExpander();
    void setLed(int index, LedState state, bool settingUILed=false);
    void setOctaveLed(int octave, LedState state, bool settingUILed=false);
    void setModeLed(LedState state);
    void setGateLed(LedState state);
    void setAllLeds(int state);
    void updateOctaveLeds(int octave);
    void updateLoopMultiplierLeds();
    void updateActiveDegreeLeds();
    void updateLeds(uint8_t touched);  // could be obsolete
    
    // Pitch Bend
    void calibratePitchBend();
    void updatePitchBendDAC(uint16_t value);
    void handlePitchBend();
    void setPitchBendRange(int touchedIndex);
    void setPitchBendOffset(uint16_t pitchBend);

    void handleDegreeChange();
    void handleIOInterupt();
    void setMode(Mode targetMode);
    
    void tickClock();
    void stepClock();
    void resetClock();
    
    int quantizePosition(int position);
    int calculateMIDINoteValue(int index, int octave);
    int calculateDACNoteValue(int index, int octave);

    void setOctave(int value);
    void triggerNote(int index, int octave, NoteState state, bool blinkLED=false);
    void setGate(bool state);
    void setGlobalGate(bool state);
    void freeze(bool enable);
    void reset();
    void generateDacVoltageMap();

    // UI METHODS
    void enableUIMode(UIMode target);
    void disableUIMode();
    void updateLoopLengthUI();
    void handleLoopLengthUI();
    void updatePitchBendRangeUI();

    // SEQUENCER METHODS
    void initSequencer();
    void clearEvent(int position);
    void clearEventSequence();
    void clearPitchBendSequence();
    void createEvent(int position, int noteIndex, bool gate);
    void createChordEvent(int position, uint8_t notes);
    void createPitchBendEvent(int position, uint16_t pitchBend);
    void clearLoop(); // refractor into clearEventSequence()
    void enableLoopMode();
    void disableLoopMode();
    void setLoopLength(int num);
    void setLoopMultiplier(int value);
    void setLoopTotalPPQN();  // refractor into metronom class
    void setLoopTotalSteps(); // refractor into metronom class
    void handleSequence(int position);

    // QUANTIZER METHODS
    void initQuantizerMode();
    void handleCVInput(int value);
    void setActiveDegrees(int degrees);
    void setActiveDegreeLimit(int value);
    void setActiveOctaves(int octave);
    
};

#endif