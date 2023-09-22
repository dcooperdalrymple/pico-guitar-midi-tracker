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

#include "tracker.hpp"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/float.h"
#include "config.h"
#include "global.h"

// LED
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "midi.hpp"

Tracker::Tracker(Midi *mid) {
    midi = mid;

    // Setup led to indicate note status
    pwm_config led_config = pwm_get_default_config();
    pwm_config_set_clkdiv(&led_config, 4.f);

    // enable led as pwm
    gpio_set_function(LED_NOTE_PIN, GPIO_FUNC_PWM);
    led_slice = pwm_gpio_to_slice_num(LED_NOTE_PIN);
    pwm_init(led_slice, &led_config, true);

    // set initial value
    setLedValue(0);

    velocity = 100;
    lanota = -1;
    preparada = 0;
    nota_actual = -1;
    TrigVal = .25f;
    hay = 0;
    ponla = 0;
    Moctave = 0;

    schmittBuffer = NULL;
    schmittPointer = NULL;
    static const char *englishNotes[12] =
    { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
    notes = englishNotes;
    note = 0;
    nfreq = 0;
    afreq = 0;

    fSAMPLE_RATE = (float)SAMPLE_RATE;
    updateFreqs(440.0f);

    setTriggerAdjust(1);
    setVelAdjust(1);

    schmittInit(32);
};

Tracker::~Tracker() {
    schmittFree();
};

void Tracker::updateFreqs(float val) {
    int i;
    aFreq = val;
    freqs[0] = aFreq;
    lfreqs[0] = logf(freqs[0]);
    for (i = 1; i < 12; i++) {
        freqs[i] = freqs[i - 1] * D_NOTE;
        lfreqs[i] = lfreqs[i - 1] + LOG_D_NOTE;
    }
};

void Tracker::displayFrequency(float ffreq) {
    int i;
    int noteoff = 0;
    int octave = 4;

    float ldf, mldf;
    float lfreq;

    if (ffreq < 1E-15)
        ffreq = 1E-15f;
    lfreq = logf (ffreq);
    while (lfreq < lfreqs[0] - LOG_D_NOTE * .5f)
        lfreq += LOG_2;
    while (lfreq >= lfreqs[0] + LOG_2 - LOG_D_NOTE * .5f)
        lfreq -= LOG_2;
    mldf = LOG_D_NOTE;
    for (i = 0; i < 12; i++) {
        ldf = fabsf (lfreq - lfreqs[i]);
        if (ldf < mldf) {
            mldf = ldf;
            note = i;
        }
    }
    nfreq = freqs[note];
    while (nfreq / ffreq > D_NOTE_SQRT) {
        nfreq *= .5f;
        octave--;
        if (octave < -2) {
            noteoff = 1;
            break;
        }

    }
    while (ffreq / nfreq > D_NOTE_SQRT) {
        nfreq *= 2.0f;
        octave++;
        if (octave > 9) {
            noteoff = 1;
            break;
        }
    }

    cents = lrintf(1200.0f * (logf(ffreq / nfreq) / LOG_2));
    lanota = 24 + (octave * 12) + note - 3;

    if ((noteoff) & (hay)) {
        sendNoteOff(nota_actual);
        hay = 0;
        nota_actual = -1;
    }

    if ((preparada == lanota) && (lanota != nota_actual)) {
        hay = 1;
        if (nota_actual != -1) {
            sendNoteOff(nota_actual);
        }

        sendNoteOn(lanota);
        nota_actual = lanota;
    }


    if ((lanota > 0 && lanota < 128) && (lanota != nota_actual))
        preparada = lanota;

};

void Tracker::schmittInit(int size) {
    blockSize = SAMPLE_RATE / size;
    schmittBuffer = (signed short int *) malloc(blockSize * sizeof (signed short int));
    schmittPointer = schmittBuffer;
};

void Tracker::schmittS16LE(int nframes, int16_t *indata) {
    int i, j;
    float trigfact = 0.6f;

    for (i = 0; i < nframes; i++) {
        *schmittPointer++ = indata[i];
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
                afreq = fSAMPLE_RATE *((float)tc / (float) (endpoint - startpoint));
                displayFrequency(afreq);
            }
        }
    }
};

void Tracker::schmittFree() {
    free(schmittBuffer);
};

void Tracker::schmittChar(int nframes, uint8_t *indata) {
    int16_t buf[nframes];
    for (int i = 0; i < nframes; i++) {
        buf[i] = ((int16_t)indata[i] - dc_offset) * 256;
    }
    schmittS16LE(nframes, buf);
};

void Tracker::sendNoteOn(int nota) {
    int anota = nota + (Moctave * 12);
    if((anota<0) || (anota>127)) return;

    velocity = lrintf((float)ac_level / 256.0f * VelVal);
    if (velocity > 127)
        velocity = 127;
    if (velocity < 1)
        velocity = 1;

    setLedValue(velocity * 2);
    midi->sendNoteOn(anota, velocity);
};

void Tracker::sendNoteOff(int nota) {
    int anota = nota + (Moctave * 12);
    if ((anota<0) || (anota>127)) return;
    setLedValue(0);
    midi->sendNoteOff(anota);
};

void Tracker::panic() {
    hay = 0;
    nota_actual = -1;
    setLedValue(0);
    midi->panic();
};

void Tracker::setTriggerAdjust(int val) {
    TrigVal = 1.0f / (float)val;
};

void Tracker::setVelAdjust(int val) {
    VelVal = 100.0f / (float)val;
};

void Tracker::setLedValue(uint8_t value) {
    pwm_set_gpio_level(LED_NOTE_PIN, (uint16_t)value * value);
};
