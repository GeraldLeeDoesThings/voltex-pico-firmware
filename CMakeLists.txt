cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

include_directories(src)

option(DEBUG_MODE "Free up the usb for printing" OFF)

if (DEBUG_MODE MATCHES ON)
    message(STATUS "Debug mode is enabled")
    add_executable(main
        src/main.cpp
        src/button.cpp
        src/event.cpp
        src/joystick.cpp
        src/rotary_encoder.cpp
    )
    target_link_libraries(main PRIVATE pico_stdlib)
    pico_enable_stdio_usb(main 1)
    add_definitions(-DDEBUG_MODE)
else()
    message(STATUS "Release mode is enabled")
    add_executable(main
        src/main.cpp
        src/button.cpp
        src/event.cpp
        src/joystick.cpp
        src/rotary_encoder.cpp
        src/usb_descriptors.c
    )
    target_include_directories(main PRIVATE include/)
    target_link_libraries(main PRIVATE pico_stdlib tinyusb_device tinyusb_board hardware_pwm)
    pico_enable_stdio_usb(main 0)
endif()

# pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)

add_compile_options(
    -Wall -Werror -Wno-error=maybe-uninitialized -O3
)
