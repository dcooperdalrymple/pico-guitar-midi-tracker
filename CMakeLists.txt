cmake_minimum_required(VERSION 3.13)

set(NAME pico-guitar-midi-tracker)
set(URL "https://github.com/dcooperdalrymple/pico-guitar-midi-tracker")

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
project(${NAME} C CXX ASM)

add_executable(${NAME}
    src/midi.cpp
    src/tracker.cpp
    src/main.cpp
)

target_link_libraries(${NAME}
    pico_stdlib
    pico_float
    hardware_gpio
    hardware_pwm
    hardware_irq
    hardware_adc
    hardware_clocks
    hardware_dma
    pico_multicore
)

pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

pico_add_extra_outputs(${NAME})

pico_set_program_url(${NAME} ${URL})
