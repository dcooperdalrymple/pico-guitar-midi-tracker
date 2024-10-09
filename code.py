# SPDX-FileCopyrightText: Copyright (c) 2024 Cooper Dalrymple
#
# SPDX-License-Identifier: MIT

import board
import digitalio
import pwmio
import math

# ADC modules
import analogbufio
import array
import ulab.numpy as np

# MIDI modules
import busio
import adafruit_midi
from adafruit_midi.note_off import NoteOff
from adafruit_midi.note_on import NoteOn

# Constants
SAMPLE_RATE = 44100
BUFFER_SIZE = 4096
MIDI_CHANNEL = 0
LEVEL_ATTACK = 0.5
LEVEL_RELEASE = 0.2

LOG2_A4 = math.log2(440)

# Pin assignments
led_level_pin, led_midi_pin = board.LED, board.GP2
adc_pin = board.GP26
uart_tx, uart_rx = board.GP4, board.GP5

# ADC Setup
led_level = pwmio.PWMOut(led_level_pin)

raw_buffer = array.array("H", [0x0000] * BUFFER_SIZE)
buffer = np.array(raw_buffer, dtype=np.uint16)

adc = analogbufio.BufferedIn(adc_pin, sample_rate=SAMPLE_RATE)

# MIDI Setup
led_midi = digitalio.DigitalInOut(led_midi_pin)
led_midi.direction = digitalio.Direction.OUTPUT

uart = busio.UART(
    tx=uart_tx,
    rx=uart_rx,
    baudrate=31250,
    timeout=0.001,
)
midi = adafruit_midi.MIDI(
    midi_out=uart,
    out_channel=MIDI_CHANNEL,
)

note = 0
while True:

    # Read data from ADC
    adc.readinto(raw_buffer)
    
    # Calculate max level
    level = abs((np.max(buffer) - 2 ** 15) / (2 ** 15))
    led_level.duty_cycle = level * (2 ** 16 - 1)

    # Note no longer detected
    if note and level < LEVEL_RELEASE:
        midi.send(NoteOff(note, 0))
        led_midi.value = False
        note = 0
        continue

    # No note detected and level too low
    if not note and level < LEVEL_ATTACK:
        continue

    # Use the Fast Fourier Transform to determine the peak frequency of the signal
    fft_data = ulab.utils.spectrogram(buffer)
    # fft = fft[1 : (len(fft) // 2) - 1]
    current_freq = np.argmax(fft) / BUFFER_SIZE * SAMPLE_RATE / 4

    # Determine MIDI note value
    current_note = round(12 * (math.log2(freq) - LOG2_A4) + 69)

    # TODO: If same as current note but different frequencies, pitch shift?

    # Release previous note
    if note and current_note != note:
        midi.send(NoteOff(note, 0))

    # Press new note
    midi.send(NoteOn(current_note, level * 127))
    led_midi.value = True
