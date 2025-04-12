cmake_minimum_required(VERSION 3.13)

if((NOT DEFINED ENV{PICO_SDK_PATH}) AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_FETCH_FROM_GIT_TAG "2.0.0")
    set(PICO_SDK_FETCH_FROM_GIT ON)
endif()

if((NOT DEFINED ENV{PICO_BOARD}) AND (NOT PICO_BOARD))
    set(PICO_BOARD "pico")
endif()

include(pico_sdk_import.cmake)

project(DivaCon2040 VERSION 0.39.0)

pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(libs)

file(
  GLOB
  ${PROJECT_NAME}_SOURCES
  src/*.cpp
  src/usb/*.c
  src/usb/device/*.c
  src/usb/device/hid/*.c
  src/usb/device/vendor/*.c
  src/utils/*.cpp
  src/peripherals/*.cpp)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC tinyusb_device
         tinyusb_board
         pico_stdlib
         pico_multicore
         pico_rand
         pico_mbedtls
         mpr121
         cap1188
         is31se5117a
         pico_ssd1306
         pio_ws2812)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})
