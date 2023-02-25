#include "peripherals/Buttons.h"
#include "peripherals/Display.h"
#include "peripherals/TouchSlider.h"
#include "peripherals/TouchSliderLeds.h"
#include "usb/usb_driver.h"

#include "hardware/i2c.h"

namespace Divacon {

const usb_mode_t default_usb_mode = USB_MODE_DIRECTINPUT;

const Divacon::Peripherals::Buttons::Config default_buttons_config = {
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
            1, // East
            3, // South
            2, // West

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

const Divacon::Peripherals::TouchSlider::Config default_touch_slider_config = {
    Divacon::Peripherals::TouchSlider::Config::Mode::ARCADE, // Stick emulation mode

    12, // Touch threshold
    6,  // Release threshold

    16,                 // SDA Pin
    17,                 // SCL Pin
    i2c0,               // I2C Block
    800000,             // I2C Speed
    {0x5A, 0x5D, 0x5C}, // MPR121 Addresses
};

const Divacon::Peripherals::TouchSliderLeds::Config default_touch_slider_leds_config = {
    28,             // LED Pin
    false,          // Is RGBW strip
    true,           // Reverse LED order
    2,              // LEDs per segment
    150,            // Maximum brightness
    {255, 60, 255}, // Background color
    {60, 230, 255}, // Touched color
};

const Divacon::Peripherals::Display::Config default_display_config = {
    26,      // SDA Pin
    27,      // SCL Pin
    i2c1,    // I2C Block
    1000000, // I2C Speed
    0x3C,    // I2C Address
};

} // namespace Divacon