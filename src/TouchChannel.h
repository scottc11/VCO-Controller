#ifndef __EVENT_LIST_H
#define __EVENT_LIST_H

#include "main.h"
#include "BeatClock.h"
#include "ShiftRegister.h"
#include "MCP23017.h"
#include "MCP4922.h"
#include "CAP1208.h"
#include "TCA9544A.h"
#include "MIDI.h"

typedef struct EventNode {
  uint8_t index;             // note index :: one of 0..7
  uint16_t startPos;         // the point in time in which the EventNode occured
  uint16_t endPos;           // the point in time the EventNode finishes
  bool triggered;            // has the EventNode been triggered
  struct EventNode *next;    // pointer to the 'next' EventNode to occur (linked list)
} EventNode;

// Linked List
class TouchChannel {
  enum Mode {
    MONOPHONIC,
    QUANTIZER,
    LOOPER,
  };

  public:
    int channel;                     // 0 based index to represent channel
    Mode mode;                       // which mode channel is currently in
    bool ETL = false;                // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created
    DigitalOut gateOut;              // gate output pin
    BeatClock * beatClock;
    CAP1208 touch;                   // i2c touch IC
    MCP4922 * dac;                   // pointer to dual channel digital-analog-converter
    MCP4922::_DAC dacChannel;           // which dac to address
    MCP23017 * io;                   // for leds and switches
    InterruptIn touchInterupt;
    InterruptIn ioInterupt;          // gpio interupt pin
    volatile bool switchHasChanged;  // toggle switches interupt flag
    volatile bool touchDetected;
    int touched;                 // variable for holding the currently touched degrees
    int prevTouched;             // variable for holding the previously touched degrees
    int octave;                      // current octave
    int counter;
    
    MIDI *midi;                      // pointer to mbed midi instance
    int leds[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };

    TouchChannel(
        int _channel,
        PinName gateOutPin,
        PinName ioIntPin,
        PinName tchIntPin,
        MCP23017 *io_p,
        MIDI *midi_p,
        BeatClock *_clock,
        MCP4922 *dac_ptr,
        MCP4922::_DAC _dacChannel
      ) : gateOut(gateOutPin), ioInterupt(ioIntPin, PullUp), touchInterupt(tchIntPin, PullUp) {
      
      head = NULL;
      newEvent = NULL;
      queued = NULL;
      beatClock = _clock;
      dac = dac_ptr;
      dacChannel = _dacChannel;
      io = io_p;
      midi = midi_p;
      touchInterupt.fall(callback(this, &TouchChannel::handleTouchInterupt));
      ioInterupt.fall(callback(this, &TouchChannel::handleioInterupt));
      counter = 0;
      octave = 0;
      touched = 0;
      prevTouched = 0;
      channel = _channel;
    }

    void init(I2C *touchI2C, TCA9544A *mux_ptr);
    void handleioInterupt() { switchHasChanged = true; }
    void handleTouchInterupt() { touchDetected = true; }
    void poll();
    void setLed(int led_index);
    void updateLeds(uint8_t touched);
    void setOctaveLed();
    void handleTouch();
    void handleModeSwitch();
    void handleOctaveSwitch();

    void createEvent(int position, int noteIndex);
    void addEvent(int position);
    bool hasEventInQueue();
    void handleQueuedEvent(int position);

  private:
    EventNode* head;
    EventNode* newEvent;  // to be created and deleted everytime a user presses event create button
    EventNode* queued;    // the currently active / next / ensuing / succeeding event
  
    enum SWITCH_STATES {      
      // octave switch
      OCTAVE_UP = 0b00001000,
      OCTAVE_DOWN = 0b00000100,
    };
};

#endif