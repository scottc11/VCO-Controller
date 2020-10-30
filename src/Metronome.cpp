#include "Metronome.h"

// clock initialization
void Metronome::init() {
  this->updateTempo(tickInterval);
  this->pollTempoPot();
}

void Metronome::poll() {
  this->pollTempoPot();
}

void Metronome::pollTempoPot() {
  newTempoPotValue = tempoPot.read_u16();
  int debounce = 250;
  if (newTempoPotValue != oldTempoPotValue) {
    if (newTempoPotValue > oldTempoPotValue + debounce || newTempoPotValue < oldTempoPotValue - debounce)
    {
      // 65535 / 150 = 437
      int index = newTempoPotValue / 437;
      this->updateTempo(usTempoMap[index]);
      oldTempoPotValue = newTempoPotValue;
    }
  }
}

void Metronome::updateTempo(int us) {
  tickInterval = us;
  ticker.attach_us(callback(this, &Metronome::tick), tickInterval);
}

void Metronome::tick() {
  if (currTick == 1) {
    tempoLed.write(1);
  } else {
    tempoLed.write(0);
  }
  
  currTick += 1;
  position += 1;

  if (currTick > ticksPerStep) {
    this->step();
  }
}

void Metronome::step() {
  currTick = 1;
  currStep += 1;
  
  // reset step count / reset loop
  if (currStep > numSteps) {
    currStep = 1;
    position = 1;
  }
}



void Metronome::setNumberOfSteps(int num) {
  numSteps = num;
}