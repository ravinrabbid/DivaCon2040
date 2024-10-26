#include "peripherals/ButtonLeds.h"
#include "peripherals/Buttons.h"
#include "peripherals/Display.h"
#include "peripherals/TouchSlider.h"
#include "peripherals/TouchSliderLeds.h"
#include "usb/device_driver.h"

#include "hardware/i2c.h"

namespace Divacon::Config::Default {

const usb_mode_t usb_mode = USB_MODE_SWITCH_DIVACON;

const Peripherals::Buttons::Config buttons_config = {
    {
        // Pin config
        {
            // Dpad
            21, // Up
            20, // Down
            19, // Left
            18, // Right
        },
        {
            // Buttons
            0, // North
            3, // East
            2, // South
            1, // West

            6, // L1
            7, // L2
            8, // L3

            13, // R1
            14, // R2
            15, // R3

            11, // Start
            9,  // Select
            10, // Home
        },
    },
    3, // Debounce delay in milliseconds
};

const Peripherals::ButtonLeds::Config button_leds_config = {
    {
        22, // North
        5,  // East
        4,  // South
        12, // West
    },
    false, // Invert
};

const Peripherals::TouchSlider::Config touch_slider_config = {
    Peripherals::TouchSlider::Config::Mode::ARCADE, // Stick emulation mode

    12, // Touch threshold
    6,  // Release threshold

    16,                 // SDA Pin
    17,                 // SCL Pin
    i2c0,               // I2C Block
    800000,             // I2C Speed
    {0x5A, 0x5D, 0x5C}, // MPR121 Addresses
};

const Peripherals::TouchSliderLeds::Config touch_slider_leds_config = {
    28,              // LED Pin
    false,           // Is RGBW strip
    true,            // Reverse LED order
    2,               // LEDs per segment
    128,             // Brightness
    96,              // Fade speed
    {235, 85, 120},  // Background color
    {140, 255, 220}, // Touched color
};

const Peripherals::Display::Config display_config = {
    26,      // SDA Pin
    27,      // SCL Pin
    i2c1,    // I2C Block
    1000000, // I2C Speed
    0x3C,    // I2C Address
};

} // namespace Divacon::Config::Default