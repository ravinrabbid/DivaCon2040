#ifndef _PERIPHERALS_TOUCHSLIDERLEDS_H_
#define _PERIPHERALS_TOUCHSLIDERLEDS_H_

#include <algorithm>
#include <array>
#include <stdint.h>
#include <vector>

namespace Divacon::Peripherals {

class TouchSliderLeds {
  public:
    struct Config {
        struct Color {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        uint8_t led_pin;
        bool is_rgbw;
        bool reverse;
        uint16_t leds_per_segment;
        uint8_t brightness;
        uint8_t fade_speed;
        Color background_color;
        Color touched_color;
    };

    using RawFrameMessage = std::array<Config::Color, 32>;

  private:
    Config m_config;
    uint32_t m_touched;
    std::vector<uint32_t> m_frame;

    uint8_t m_background_brightness;
    std::array<uint8_t, 32> m_touched_brightness;

    bool m_raw_mode;

  public:
    TouchSliderLeds(const Config &config);

    void setBrightness(uint8_t brightness);
    void setTouched(uint32_t touched);
    void setPlayerColor(Config::Color color);

    void update();
    void update(const RawFrameMessage &frame);
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_TOUCHSLIDERLEDS_H_