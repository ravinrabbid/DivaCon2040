#ifndef _PERIPHERALS_TOUCHSLIDERLEDS_H_
#define _PERIPHERALS_TOUCHSLIDERLEDS_H_

#include <algorithm>
#include <array>
#include <optional>
#include <stdint.h>
#include <vector>

namespace Divacon::Peripherals {

class TouchSliderLeds {
  private:
    const static size_t SEGMENT_COUNT = 32;

  public:
    struct Config {
        struct Color {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        enum class IdleMode {
            Off,
            Static,
            Pulse,
            RainbowStatic,
            RainbowCycle,
        };

        enum class TouchedMode {
            Off,
            Idle,
            Touched,
            TouchedFade,
            TouchedIdle,
        };

        uint8_t led_pin;
        bool is_rgbw;
        bool reverse;
        uint16_t leds_per_segment;

        uint8_t brightness;
        uint8_t animation_speed;
        IdleMode idle_mode;
        TouchedMode touched_mode;
        Color idle_color;
        Color touched_color;

        bool enable_player_color;
        bool enable_pdloader_support;
    };

    using RawFrameMessage = std::array<Config::Color, SEGMENT_COUNT>;

  private:
    Config m_config;
    uint32_t m_touched;

    std::vector<uint32_t> m_rendered_frame;

    std::array<Config::Color, SEGMENT_COUNT> m_idle_buffer;
    std::array<Config::Color, SEGMENT_COUNT> m_touched_buffer;

    std::optional<Config::Color> m_player_color;

    bool m_raw_mode;

    void updateIdle(uint32_t steps);
    void updateTouched(uint32_t steps);

    void render(uint32_t steps);
    void show();

  public:
    TouchSliderLeds(const Config &config);

    void setBrightness(uint8_t brightness);
    void setAnimationSpeed(uint8_t speed);
    void setIdleMode(Config::IdleMode mode);
    void setTouchedMode(Config::TouchedMode mode);
    void setEnablePlayerColor(bool do_enable);
    void setEnablePdloaderSupport(bool do_enable);

    void setTouched(uint32_t touched);
    void setPlayerColor(Config::Color color);

    void update();
    void update(const RawFrameMessage &frame);
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_TOUCHSLIDERLEDS_H_