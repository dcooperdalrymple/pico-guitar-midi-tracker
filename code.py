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
import ulab.utils

# MIDI modules
import busio
import adafruit_midi
from adafruit_midi.note_off import NoteOff
from adafruit_midi.note_on import NoteOn

# Constants
SAMPLE_RATE = 192000
BUFFER_SIZE = 2048
MIDI_CHANNEL = 0
LEVEL_ATTACK = 0.1 # Amount of difference in level to trigger a note
LEVEL_RELEASE = 0.05 # Minimum level to sustain note

LOG2_A4 = math.log(440, 2)

# Pin assignments
led_level_pin, led_midi_pin = board.LED, board.GP2
adc_pin = board.GP26
uart_tx, uart_rx = board.GP4, board.GP5

# ADC Setup
led_level = pwmio.PWMOut(led_level_pin)

raw_buffer = array.array("H", [0x0000] * BUFFER_SIZE)
adc = analogbufio.BufferedIn(adc_pin, sample_rate=SAMPLE_RATE)

def get_buffer() -> np.ndarray:
    adc.readinto(raw_buffer)
    return np.array(raw_buffer, dtype=np.uint16)

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

# Calibrate ADC
mid = np.max(get_buffer())
level_max = min(mid, 2 ** 16 - mid)

level = 0.0
note = 0
while True:

    # Read data from ADC
    buffer = get_buffer()
    
    # Calculate max level
    current_level = min(max(abs((np.max(buffer) - mid) / level_max), 0.0), 1.0)
    led_level.duty_cycle = int(current_level * (2 ** 16 - 1))

    # Note no longer detected
    if note and current_level < LEVEL_RELEASE:
        midi.send(NoteOff(note, 0))
        led_midi.value = False
        level = current_level
        note = 0
        print("Release")
        continue

    # No note detected and level too low
    if current_level - level < LEVEL_ATTACK:
        continue
    
    # Convert np.uint16 to np.int16
    mean = int(np.mean(buffer))
    buffer = np.array([x - mean for x in buffer], dtype=np.int16)

    # Use the Fast Fourier Transform to determine the peak frequency of the signal
    fft_data = ulab.utils.spectrogram(buffer)
    fft_data = fft_data[1 : (len(fft_data) // 2) - 1]
    #fft_data = np.log(fft_data)
    current_freq = np.argmax(fft_data) / BUFFER_SIZE * SAMPLE_RATE / 4
    if not current_freq:
        continue
    
    # Determine MIDI note value
    current_note = round(12 * (math.log(current_freq, 2) - LOG2_A4) + 69)

    # Release previous note
    if note and current_note != note:
        midi.send(NoteOff(note, 0))

    # Press new note
    midi.send(NoteOn(current_note, int(current_level * 127)))
    led_midi.value = True
    note = current_note
    level = current_level
    print("Press {:d}".format(current_note))
