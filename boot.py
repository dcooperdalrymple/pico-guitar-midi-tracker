# SPDX-FileCopyrightText: Copyright (c) 2024 Cooper Dalrymple
#
# SPDX-License-Identifier: MIT

import supervisor
import usb_midi

supervisor.set_usb_identification(
    manufacturer="relic",
    product="pico-guitar-tracker",
)

# only for CircuitPython 9.1+
usb_midi.set_names(
    streaming_interface_name="pico-guitar-tracker",
    in_jack_name="midi in",
    out_jack_name="midi out",
)
