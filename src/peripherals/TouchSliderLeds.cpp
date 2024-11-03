#include "peripherals/TouchSliderLeds.h"

#include "pico/rand.h"
#include "pio_ws2812/ws2812.h"

#include <algorithm>

namespace Divacon::Peripherals {

namespace {
const static uint32_t pulse_step_count = 4096;
static const uint8_t pulse_dim_percent_min = 40;
static const uint8_t pulse_dim_percent_max = 100;

const static uint32_t rainbow_step_count = 4096;

const static uint32_t fade_step_count = 2048;

const static uint32_t blend_step_count = 128;

// Use alternating frames to allow for smoother animation
const static size_t rainbow_length = 40;
const static std::array<std::array<TouchSliderLeds::Config::Color, rainbow_length>, 2> rainbow_colors{{
    {{
        {0x5a, 0x3a, 0xc6}, {0x76, 0x36, 0xaa}, {0x91, 0x34, 0x8e}, {0xad, 0x30, 0x72}, {0xca, 0x2e, 0x56},
        {0xe6, 0x2a, 0x3a}, {0xf2, 0x2f, 0x2b}, {0xe6, 0x42, 0x33}, {0xce, 0x5c, 0x46}, {0xb6, 0x74, 0x59},
        {0x9e, 0x8d, 0x6c}, {0x86, 0xa6, 0x7e}, {0x6e, 0xbf, 0x90}, {0x57, 0xd8, 0xa3}, {0x4c, 0xea, 0xac},
        {0x58, 0xf0, 0xa3}, {0x6d, 0xf0, 0x92}, {0x83, 0xf0, 0x82}, {0x99, 0xf0, 0x72}, {0xae, 0xf0, 0x61},
        {0xc4, 0xf0, 0x50}, {0xdb, 0xf0, 0x40}, {0xec, 0xea, 0x34}, {0xf2, 0xdc, 0x34}, {0xf4, 0xc9, 0x38},
        {0xf6, 0xb6, 0x3c}, {0xf8, 0xa2, 0x40}, {0xfa, 0x90, 0x45}, {0xfc, 0x7d, 0x49}, {0xfe, 0x6a, 0x4d},
        {0xf8, 0x5c, 0x56}, {0xe2, 0x56, 0x68}, {0xc6, 0x52, 0x7d}, {0xaa, 0x50, 0x93}, {0x8e, 0x4c, 0xa9},
        {0x72, 0x4a, 0xbf}, {0x56, 0x46, 0xd5}, {0x3a, 0x44, 0xeb}, {0x2e, 0x40, 0xf3}, {0x3e, 0x3c, 0xe2},
    }},
    {{
        {0x68, 0x38, 0xb7}, {0x83, 0x35, 0x9c}, {0x9f, 0x32, 0x80}, {0xbb, 0x2f, 0x64}, {0xd8, 0x2c, 0x48},
        {0xf3, 0x29, 0x2c}, {0xf2, 0x35, 0x2a}, {0xda, 0x4f, 0x3c}, {0xc3, 0x68, 0x50}, {0xaa, 0x81, 0x62},
        {0x92, 0x99, 0x75}, {0x7a, 0xb2, 0x87}, {0x63, 0xcc, 0x9a}, {0x4b, 0xe5, 0xac}, {0x4d, 0xf0, 0xab},
        {0x62, 0xf0, 0x9b}, {0x78, 0xf0, 0x8a}, {0x8e, 0xf0, 0x7a}, {0xa4, 0xf0, 0x69}, {0xb9, 0xf0, 0x59},
        {0xd0, 0xf0, 0x48}, {0xe6, 0xf0, 0x37}, {0xf1, 0xe5, 0x32}, {0xf3, 0xd2, 0x36}, {0xf5, 0xc0, 0x3a},
        {0xf7, 0xac, 0x3e}, {0xf9, 0x99, 0x43}, {0xfb, 0x86, 0x47}, {0xfd, 0x74, 0x4b}, {0xff, 0x61, 0x4f},
        {0xf0, 0x57, 0x5d}, {0xd4, 0x54, 0x72}, {0xb7, 0x51, 0x88}, {0x9c, 0x4e, 0x9e}, {0x80, 0x4b, 0xb4},
        {0x64, 0x48, 0xca}, {0x48, 0x45, 0xe0}, {0x2c, 0x42, 0xf6}, {0x2f, 0x3e, 0xf0}, {0x4c, 0x3b, 0xd4},
    }},
}};

struct AnimationStepper {
    const uint32_t steps_to_advance;
    uint32_t current_steps;

