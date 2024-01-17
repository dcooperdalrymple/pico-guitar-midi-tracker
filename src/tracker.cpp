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

    notenum = -1;
    noteFreq = 0;
    detectFreq = 0;
    rootFreq = 0;

    fSAMPLE_RATE = (float)SAMPLE_RATE;
    calculateNoteFrequencies(ROOT_FREQ);

    setTriggerLevel(64);
    setVelocityAmount(0);
    setOctave(0);
};

void Tracker::setTriggerLevel(uint8_t val) {
    triggerLevel = val;
};

void Tracker::setVelocityAmount(uint8_t val) {
    velocityAmount = (float)val;
};

void Tracker::setOctave(int16_t oct) {
    octave = oct;
};

void Tracker::panic() {
    notenum = -1;
    setLedValue(0);
    midi->panic();
};

void Tracker::detectNote(float freq) {
    // TODO: Gradually filter detected note to prevent sudden changes
    // TODO: Allow multiple note detection

    uint8_t i, note;
    int oct = ROOT_OCTAVE; // Detected note octave
    float temp, logFreq, noteDev, noteFreq; // logFreq = log of frequency, noteDev = deviation of note frequency from detected frequency (logarithmic)

    // Remove note if it exists
    removeNote();

    // Calculate log of frequency
    if (freq < 1e-15) freq = 1e-15f;
    logFreq = logf(freq);

    // Adjust log of frequency to base octave
    while (logFreq < logFreqs[0] - LOG_D_NOTE * .5f) {
        logFreq += LOG_2;
    }
    while (logFreq >= logFreqs[0] + LOG_2 - LOG_D_NOTE * .5f) {
        logFreq -= LOG_2;
    }

    // Find closest note frequency
    noteDev = LOG_D_NOTE;
    for (i = 0; i < 12; i++) {
        temp = fabsf(logFreq - logFreqs[i]);
        if (temp < noteDev) {
            noteDev = temp;
            note = i;
        }
    }
    noteFreq = noteFreqs[note];

    // Adjust note frequency back to original octave of detected frequency
    while (noteFreq / freq > D_NOTE_SQRT) {
        noteFreq *= .5f;
        oct--;
        if (oct < -2) return;
    }
    while (freq / noteFreq > D_NOTE_SQRT) {
        noteFreq *= 2.0f;
        oct++;
        if (oct > 9) return;
    }

    // Convert note from index to midi note number
    notenum = 24 + (oct * 12) + note - 3;

    sendNoteOn(notenum);
};

void Tracker::removeNote() {
    if (notenum < 0) return;
    sendNoteOff(notenum);
    notenum = -1;
};

void Tracker::sendNoteOn(int note) {
    int _note = note + (octave * 12);
    if (_note < 0 || _note > 127) return;

    uint8_t velocity = 127 - lrintf((1.0f - (float)ac_level / 256.0f) * velocityAmount);
    if (velocity > 127) velocity = 127;
    if (velocity < 1) velocity = 1;

    setLedValue(velocity * 2);
    midi->sendNoteOn((uint8_t)_note, velocity);
};

void Tracker::sendNoteOff(int note) {
    int _note = note + (octave * 12);
    if (_note < 0 || _note > 127) return;
    setLedValue(0);
    midi->sendNoteOff(_note);
};

void Tracker::setLedValue(uint8_t value) {
    pwm_set_gpio_level(LED_NOTE_PIN, (uint16_t)value * value);
};

void Tracker::calculateNoteFrequencies(float val) {
    int i;
    rootFreq = val;
    noteFreqs[0] = rootFreq;
    logFreqs[0] = logf(noteFreqs[0]);
    for (i = 1; i < 12; i++) {
        noteFreqs[i] = noteFreqs[i - 1] * D_NOTE;
        logFreqs[i] = logFreqs[i - 1] + LOG_D_NOTE;
    }
};
