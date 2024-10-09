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

def fftfreq(buffer:np.ndarray, rate:int = SAMPLE_RATE, clip:int = 8) -> float:
    # Convert np.uint16 to np.int16
    if buffer.dtype == np.uint16:
        mean = int(np.mean(buffer))
        buffer = np.array([x - mean for x in buffer], dtype=np.int16)
    
    data = spectrogram(buffer)
    data = np.log(data + 1e-7)
    data = data[1:(len(data) // 2) - 1]

    return np.argmax(data) / len(data) * rate / 4

LOG2_A4 = math.log(440, 2)
def hz_to_midi(freq:float) -> int|None:
    if not freq:
        return None
    return round(12 * (math.log(freq, 2) - LOG2_A4) + 69)

while True:
    # Read data from ADC
    buffer = read()

    # Calculate signal properties
    level = calculate_level(buffer)
    frequency = fftfreq(buffer)
    note = hz_to_midi(frequency)

    print("Level: {:>4.2f}\tFrequency: {:>7.1f}\tNote: {:>3d}".format(level, frequency, note or 0))
