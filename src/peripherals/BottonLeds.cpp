#include "peripherals/ButtonLeds.h"

#include "hardware/gpio.h"

namespace Divacon::Peripherals {

ButtonLeds::ButtonLeds(const Config &config) : m_config(config), m_buttons({}), m_raw_mode(false) {
    uint button_mask = 0                          //
                       | 1 << m_config.pins.north //
                       | 1 << m_config.pins.east  //
                       | 1 << m_config.pins.south //
                       | 1 << m_config.pins.west;

    gpio_init_mask(button_mask);
    gpio_set_dir_out_masked(button_mask);
}

void ButtonLeds::setButtons(const Utils::InputState::Buttons &buttons) { m_buttons = buttons; }

void ButtonLeds::update() {
    if (m_raw_mode) {
        return;
    }

    gpio_put(m_config.pins.north, !(m_config.invert ^ m_buttons.north));
    gpio_put(m_config.pins.east, !(m_config.invert ^ m_buttons.east));
    gpio_put(m_config.pins.south, !(m_config.invert ^ m_buttons.south));
    gpio_put(m_config.pins.west, !(m_config.invert ^ m_buttons.west));
}

void ButtonLeds::update(const usb_button_led_t &raw) {
    m_raw_mode = true;

    gpio_put(m_config.pins.north, m_config.invert ^ raw.north);
    gpio_put(m_config.pins.east, m_config.invert ^ raw.east);
    gpio_put(m_config.pins.south, m_config.invert ^ raw.south);
    gpio_put(m_config.pins.west, m_config.invert ^ raw.west);
}

} // namespace Divacon::Peripherals
