# Raspberry Pi Pico Guitar Midi Tracker
Convert audio input from a guitar into monophonic midi output using Raspberry Pi Pico RP2040.

This project uses a modified version of the midi conversion script from [Rakarrack](https://github.com/dtimms/rakarrack).

## Compiling Firmware

* Configuring: `cmake -B build -S .`
* Compiling/Building: `make -C build`
* Writing: Hold BOOTSEL button on Pico, plug it in via USB, and release BOOTSEL. Copy and paste `build/pico-guitar-midi-tracker.uf2` into RPI-RP2 drive.
