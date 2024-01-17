/* tuneit.c -- Detect fundamental frequency of a sound
 * Copyright (C) 2004, 2005  Mario Lang <mlang@delysid.org>
 *
 * Modified for rakarrack by Josep Andreu
 * MIDIConverter.C  MIDI Converter class
 *
 * Modified for pico-guitar-midi-tracker by Cooper Dalrymple
 * tracker.cpp
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software Foundation.
 * Please see the file COPYING for details.
 */

#include "tracker_schmitt.hpp"

#include "pico/stdlib.h"
#include "pico/float.h"
#include "config.h"
#include "global.h"

TrackerSchmitt::TrackerSchmitt(Midi *mid) : Tracker(mid) {
    schmittInit(32);
};

TrackerSchmitt::~TrackerSchmitt() {
    free(schmittBuffer);
};

void TrackerSchmitt::schmittInit(size_t size) {
    blockSize = SAMPLE_RATE / size;
    schmittBuffer = (int16_t*)malloc(blockSize * sizeof(int16_t));
    schmittPointer = schmittBuffer;
};

void TrackerSchmitt::process(size_t len, uint8_t *data) {
    // Check buffer level against trigger level
    if (ac_level < triggerLevel) {
        removeNote();
        return;
    }

    // Convert to int16_t
    int16_t buffer[len];
    for (int i = 0; i < len; i++) {
        buffer[i] = ((int16_t)data[i] - dc_offset) * 256;
    }

    size_t i, j;
    float trigfact = 0.6f;
    for (i = 0; i < len; i++) {
        *schmittPointer++ = buffer[i];
        if (schmittPointer - schmittBuffer >= blockSize) {
            int endpoint, startpoint, t1, t2, A1, A2, tc, schmittTriggered;
            schmittPointer = schmittBuffer;
            for (j = 0, A1 = 0, A2 = 0; j < blockSize; j++) {
                if (schmittBuffer[j] > 0 && A1 < schmittBuffer[j])
                    A1 = schmittBuffer[j];
                if (schmittBuffer[j] < 0 && A2 < -schmittBuffer[j])
                    A2 = -schmittBuffer[j];
            }
            t1 = lrintf((float)A1 * trigfact + 0.5f);
            t2 = -lrintf((float)A2 * trigfact + 0.5f);
            startpoint = 0;
            for (j = 1; schmittBuffer[j] <= t1 && j < blockSize; j++);
            for (; !(schmittBuffer[j] >= t2 && schmittBuffer[j + 1] < t2) && j < blockSize; j++);
            startpoint = j;
            schmittTriggered = 0;
            endpoint = startpoint + 1;
            for (j = startpoint, tc = 0; j < blockSize; j++) {
                if (!schmittTriggered) {
                    schmittTriggered = (schmittBuffer[j] >= t1);
                } else if (schmittBuffer[j] >= t2 && schmittBuffer[j + 1] < t2) {
                    endpoint = j;
                    tc++;
                    schmittTriggered = 0;
                }
            }
            if (endpoint > startpoint) {
                detectNote(fSAMPLE_RATE *((float)tc / (float) (endpoint - startpoint)));
            }
        }
    }
};
