#ifndef __TOUCH_CHANNEL_H
#define __TOUCH_CHANNEL_H

#include "main.h"
#include "Metronome.h"
#include "Degrees.h"
#include "DAC8554.h"
#include "CAP1208.h"
#include "TCA9544A.h"
#include "TLC59116.h"
#include "MCP4461.h"
#include "MIDI.h"
#include "QuantizeMethods.h"
#include "BitwiseMethods.h"
#include "EventLoop.h"

typedef struct QuantizerValue {
  int threshold;
  int noteIndex;
} QuantizerValue;


class TouchChannel : public EventLoop {
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
    };

    enum LedColor {
      RED,
      GREEN
    };

    enum Mode {
      MONO = 0,
      MONO_LOOP = 1,
      QUANTIZE = 2,
      QUANTIZE_LOOP = 3,
      FREEZE = 4,
    };

    enum UIMode { // not yet implemented
      DEFAULT_UI,
      LOOP_LENGTH_UI,
    };

  public:
    int channel;                    // 0 based index to represent channel
    bool isSelected;
    Mode mode;                      // which mode channel is currently in
    Mode prevMode;                  // used for reverting to previous mode when toggling between UI modes
    UIMode uiMode;                  // for settings and alt LED uis
    DigitalOut gateOut;             // gate output pin
    DigitalOut ctrlLed;             // via global controls
    DigitalIn modeBtn;              // tactile button for toggleing between channel modes
    Timer *gestureTimer;            // timer for handling duration based touch events
    MIDI *midi;                     // pointer to mbed midi instance
    CAP1208 *touch;                 // i2c touch IC
    DAC8554 *dac;                   // pointer to dual channel digital-analog-converter
    DAC8554::Channels dacChannel;   // which dac to address
    TLC59116 *leds;                 // led driver
    TLC59116 *octLeds;              // led driver
    MCP4461 *digiPot;               // digitial potentiometer
    MCP4461::Wiper wiperChannel;    // wiper channel to use
    int *octLedPins;                // pin list for led Driver
    Degrees *degrees;
    InterruptIn touchInterupt;
    AnalogIn cvInput;                // CV input pin for quantizer mode
    AnalogIn slewCvInput;            // CV input pin for slew control

    volatile bool switchHasChanged;  // toggle switches interupt flag
    volatile bool touchDetected;
    
    // quantizer variables
    bool enableQuantizer;                 // by default set to true, only ever changes with a 'freeze' event
    int activeDegrees;                    // 8 bits to determine which scale degrees are presently active/inactive (active = 1, inactive= 0)
    int numActiveDegrees;                 // number of degrees which are active (to quantize voltage input)
    int activeDegreeLimit;                // the max number of degrees allowed to be enabled at one time.
    QuantizerValue activeDegreeValues[8]; // array which holds noteIndex values and their associated DAC/1vo values

    int dacVoltageMap[8][3];
    int dacVoltageValues[13];

    int redLedPins[8] = { 14, 12, 10, 8, 6, 4, 2, 0 };    // hardcoded values to be passed to the 16 chan LED driver
    int greenLedPins[8] = { 15, 13, 11, 9, 7, 5, 3, 1 };  // hardcoded values to be passed to the 16 chan LED driver
    uint16_t ledStates;            // 16 bits to represent each bi-color led  | 0-Red | 0-Green | 1-Red | 1-Green | 2-Red | 2-Green | etc...
    unsigned int currCVInputValue; // 16 bit value (0..65,536)
    unsigned int prevCVInputValue; // 16 bit value (0..65,536)
    float currSlewCV;              // represented as a float in the range [0.0, 1.0]
    float prevSlewCV;              // represented as a float in the range [0.0, 1.0]
    int touched;                   // variable for holding the currently touched degrees
    int prevTouched;               // variable for holding the previously touched degrees
    int currOctave;                // current octave value between 0..3
    int prevOctave;                // previous octave value
    int counter;
    int currNoteIndex;
    int prevNoteIndex;

    int currModeBtnState;        // ** to be refractored into MomentaryButton class
    int prevModeBtnState;        // ** to be refractored into MomentaryButton class
    
    int currVCOInputVal;                 // the current sampled value of sinewave input
    int prevVCOInputVal;                 // the previous sampled value of sinewave input
    bool slopeIsPositive;                // whether the sine wave is rising or falling
    volatile float vcoFrequency;                  // 
    volatile float vcoFreqAvrg;                   // the running average of frequency calculations
    volatile int vcoPeriod;
    volatile int numSamplesTaken;                 // How many times we have sampled the zero crossing (used in frequency calculation formula)
    int calibrationIndex;                 // when calibrating, increment this value to step each voltage representation of a semi-tone via dacVoltageValues[]
    bool calibrationFinished;            // flag to tell program when calibration process is finished
    
    volatile bool readyToCalibrate;      // flag telling polling loop when enough freq average samples have been taken to accurately calibrate
    volatile int freqSampleIndex = 0;        // incrementing value to place current frequency sample into array
    volatile float freqSamples[MAX_FREQ_SAMPLES]; // array of frequency samples for obtaining the running average of the VCO


    TouchChannel(
        int _channel,
        Timer *timer_ptr,
        PinName modePin,
        PinName gateOutPin,
        PinName tchIntPin,
        PinName ctrlLedPin,
        PinName cvInputPin,
        PinName slewInputPin,
        CAP1208 *touch_ptr,
        TLC59116 *leds_ptr,
        TLC59116 *octLeds_ptr,
        int *_octLedPins,
        Degrees *degrees_ptr,
        MIDI *midi_p,
        DAC8554 *dac_ptr,
        DAC8554::Channels _dacChannel,
        MCP4461 *digiPot_ptr,
        MCP4461::Wiper _wiperCh
      ) : modeBtn(modePin), gateOut(gateOutPin), touchInterupt(tchIntPin, PullUp), ctrlLed(ctrlLedPin), cvInput(cvInputPin), slewCvInput(slewInputPin) {
      
      gestureTimer = timer_ptr;
      touch = touch_ptr;
      leds = leds_ptr;
      octLeds = octLeds_ptr;
      octLedPins = _octLedPins;
      degrees = degrees_ptr;
      dac = dac_ptr;
      dacChannel = _dacChannel;
      midi = midi_p;
      touchInterupt.fall(callback(this, &TouchChannel::handleTouchInterupt));
      channel = _channel;
      digiPot = digiPot_ptr;
      wiperChannel = _wiperCh;
    };

    void init();
    void poll();
    void handleTouchInterupt() { touchDetected = true; }
    
    void setLed(int index, LedState state, bool settingUILed=false);
    void setOctaveLed(int octave, LedState state, bool settingUILed=false);
    void setUILed(int index, LedState state);
    void setUIOctaveLed(int index, LedState state);
    void setAllLeds(int state);
    void updateOctaveLeds(int octave);
    void updateLoopMultiplierLeds();
    void updateActiveDegreeLeds();
    void updateLeds(uint8_t touched);  // could be obsolete

    void handleTouch();
    void handleDegreeChange();
    void toggleMode();
    void setMode(Mode targetMode);
    
    void tickClock();
    void stepClock();
    void resetClock();
    
    int quantizePosition(int position);
    int calculateMIDINoteValue(int index, int octave);
    int calculateDACNoteValue(int index, int octave);
    
    void setSlewAmount(float val);

    void setOctave(int value);
    void triggerNote(int index, int octave, NoteState state, bool dimLed=false);
    void freeze(bool enable);
    void reset();

    void enableLoopLengthUI();
    void disableLoopLengthUI();
    void updateLoopLengthUI();
    void handleLoopLengthUI();

    void clearLoop();
    void enableLoopMode();
    void disableLoopMode();
    void setLoopLength(int num);
    void setLoopMultiplier(int value);
    void setLoopTotalPPQN();  // refractor into metronom class
    void setLoopTotalSteps(); // refractor into metronom class
    
    void handleQueuedEvent(int position);

    // QUANTIZE FUNCTIONS
    void initQuantizer();
    void handleCVInput(int value);
    void setActiveDegrees(int degrees);
    void setActiveDegreeLimit(int value);

    void calibrateVCO();
    void sampleVCOFrequency();
    float calculateAverageFreq();
};

#endif