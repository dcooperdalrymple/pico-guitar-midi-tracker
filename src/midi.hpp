#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define BAUD_RATE           31250
#define DATA_BITS           8
#define STOP_BITS           1
#define PARITY              UART_PARITY_NONE

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
