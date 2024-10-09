# SPDX-FileCopyrightText: Copyright (c) 2024 Cooper Dalrymple
#
# SPDX-License-Identifier: MIT

import board
import analogbufio
import array
import ulab.numpy as np
import math

try:
    from ulab.utils import spectrogram
except ImportError:
    from ulab.scipy.signal import spectrogram

SAMPLE_RATE = 44100
BUFFER_SIZE = 2048

raw_buffer = array.array("H", [0x0000] * BUFFER_SIZE)
adc = analogbufio.BufferedIn(board.GP26, sample_rate=SAMPLE_RATE)

def read() -> np.ndarray:
    adc.readinto(raw_buffer)
    return np.array(raw_buffer, dtype=np.uint16)

def calculate_level(buffer:np.ndarray) -> float:
    mean = int(np.mean(buffer))
    mean_max = min(mean, 2 ** 16 - mean)
    return min(max(abs((np.max(buffer) - mean) / mean_max), 0.0), 1.0)

def chromagram(data:np.ndarray, rate:int = SAMPLE_RATE):
    # Convert np.uint16 to np.int16
    if data.dtype == np.uint16:
        mean = int(np.mean(data))
        data = np.array([x - mean for x in data], dtype=np.int16)

    data = spectrogram(data)

    min_freq, max_freq = 65.41, 493.88 # C2 (36) -> B4 (71)
    data_min_freq, data_max_freq = min(rate / 2 / len(data), min_freq), max(rate / 2, max_freq)

    start = (min_freq - data_min_freq) / (data_max_freq - data_min_freq) * len(data)
    stop = (max_freq - data_min_freq) / (data_max_freq - data_min_freq) * len(data)

    data = data[math.floor(start):math.ceil(stop)]

    chroma_freq = np.interp(
        np.arange(start, stop, (stop - start) / 32, dtype=np.float),
        np.arange(0, len(data), 1, dtype=np.uint16),
        data
    )

    chroma = np.zeros(12)
    for i in range(12):
        chroma[i] = np.sum(chroma_freq[i::12])

    return np.argmax(chroma), np.argmax(chroma_freq) + 36

LOG2_A4 = math.log(440, 2)
def hz_to_midi(freq:float) -> int|None:
    if not freq:
        return None
    return round(12 * (math.log(freq, 2) - LOG2_A4) + 69)

note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

while True:
    # Read data from ADC
    buffer = read()

    # Calculate signal properties
    level = calculate_level(buffer)
    note_index, notenum = chromagram(buffer)

    print("Level: {:>4.2f}\tNote Name: {:>2s}\tNote Value: {:>3d}".format(level, note_names[note_index], notenum))
