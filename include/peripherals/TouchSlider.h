#ifndef _PERIPHERALS_TOUCHSLIDER_H_
#define _PERIPHERALS_TOUCHSLIDER_H_

#include "utils/InputState.h"

#include <mpr121/Mpr121.h>

#include "hardware/i2c.h"

#include <array>
#include <memory>
#include <stdint.h>

namespace Divacon::Peripherals {

class TouchSlider {
  private:
    const static size_t mpr121_count = 3;

  public:
    struct Config {
        enum class Mode {
            ARCADE,
            STICK,
        };

        Mode mode;
        uint8_t touch_threshold;
        uint8_t release_threshold;

        uint8_t sda_pin;
        uint8_t scl_pin;
        i2c_inst_t *i2c_block;
        uint i2c_speed_hz;
        uint8_t mpr121_address[mpr121_count];
    };

  private:
    Config m_config;
    uint32_t m_touched;
    std::array<std::unique_ptr<Mpr121>, mpr121_count> m_mpr121;

    void read();

  public:
    TouchSlider(const Config &config);

    void setMode(Config::Mode mode);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_TOUCHSLIDER_H_