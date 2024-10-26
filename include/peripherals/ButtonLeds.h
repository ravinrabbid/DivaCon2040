#ifndef _PERIPHERALS_BUTTONLEDS_H_
#define _PERIPHERALS_BUTTONLEDS_H_

#include "usb/device_driver.h"
#include "utils/InputState.h"

namespace Divacon::Peripherals {

class ButtonLeds {
  public:
    struct Config {
        struct {
            uint8_t north;
            uint8_t east;
            uint8_t south;
            uint8_t west;
        } pins;

        bool invert;
    };

  private:
    Config m_config;

    Utils::InputState::Buttons m_buttons;
    bool m_raw_mode;

  public:
    ButtonLeds(const Config &config);

    void setButtons(const Utils::InputState::Buttons &buttons);

    void update();

    void update(const usb_button_led_t &raw);
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_BUTTONLEDS_H_