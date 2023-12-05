/**
 * pico-guitar-midi-tracker_midi
 * @author Cooper Dalrymple
 * @version 0.1
 * @since 0.1
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../config.h"
#include "../global.h"

// LED PWM
#include "hardware/gpio.h"

// MIDI Output
#include "../midi.hpp"

const char* const program_description = "Pico Guitar Midi Tracker v0.1: MIDI Test";

Midi *midi;

void test_note(uint8_t note, uint8_t velocity = 100) {
    gpio_put(LED_NOTE_PIN, 1);
    midi->sendNoteOn(note, velocity);
    sleep_ms(1000);

    gpio_put(LED_NOTE_PIN, 0);
    midi->sendNoteOff(note);
    sleep_ms(1000);
}

int main() {
    stdio_init_all();

    // picotool declarations
    bi_decl(bi_program_description(program_description));
    bi_decl(bi_1pin_with_name(LED_NOTE_PIN, "LED output pin"));

    gpio_init(LED_NOTE_PIN);
    gpio_set_dir(LED_NOTE_PIN, GPIO_OUT);

    // setup midi
    midi = new Midi();

    uint i;
    while (1) {
        for (i = 64; i < 88; i++) {
            test_note(i);
        }
    }

    return 0;
}
