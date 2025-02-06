cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

include_directories(src)

add_executable(main
    src/main.cpp
    src/event.cpp
    src/rotary_encoder.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)

add_compile_options(
    -Wall -Werror -Wno-error=maybe-uninitialized -O3
)
