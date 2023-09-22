#include "midi.hpp"

#include "pico/stdlib.h"
#include "config.h"

Midi::Midi() {

    // Set UART speed
    uart_init(UART_ID, BAUD_RATE);

    // Set UART pins
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Enable Tx and Rx FIFOs on UART
    uart_set_fifo_enabled(UART_ID, true);

    // Disable cr/lf conversion on Tx
    uart_set_translate_crlf(UART_ID, false);

    setChannel(MIDI_CHANNEL);

};

Midi::~Midi() {

};

void Midi::setChannel(uint8_t chan) {
    channel = chan & 0x0f;
};
uint8_t Midi::getChannel() {
    return channel;
};

void Midi::sendNoteOn(uint8_t note, uint8_t velocity) {
    while (!uart_is_writable(UART_ID)) { } // blocking
    uart_putc_raw(UART_ID, 0x90 + channel & 0x0f); // status
    uart_putc_raw(UART_ID, note & 0x7f);
    uart_putc_raw(UART_ID, velocity & 0x7f);
};

void Midi::sendNoteOff(uint8_t note) {
    while (!uart_is_writable(UART_ID)) { } // blocking
    uart_putc_raw(UART_ID, 0x80 + channel & 0x0f); // status
    uart_putc_raw(UART_ID, note & 0x7f);
    uart_putc_raw(UART_ID, 0x00); // velocity
};

void Midi::panic() {
    for (uint8_t i = 0; i < 128; i++) {
        sendNoteOff(i);
    }
};
