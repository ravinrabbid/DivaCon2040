#include "peripherals/TouchSlider.h"

#include "hardware/gpio.h"

namespace Divacon::Peripherals {

namespace {
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
} // namespace

TouchSlider::TouchControllerMpr121::TouchControllerMpr121(const TouchSlider::Config::Mpr121 &config, i2c_inst *i2c) {
    size_t idx = 0;
    for (auto &mpr121 : m_mpr121) {
        mpr121 = std::make_unique<Mpr121>(config.i2c_addresses[idx], i2c, config.touch_threshold,
                                          config.release_threshold, true);
        idx++;
    }
}

uint32_t TouchSlider::TouchControllerMpr121::read() {
    // Electrodes are mapped according to below table.
    // If you had more success hooking up your slider,
    // change this accordingly.
    //
    //         | m_mpr121[0] | m_mpr121[1] | m_mpr121[2] |
    // --------+-------------+-------------+-------------+
    // Pin     |    0..11    |    2..9     |    0..11    |
    // Touched |   31..20    |   19..12    |    11..0    |

    return (reverseBits(m_mpr121[0]->getTouched()) << 16) | (reverseBits(m_mpr121[1]->getTouched() & 0x03FC) << 6) |
           (reverseBits(m_mpr121[2]->getTouched()) >> 4);
}

TouchSlider::TouchControllerCap1188::TouchControllerCap1188(const TouchSlider::Config::Cap1188 &config, i2c_inst *i2c) {
    size_t idx = 0;
    for (auto &cap1188 : m_cap1188) {
        cap1188 = std::make_unique<Cap1188>(config.i2c_addresses[idx], i2c, config.threshold, config.sensitivity,
                                            Cap1188::Gain::G1);
        idx++;
    }
}

uint32_t TouchSlider::TouchControllerCap1188::read() {
    // Electrodes are mapped according to below table.
    //
    //         | m_cap1188[3] | m_cap1188[2] | m_cap1188[1] | m_cap1188[0] |
    // --------+--------------+--------------+--------------+--------------+
    // Pin     |     7..0     |     7..0     |     7..0     |     7..0     |
    // Touched |    31..24    |    23..16    |    15..8     |     7..0     |

    return (m_cap1188[3]->getTouched() << 24) | (m_cap1188[2]->getTouched() << 16) | (m_cap1188[1]->getTouched() << 8) |
           (m_cap1188[0]->getTouched());
}

TouchSlider::TouchControllerIs31se5117a::TouchControllerIs31se5117a(const TouchSlider::Config::Is31se5117a &config,
                                                                    i2c_inst *i2c) {
    size_t idx = 0;
    for (auto &is31se5117a : m_is31se5117a) {
        is31se5117a =
            std::make_unique<Is31se5117a>(config.i2c_addresses[idx], i2c, config.threshold, config.hysteresis);
        idx++;
    }
}

uint32_t TouchSlider::TouchControllerIs31se5117a::read() {
    // Electrodes are mapped according to below table.
    //
    //         | m_is31se5117a[1] | m_is31se5117a[0] |
    // --------+------------------+------------------+
    // Pin     |       0..15      |       0..15      |
    // Touched |      31..16      |      15..0       |

    return ((reverseBits(m_is31se5117a[1]->getTouched()) << 16) | reverseBits(m_is31se5117a[0]->getTouched()));
}

TouchSlider::TouchSlider(const Config &config, usb_mode_t mode) : m_config(config), m_mode(mode), m_touched(0) {
    gpio_set_function(m_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(m_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(m_config.sda_pin);
    gpio_pull_up(m_config.scl_pin);

    i2c_init(m_config.i2c_block, m_config.i2c_speed_hz);

    std::visit(
        [this](auto &&config) {
            using T = std::decay_t<decltype(config)>;

            if constexpr (std::is_same_v<T, Config::Mpr121>) {
                m_touch_controller = std::make_unique<TouchControllerMpr121>(config, m_config.i2c_block);
            } else if constexpr (std::is_same_v<T, Config::Cap1188>) {
                m_touch_controller = std::make_unique<TouchControllerCap1188>(config, m_config.i2c_block);
            } else if constexpr (std::is_same_v<T, Config::Is31se5117a>) {
                m_touch_controller = std::make_unique<TouchControllerIs31se5117a>(config, m_config.i2c_block);
            } else {
                static_assert(false, "Unknown touch controller!");
            }
        },
        m_config.touch_config);
}

void TouchSlider::updateInputStateArcade(Utils::InputState &input_state) {
    // The 32bit state vector is mapped into the 4 8bit axes of the analog sticks, XORed
    // with the stick center postion to ensure no stick movement when the slider is not touched.
    input_state.sticks.right.y = (uint8_t)((m_touched & 0xFF000000) >> 24) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.right.x = (uint8_t)((m_touched & 0x00FF0000) >> 16) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.left.y = (uint8_t)((m_touched & 0x0000FF00) >> 8) ^ Utils::InputState::AnalogStick::center;
    input_state.sticks.left.x = (uint8_t)((m_touched & 0x000000FF)) ^ Utils::InputState::AnalogStick::center;
}

void TouchSlider::updateInputStateStick(Utils::InputState &input_state) {
    struct State {
        uint8_t left_limit;
        uint8_t right_limit;
        uint8_t x_axis;
    };

    static State previous_left = {0, UINT8_MAX, Utils::InputState::AnalogStick::center};
    static State previous_right = {0, UINT8_MAX, Utils::InputState::AnalogStick::center};

    auto handleSide = [](uint16_t touched, State &previous_state, uint8_t &target) {
        if (touched != 0) {
            uint8_t left_limit = 0;
            uint8_t right_limit = UINT8_MAX;

            // Find leftmost touch
            for (uint8_t i = 0; i < 16; ++i) {
                if ((0x0001 << (15 - i)) & touched) {
                    left_limit = (15 - i);
                    break;
                }
            }
            // Find rightmost touch
            for (uint8_t i = 0; i < 16; ++i) {
                if ((0x0001 << i) & touched) {
                    right_limit = i;
                    break;
                }
            }

            // Either of the extreme positions moved left
            if (((left_limit > previous_state.left_limit) && (right_limit >= previous_state.right_limit)) ||
                ((left_limit >= previous_state.left_limit) && (right_limit > previous_state.right_limit))) {
                target = 0;
                // Either of the extreme positions moved right
            } else if (((left_limit < previous_state.left_limit) && (right_limit <= previous_state.right_limit)) ||
                       ((left_limit <= previous_state.left_limit) && (right_limit < previous_state.right_limit))) {
                target = UINT8_MAX;
                // No movement, but still touched
            } else {
                target = previous_state.x_axis;
            }

            previous_state.left_limit = left_limit;
            previous_state.right_limit = right_limit;
        } else {
            // No touch, reset
            previous_state.left_limit = 0;
            previous_state.right_limit = UINT8_MAX;
            target = Utils::InputState::AnalogStick::center;
        }
        previous_state.x_axis = target;
    };

    // Interpret slider as two distinctive zones, controlling left and right
    // stick x-axis respectively
    handleSide(m_touched >> 16, previous_left, input_state.sticks.left.x);
    handleSide(m_touched & 0x0000FFFF, previous_right, input_state.sticks.right.x);

    input_state.sticks.left.y = Utils::InputState::AnalogStick::center;
    input_state.sticks.right.y = Utils::InputState::AnalogStick::center;
}

void TouchSlider::updateInputState(Utils::InputState &input_state) {

    read();

    switch (m_mode) {

    case USB_MODE_SWITCH_DIVACON:
    case USB_MODE_PS4_DIVACON:
    case USB_MODE_PDLOADER:
    case USB_MODE_MIDI:
    case USB_MODE_DEBUG:
        updateInputStateArcade(input_state);
        break;
    case USB_MODE_SWITCH_HORIPAD:
    case USB_MODE_DUALSHOCK3:
    case USB_MODE_DUALSHOCK4:
    case USB_MODE_PS4_COMPAT:
    case USB_MODE_XBOX360:
    case USB_MODE_KEYBOARD:
        updateInputStateStick(input_state);
        break;
    }

    input_state.touches = m_touched;
}

void TouchSlider::read() {
    static uint32_t last_read = 0;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((last_read + 1) <= now) {
        m_touched = m_touch_controller->read();
    }
}

} // namespace Divacon::Peripherals