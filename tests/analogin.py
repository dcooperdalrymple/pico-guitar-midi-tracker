# SPDX-FileCopyrightText: Copyright (c) 2024 Cooper Dalrymple
#
# SPDX-License-Identifier: MIT

import board
import pwmio
import analogbufio
import array
import ulab.numpy as np

SAMPLE_RATE = 44100
BUFFER_SIZE = 4096

led = pwmio.PWMOut(board.LED)

raw_buffer = array.array("H", [0x0000] * BUFFER_SIZE)
buffer = np.array(raw_buffer, dtype=np.uint16)

adc = analogbufio.BufferedIn(board.GP26, sample_rate=SAMPLE_RATE)

while True:
    adc.readinto(raw_buffer)
    level = abs((np.max(buffer) - 2 ** 15) / (2 ** 15))
    led.duty_cycle = level * (2 ** 16 - 1)
    print(level)
