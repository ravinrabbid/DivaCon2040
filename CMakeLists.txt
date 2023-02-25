cmake_minimum_required(VERSION 3.13)

include(/usr/share/pico-sdk/pico_sdk_init.cmake)

project(DivaCon2040)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(libs)

file(GLOB divacon_SOURCES src/*.cpp src/usb/*.c src/utils/*.cpp
     src/peripherals/*.cpp)

add_executable(divacon ${divacon_SOURCES})

target_include_directories(divacon PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(
  divacon
  PUBLIC tinyusb_device
         tinyusb_board
         pico_stdlib
         pico_multicore
         mpr121
         pico_ssd1306
         pio_ws2812)

pico_enable_stdio_usb(divacon 1)
pico_enable_stdio_uart(divacon 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(divacon)