    uint32_t advance(uint32_t steps) {
        current_steps += steps;
        if (current_steps < steps_to_advance) {
            return 0;
        }

        const auto advance = current_steps / steps_to_advance;

        current_steps %= steps_to_advance;

        return advance;
    }
};

TouchSliderLeds::Config::Color dim_color(const TouchSliderLeds::Config::Color &color, float dim_factor) {
    return TouchSliderLeds::Config::Color{
        .r = (uint8_t)((float)color.r * dim_factor),
        .g = (uint8_t)((float)color.g * dim_factor),
        .b = (uint8_t)((float)color.b * dim_factor),
    };
}

TouchSliderLeds::Config::Color max_color(const TouchSliderLeds::Config::Color &a,
                                         const TouchSliderLeds::Config::Color &b) {
    return TouchSliderLeds::Config::Color{
        .r = std::max(a.r, b.r),
        .g = std::max(a.g, b.g),
        .b = std::max(a.b, b.b),
    };
}

} // namespace

TouchSliderLeds::TouchSliderLeds(const Config &config)
    : m_config(config), m_touched(0), m_idle_buffer({}), m_touched_buffer({}), m_player_color(std::nullopt),
      m_raw_mode(false) {
    m_rendered_frame = std::vector<uint32_t>(32 * config.leds_per_segment, ws2812_rgb_to_u32pixel(0, 0, 0));

    ws2812_init(config.led_pin, m_config.is_rgbw);
}

void TouchSliderLeds::setBrightness(uint8_t brightness) { m_config.brightness = brightness; }
void TouchSliderLeds::setAnimationSpeed(uint8_t speed) { m_config.animation_speed = speed; };
void TouchSliderLeds::setIdleMode(Config::IdleMode mode) { m_config.idle_mode = mode; };
void TouchSliderLeds::setTouchedMode(Config::TouchedMode mode) { m_config.touched_mode = mode; };
void TouchSliderLeds::setIdleColor(Config::Color color) { m_config.idle_color = color; };
void TouchSliderLeds::setTouchedColor(Config::Color color) { m_config.touched_color = color; };
void TouchSliderLeds::setEnablePlayerColor(bool do_enable) { m_config.enable_player_color = do_enable; };
void TouchSliderLeds::setEnablePdloaderSupport(bool do_enable) { m_config.enable_pdloader_support = do_enable; };

void TouchSliderLeds::setTouched(uint32_t touched) { m_touched = touched; }
void TouchSliderLeds::setPlayerColor(TouchSliderLeds::Config::Color color) { m_player_color = color; }

void TouchSliderLeds::updateIdle(uint32_t steps) {
    // Pulse
    static AnimationStepper pulse_stepper{pulse_step_count, 0};
    static uint8_t pulse_dim_percent = pulse_dim_percent_max;
    static int8_t pulse_advance_factor = -1;

    // Rainbow
    static AnimationStepper rainbow_stepper{rainbow_step_count, 0};
    static size_t rainbow_position = get_rand_32() % rainbow_length;

    if (steps <= 0) {
        return;
    }

    // Check for player color
    const auto idle_color =
        m_config.enable_player_color ? m_player_color.value_or(m_config.idle_color) : m_config.idle_color;

    switch (m_config.idle_mode) {
    case Config::IdleMode::Off:
        m_idle_buffer.fill({0x00, 0x00, 0x00});
        break;
    case Config::IdleMode::Static:
        m_idle_buffer.fill(idle_color);
        break;
    case Config::IdleMode::Pulse: {
        const auto advance = pulse_stepper.advance(steps);

        if (pulse_advance_factor < 0 && advance >= (uint8_t)(pulse_dim_percent - pulse_dim_percent_min)) {
            pulse_dim_percent = pulse_dim_percent_min;
            pulse_advance_factor = -pulse_advance_factor;
        } else if (pulse_advance_factor > 0 && advance + pulse_dim_percent >= pulse_dim_percent_max) {
            pulse_dim_percent = pulse_dim_percent_max;
            pulse_advance_factor = -pulse_advance_factor;
        } else {
            pulse_dim_percent = pulse_dim_percent + (pulse_advance_factor * advance);
        }

        m_idle_buffer.fill(dim_color(idle_color, (float)pulse_dim_percent / 100.));
    } break;
    case Config::IdleMode::RainbowCycle:
        rainbow_position =
            (rainbow_position + rainbow_stepper.advance(steps)) % (rainbow_length * rainbow_colors.size());
        [[fallthrough]];
    case Config::IdleMode::RainbowStatic: {
        const auto frame = rainbow_position % rainbow_colors.size();
        const auto frame_position = rainbow_position / rainbow_colors.size();

        for (size_t idx = 0; idx < m_idle_buffer.size(); ++idx) {
            size_t offset = (frame_position + idx) % rainbow_length;
            m_idle_buffer[idx] = rainbow_colors[frame][offset];
        }
    } break;
    }
}
void TouchSliderLeds::updateTouched(uint32_t steps) {
    static AnimationStepper fade_stepper{fade_step_count, 0};
    static std::array<uint8_t, SEGMENT_COUNT> fade_percent = {};

    switch (m_config.touched_mode) {
    case Config::TouchedMode::Off:
        m_touched_buffer.fill({0x00, 0x00, 0x00});
        break;
    case Config::TouchedMode::Idle:
        std::copy(m_idle_buffer.cbegin(), m_idle_buffer.cend(), m_touched_buffer.begin());
        break;
    case Config::TouchedMode::Touched:
        for (size_t idx = 0; idx < SEGMENT_COUNT; ++idx) {
            if (m_touched & ((uint32_t)0x80000000 >> idx)) {
                m_touched_buffer[idx] = m_config.touched_color;
            } else {
                m_touched_buffer[idx] = {0x00, 0x00, 0x00};
            }
        }
        break;
    case Config::TouchedMode::TouchedFade: {
        const auto advance = fade_stepper.advance(steps);

        for (size_t idx = 0; idx < SEGMENT_COUNT; ++idx) {
            if (m_touched & ((uint32_t)0x80000000 >> idx)) {
                m_touched_buffer[idx] = m_config.touched_color;
                fade_percent[idx] = 100;
            } else {
                m_touched_buffer[idx] = dim_color(m_touched_buffer[idx], (float)fade_percent[idx] / 100.);
                fade_percent[idx] = advance > fade_percent[idx] ? 0 : fade_percent[idx] - advance;
            }
        }
    } break;
    case Config::TouchedMode::TouchedIdle: {
        const auto advance = fade_stepper.advance(steps);

        for (size_t idx = 0; idx < SEGMENT_COUNT; ++idx) {
            if (m_touched & ((uint32_t)0x80000000 >> idx)) {
                m_touched_buffer[idx] = m_idle_buffer[idx];
                fade_percent[idx] = 100;
            } else {
                m_touched_buffer[idx] = dim_color(m_touched_buffer[idx], (float)fade_percent[idx] / 100.);
                fade_percent[idx] = advance > fade_percent[idx] ? 0 : fade_percent[idx] - advance;
            }
        }
    } break;
    }
}

void TouchSliderLeds::render(uint32_t steps) {
    static AnimationStepper blend_stepper{blend_step_count, 0};
    static uint8_t blend_percent = 100;

    const auto blend_advance = blend_stepper.advance(steps);
    if (m_touched) {
        blend_percent = blend_advance > blend_percent ? 0 : blend_percent - blend_advance;
    } else {
        blend_percent = blend_advance + blend_percent > 100 ? 100 : blend_percent + blend_advance;
    }

    const float brightness_dim_factor = (float)m_config.brightness / 255.;
    const float blend_dim_factor = (float)blend_percent / 100.;

    size_t idx = 0;
    for (auto &rendered_segment : m_rendered_frame) {
        auto blended_segment = max_color(dim_color(m_idle_buffer[idx / m_config.leds_per_segment], blend_dim_factor),
                                         m_touched_buffer[idx / m_config.leds_per_segment]);

        rendered_segment =
            ws2812_rgb_to_u32pixel(blended_segment.r * brightness_dim_factor, blended_segment.g * brightness_dim_factor,
                                   blended_segment.b * brightness_dim_factor);

        ++idx;
    }
}

void TouchSliderLeds::show() { ws2812_put_frame(m_rendered_frame.data(), m_rendered_frame.size()); }

void TouchSliderLeds::update() {
    static uint32_t previous_frame_time = to_ms_since_boot(get_absolute_time());

    const uint32_t now = to_ms_since_boot(get_absolute_time());
    const uint32_t elapsed = now - previous_frame_time;
    const uint32_t steps = elapsed * m_config.animation_speed;

    previous_frame_time = now;

    if (m_raw_mode && m_config.enable_pdloader_support) {
        return;
    }

    updateIdle(steps);
    updateTouched(steps);

    render(steps);
    show();
}

void TouchSliderLeds::update(const TouchSliderLeds::RawFrameMessage &frame) {
    if (!m_config.enable_pdloader_support) {
        return;
    }

    m_raw_mode = true;

    size_t idx = 0;
    for (const auto &color : frame) {
        for (int led = 0; led < m_config.leds_per_segment; ++led) {
            // Allow limiting max brightness to stay within USB power restrictions.
            uint8_t color_max = std::max(color.r, std::max(color.g, color.b));
            if (color_max > m_config.brightness) {
                float dim_factor = (float)m_config.brightness / (float)color_max;
                m_rendered_frame.at((idx * m_config.leds_per_segment) + led) = ws2812_rgb_to_u32pixel(
                    (float)color.r * dim_factor, (float)color.g * dim_factor, (float)color.b * dim_factor);
            } else {
                m_rendered_frame.at((idx * m_config.leds_per_segment) + led) =
                    ws2812_rgb_to_u32pixel(color.r, color.g, color.b);
            }
        }
        ++idx;
    }

    ws2812_put_frame(m_rendered_frame.data(), m_rendered_frame.size());
}

} // namespace Divacon::Peripherals