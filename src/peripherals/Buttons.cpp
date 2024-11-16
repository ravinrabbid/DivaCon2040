#include "peripherals/Buttons.h"

#include "hardware/gpio.h"
#include "pico/time.h"

namespace Divacon::Peripherals {

Buttons::Button::Button(uint8_t pin) : gpio_pin(pin), gpio_mask(1 << pin), last_change(0), active(false) {}

void Buttons::Button::setState(bool state, uint8_t debounce_delay) {
    if (active == state) {
        return;
    }

    // Immediately change the input state, but only allow a change every debounce_delay milliseconds.
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (last_change + debounce_delay <= now) {
        active = state;
        last_change = now;
    }
}

void Buttons::socdClean(Utils::InputState &input_state) {

    // Last input has priority
    if (input_state.dpad.up && input_state.dpad.down) {
        if (m_socd_state.lastVertical == Id::DOWN) {
            input_state.dpad.down = false;
        } else if (m_socd_state.lastVertical == Id::UP) {
            input_state.dpad.up = false;
        }
    } else if (input_state.dpad.up) {
        m_socd_state.lastVertical = Id::UP;
    } else {
        m_socd_state.lastVertical = Id::DOWN;
    }

    if (input_state.dpad.left && input_state.dpad.right) {
        if (m_socd_state.lastHorizontal == Id::RIGHT) {
            input_state.dpad.right = false;
        } else if (m_socd_state.lastHorizontal == Id::LEFT) {
            input_state.dpad.left = false;
        }
    } else if (input_state.dpad.left) {
        m_socd_state.lastHorizontal = Id::LEFT;
    } else {
        m_socd_state.lastHorizontal = Id::RIGHT;
    }
}

Buttons::Buttons(const Config &config) : m_config(config), m_socd_state{Id::DOWN, Id::RIGHT} {
    m_buttons.emplace(Id::UP, config.pins.dpad.up);
    m_buttons.emplace(Id::DOWN, config.pins.dpad.down);
    m_buttons.emplace(Id::LEFT, config.pins.dpad.left);
    m_buttons.emplace(Id::RIGHT, config.pins.dpad.right);
    m_buttons.emplace(Id::NORTH, config.pins.buttons.north);
    m_buttons.emplace(Id::EAST, config.pins.buttons.east);
    m_buttons.emplace(Id::SOUTH, config.pins.buttons.south);
    m_buttons.emplace(Id::WEST, config.pins.buttons.west);
    m_buttons.emplace(Id::L1, config.pins.buttons.l1);
    m_buttons.emplace(Id::L2, config.pins.buttons.l2);
    m_buttons.emplace(Id::L3, config.pins.buttons.l3);
    m_buttons.emplace(Id::R1, config.pins.buttons.r1);
    m_buttons.emplace(Id::R2, config.pins.buttons.r2);
    m_buttons.emplace(Id::R3, config.pins.buttons.r3);
    m_buttons.emplace(Id::START, config.pins.buttons.start);
    m_buttons.emplace(Id::SELECT, config.pins.buttons.select);
    m_buttons.emplace(Id::HOME, config.pins.buttons.home);

    for (const auto &button : m_buttons) {
        gpio_init(button.second.getGpioPin());
        gpio_set_dir(button.second.getGpioPin(), GPIO_IN);
        gpio_pull_up(button.second.getGpioPin());
    }
}

void Buttons::setMirrorToDpad(bool mirror_to_dpad) { m_config.mirror_to_dpad = mirror_to_dpad; }

void Buttons::updateInputState(Utils::InputState &input_state) {
    uint32_t gpio_state = ~gpio_get_all();

    for (auto &button : m_buttons) {
        button.second.setState(gpio_state & button.second.getGpioMask(), m_config.debounce_delay_ms);
    }

    input_state.dpad.up = m_buttons.at(Id::UP).getState();
    input_state.dpad.down = m_buttons.at(Id::DOWN).getState();
    input_state.dpad.left = m_buttons.at(Id::LEFT).getState();
    input_state.dpad.right = m_buttons.at(Id::RIGHT).getState();
    input_state.buttons.north = m_buttons.at(Id::NORTH).getState();
    input_state.buttons.east = m_buttons.at(Id::EAST).getState();
    input_state.buttons.south = m_buttons.at(Id::SOUTH).getState();
    input_state.buttons.west = m_buttons.at(Id::WEST).getState();
    input_state.buttons.l1 = m_buttons.at(Id::L1).getState();
    input_state.buttons.l2 = m_buttons.at(Id::L2).getState();
    input_state.buttons.l3 = m_buttons.at(Id::L3).getState();
    input_state.buttons.r1 = m_buttons.at(Id::R1).getState();
    input_state.buttons.r2 = m_buttons.at(Id::R2).getState();
    input_state.buttons.r3 = m_buttons.at(Id::R3).getState();
    input_state.buttons.start = m_buttons.at(Id::START).getState();
    input_state.buttons.select = m_buttons.at(Id::SELECT).getState();
    input_state.buttons.home = m_buttons.at(Id::HOME).getState();

    if (m_config.mirror_to_dpad) {
        input_state.dpad.up |= m_buttons.at(Id::NORTH).getState();
        input_state.dpad.down |= m_buttons.at(Id::SOUTH).getState();
        input_state.dpad.left |= m_buttons.at(Id::WEST).getState();
        input_state.dpad.right |= m_buttons.at(Id::EAST).getState();
    }

    socdClean(input_state);
}
} // namespace Divacon::Peripherals
