# SPDX-FileCopyrightText: Copyright (c) 2024 Cooper Dalrymple
#
# SPDX-License-Identifier: MIT

import board
import pwmio
import analogbufio
import array
import ulab.numpy as np

SAMPLE_RATE = 44100
BUFFER_SIZE = 256

led = pwmio.PWMOut(board.LED)

raw_buffer = array.array("H", [0x0000] * BUFFER_SIZE)
adc = analogbufio.BufferedIn(board.GP26, sample_rate=SAMPLE_RATE)

def get_buffer() -> np.ndarray:
    adc.readinto(raw_buffer)
    return np.array(raw_buffer, dtype=np.uint16)

# Calibrate ADC
mid = np.max(get_buffer())
level_max = min(mid, 2 ** 16 - mid)

while True:
    level = min(max(abs((np.max(get_buffer()) - mid) / level_max), 0.0), 1.0)
    led.duty_cycle = int(level * (2 ** 16 - 1))
    print(level)
