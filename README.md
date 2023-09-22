# Raspberry Pi Pico EVSE
Convert audio input from a guitar into monophonic midi output using Raspberry Pi Pico RP2040.

## Compiling Firmware

* Configuring: `cmake -B build -S .`
* Compiling/Building: `make -C build`
* Writing: Hold BOOTSEL button on Pico, plug it in via USB, and release BOOTSEL. Copy and paste `rpi-pico-evse.uf2` into RPI-RP2 drive.
