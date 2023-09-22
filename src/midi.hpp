#pragma once

#include "pico/stdlib.h"

class Midi {

public:

    Midi();
    ~Midi();

    void setChannel(uint8_t chan);
    uint8_t getChannel();

    void sendNoteOn(uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t note);

    void panic();

private:

    uint8_t channel;

};
