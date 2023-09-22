/* tuneit.c -- Detect fundamental frequency of a sound
* Copyright (C) 2004, 2005  Mario Lang <mlang@delysid.org>
*
* Modified for rakarrack by Josep Andreu
* MIDIConverter.h  MIDIConverter definitions
*
* Modified for pico-guitar-midi-tracker by Cooper Dalrymple
* tracker.hpp
*
* This is free software, placed under the terms of the
* GNU General Public License, as published by the Free Software Foundation.
* Please see the file COPYING for details.
*/

#pragma once

#include "pico/stdlib.h"
#include "midi.hpp"

class Tracker {

public:

    Tracker(Midi *mid);
    ~Tracker();

    float *efxoutl;
    float *efxoutr;
    signed short int *schmittBuffer;
    signed short int *schmittPointer;
    const char **notes;
    int note;
    float nfreq, afreq, freq;
    float TrigVal;
    int cents;
    void schmittChar(int nframes, uint8_t *indata);
    void panic();
    void setTriggerAdjust(int val);
    void setVelAdjust(int val);

    int lanota;
    int nota_actual;
    int hay;
    int preparada;
    int ponla;
    int velocity;
    int Moctave;

    float VelVal;

    // Constants
    float fSAMPLE_RATE;
    float aFreq;
    float freqs[12];
    float lfreqs[12];

private:

    void updateFreqs(float val);
    void displayFrequency(float freq);
    void schmittInit(int size);
    void schmittS16LE(int nframes, int16_t *indata);
    void schmittFree();
    void sendNoteOn(int note);
    void sendNoteOff(int note);

    int blockSize;

    Midi *midi;
    uint led_slice;
    void setLedValue(uint8_t value);

};
