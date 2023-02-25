#ifndef _PERIPHERALS_DISPLAY_H_
#define _PERIPHERALS_DISPLAY_H_

#include "usb/usb_driver.h"
#include "utils/Menu.h"

#include <ssd1306/ssd1306.h>

#include "hardware/i2c.h"

#include <memory>
#include <stdint.h>

namespace Divacon::Peripherals {

class Display {
  public:
    struct Config {
        uint8_t sda_pin;
        uint8_t scl_pin;
        i2c_inst_t *i2c_block;
        uint i2c_speed_hz;
        uint8_t i2c_address;
    };

  private:
    enum class State {
        Idle,
        Menu,
    };

    Config m_config;
    State m_state;

    uint32_t m_touched;
    usb_mode_t m_usb_mode;
    uint8_t m_player;
    Utils::Menu::State m_menu_state;

    ssd1306_t m_display;

    void drawIdleScreen();
    void drawMenuScreen();

  public:
    Display(const Config &config);

    void setTouched(uint32_t touched);
    void setUsbMode(usb_mode_t mode);
    void setPlayerId(uint8_t player);
    void setMenuState(const Utils::Menu::State &menu_state);

    void showIdle();
    void showMenu();

    void update();
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_DISPLAY_H_