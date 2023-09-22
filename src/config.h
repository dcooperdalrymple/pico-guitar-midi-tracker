#ifndef CONFIG_H_
#define CONFIG_H_

// ADC
#define ADC_CHANNEL         0 // GPIO26
#define SAMPLE_RATE         22050
#define BUFFER_SIZE         256
#define BUFFER_COUNT        2
#define LED_LVL_PIN         25

// MIDI
#define UART_ID             uart1
#define BAUD_RATE           31250
#define UART_TX_PIN         4
#define UART_RX_PIN         5
#define UART_BUFFER_SIZE    32
#define MIDI_CHANNEL        0

// Tracker
#define LED_NOTE_PIN        2

#endif
