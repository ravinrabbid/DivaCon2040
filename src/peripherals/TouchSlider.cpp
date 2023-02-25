#include "peripherals/TouchSlider.h"

#include "hardware/gpio.h"

namespace Divacon::Peripherals {

TouchSlider::TouchSlider(const Config &config) : m_config(config), m_touched(0) {
    i2c_init(m_config.i2c_block, m_config.i2c_speed_hz);
    gpio_set_function(m_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(m_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(m_config.sda_pin);
    gpio_pull_up(m_config.scl_pin);

    size_t idx = 0;
    for (auto &mpr121 : m_mpr121) {
        mpr121 = std::make_unique<Mpr121>(m_config.mpr121_address[idx], m_config.i2c_block, m_config.touch_threshold,
                                          m_config.release_threshold, true);
        idx++;
    }
}

void TouchSlider::setMode(Config::Mode mode) { m_config.mode = mode; };

void TouchSlider::updateInputState(Utils::InputState &input_state) {
    read();

    // TODO stick mode

    input_state.touches = m_touched;

    input_state.sticks.right.y = (uint8_t)((m_touched & 0xFF000000) >> 24) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.right.x = (uint8_t)((m_touched & 0x00FF0000) >> 16) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.left.y = (uint8_t)((m_touched & 0x0000FF00) >> 8) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.left.x = (uint8_t)((m_touched & 0x000000FF)) ^ Utils::InputState::AnalogStick::center;
}

void TouchSlider::read() {
    static uint32_t last_read = 0;

    auto reverseBits = [](uint16_t input) {
        uint16_t result = 0;
        uint8_t bit = 0;
        while (input > 0) {
            result += (input % 2) << (15 - bit);
            input >>= 1;
            bit++;
        }
        return result;
    };

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((last_read + 1) <= now) {
        m_touched = (reverseBits(m_mpr121[0]->getTouched()) << 16) |
                    (reverseBits(m_mpr121[1]->getTouched() & 0x03FC) << 6) |
                    (reverseBits(m_mpr121[2]->getTouched()) >> 4);
        last_read = now;
    }
}

} // namespace Divacon::Peripherals