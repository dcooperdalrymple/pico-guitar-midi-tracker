#pragma once

#include "pico/stdlib.h"
#include "midi.hpp"

#define LOG_2               0.693147f
#define D_NOTE              1.059463f
#define LOG_D_NOTE          0.057762f
#define D_NOTE_SQRT         1.029302f
#define ROOT_FREQ           440.0f
#define ROOT_OCTAVE         4

class Tracker {

public:

    Tracker(Midi *mid);
    virtual ~Tracker() {};

    void setTriggerLevel(uint8_t val);
    void setVelocityAmount(uint8_t val);
    void setOctave(int16_t oct);

    void panic();

    virtual void process(size_t len, uint8_t *data) = 0;

protected:

    void detectNote(float freq);
    void removeNote();

    float fSAMPLE_RATE;

    uint8_t triggerLevel;
    float velocityAmount;

private:

    Midi *midi;
    void sendNoteOn(int note);
    void sendNoteOff(int note);

    uint led_slice;
    void setLedValue(uint8_t value);

    void calculateNoteFrequencies(float val);

    int16_t notenum, octave;
    float noteFreq, detectFreq, rootFreq;
    float noteFreqs[12];
    float logFreqs[12];

};
