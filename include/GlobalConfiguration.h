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
            18, // Up
            19, // Down
            20, // Left
            21, // Right
        },
        {
            // Buttons
            6, // North
            9, // East
            8, // South
            7, // West

            0, // L1
            1, // L2
            2, // L3

            3, // R1
            4, // R2
            5, // R3

            26, // Start
            27, // Select
            22, // Home
        },
    },
    false, // Mirror face buttons to DPad
    3,     // Debounce delay in milliseconds
};

const Peripherals::ButtonLeds::Config button_leds_config = {
    // Pin config
    {
        10, // North
        13, // East
        12, // South
        11, // West
    },
    false, // Invert
};

const Peripherals::TouchSlider::Config touch_slider_config = {
    16,     // SDA Pin
    17,     // SCL Pin
    i2c0,   // I2C Block
    800000, // I2C Speed

    // Touch controller config, either Mpr121 or Cap1188
    //
    // Peripherals::TouchSlider::Mpr121Config{
    //     {0x5A, 0x5D, 0x5C}, // MPR121 Addresses
    //     12,                 // Touch threshold
    //     6,                  // Release threshold
    // },

    Peripherals::TouchSlider::Cap1188Config{
        {0x2C, 0x2B, 0x2A, 0x29},  // CAP1188 Addresses
        64,                        // Touch threshold
        Cap1188::Sensitivity::S32, // Sensitivity
    },
};

const Peripherals::TouchSliderLeds::Config touch_slider_leds_config = {
    28,    // LED Pin
    false, // Is RGBW strip
    true,  // Reverse LED order
    2,     // LEDs per segment

    128,                                                            // Brightness
    128,                                                            // Animation speed
    Peripherals::TouchSliderLeds::Config::IdleMode::Pulse,          // Idle Mode
    Peripherals::TouchSliderLeds::Config::TouchedMode::TouchedFade, // Touched Mode
    {64, 64, 64},                                                   // Background color
    {138, 254, 171},                                                // Touched color
    true,                                                           // Enable Player Color as Idle Color (DS4 only)
    true,                                                           // Enable LED control from PDLoader (PDLoader only)
};

const Peripherals::Display::Config display_config = {
    14,      // SDA Pin
    15,      // SCL Pin
    i2c1,    // I2C Block
    1000000, // I2C Speed
    0x3C,    // I2C Address
};

} // namespace Divacon::Config::Default