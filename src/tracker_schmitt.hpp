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
#include "tracker.hpp"

class TrackerSchmitt : public Tracker {

public:
    TrackerSchmitt(Midi *mid);
    ~TrackerSchmitt();

    void process(size_t len, uint8_t *data);

private:

    void schmittInit(size_t size);
    size_t blockSize;
    int16_t *schmittBuffer;
    int16_t *schmittPointer;

};
