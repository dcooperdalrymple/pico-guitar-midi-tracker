/**
 * pico-guitar-midi-tracker
 * @author Cooper Dalrymple
 * @version 0.1
 * @since 0.1
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "config.h"
#include "global.h"

// LED PWM
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// ADC Input
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"

// Tracker
#include "midi.hpp"
#include "tracker.hpp"

const char* const program_description = "Pico Guitar Midi Tracker v0.1";

// ADC Capture Core

Midi *midi;
Tracker *tracker;

uint dma_channel;
uint8_t adc_buffer[BUFFER_COUNT][BUFFER_SIZE];
volatile uint8_t adc_buffer_read_index = 0;
volatile uint8_t adc_buffer_write_index = 1;

volatile uint8_t dc_offset = 127;
volatile uint8_t ac_level = 0;

void core1_main();

void dma_handler() {
    // clear interrupt request
    dma_hw->ints0 = 1u << dma_channel;

    // increment buffer indexes
    adc_buffer_read_index = (adc_buffer_read_index + 1) % BUFFER_COUNT;
    adc_buffer_write_index = (adc_buffer_write_index + 1) % BUFFER_COUNT;

    // start next transfer
    dma_channel_set_write_addr(dma_channel, adc_buffer[adc_buffer_write_index], true);
}

int main() {
    stdio_init_all();

    // picotool declarations
    bi_decl(bi_program_description(program_description));
    bi_decl(bi_1pin_with_name(26 + ADC_CHANNEL, "ADC input pin"));
    bi_decl(bi_1pin_with_name(LED_LVL_PIN, "LED output pin"));
    bi_decl(bi_1pin_with_name(LED_NOTE_PIN, "LED output pin"));

    // start up led core
    multicore_launch_core1(core1_main);

    // setup midi
    midi = new Midi();
    tracker = new Tracker(midi);

    // setup adc
    adc_init();
    adc_gpio_init(26 + ADC_CHANNEL);
    adc_select_input(ADC_CHANNEL);

    adc_fifo_setup(
        true,   // write each conversion to the sample FIFO
        true,   // enable DMA data request (DREQ)
        1,      // DREQ (and IRQ) asserted when at least 1 sample present
        false,  // disable ERR bit because of 8 bit reads
        true    // Shift each sample to 8 bits when pushing to FIFO
    );

    clocks_init();
    uint32_t clkdiv = clock_get_hz(clk_peri) / SAMPLE_RATE;
    if (clkdiv < 96) clkdiv = 0;
    adc_set_clkdiv(clkdiv); // full speed

    // setup dma
    dma_channel = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, DREQ_ADC); // pace transfers based on availability of adc samples

    dma_channel_configure(dma_channel, &dma_config,
        adc_buffer[adc_buffer_write_index], // destination
        &adc_hw->fifo,                      // source
        BUFFER_SIZE,                        // transfer count
        false                               // don't start, wait for dma_handler to be called
    );

    // start capture
    adc_run(true);

    // Set up DMA irq
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Call the handler once to trigger the first transfer
    dma_handler();

    uint i;
    int16_t val;
    while (1) {
        dma_channel_wait_for_finish_blocking(dma_channel);
        tracker->schmittChar(BUFFER_SIZE, adc_buffer[adc_buffer_read_index]);
    }

    adc_run(false);
    adc_fifo_drain();

    return 0;
}

// LED Core

uint led_lvl_slice;
void led_set(uint8_t value) {
    pwm_set_gpio_level(LED_LVL_PIN, (uint16_t)value * value); // square off to appear more linear
}

void core1_main() {
    // configure pwm clock divider
    pwm_config led_config = pwm_get_default_config();
    pwm_config_set_clkdiv(&led_config, 4.f);

    // enable led as pwm
    gpio_set_function(LED_LVL_PIN, GPIO_FUNC_PWM);
    led_lvl_slice = pwm_gpio_to_slice_num(LED_LVL_PIN);
    pwm_init(led_lvl_slice, &led_config, true);

    // set initial value
    led_set(0);

    absolute_time_t now_timestamp, update_timestamp;
    update_timestamp = get_absolute_time();

    uint i;
    uint8_t max, min;
    while (1) {
        dma_channel_wait_for_finish_blocking(dma_channel);

        max = 0;
        min = 255;
        for (i = 0; i < BUFFER_SIZE; i++) {
            if (adc_buffer[adc_buffer_read_index][i] > max) max = adc_buffer[adc_buffer_read_index][i];
            if (adc_buffer[adc_buffer_read_index][i] < min) min = adc_buffer[adc_buffer_read_index][i];
        }

        ac_level = max - min;
        dc_offset = ac_level / 2 + min;

        led_set(ac_level);
    }
}
