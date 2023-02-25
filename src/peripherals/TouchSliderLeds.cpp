#include "peripherals/TouchSliderLeds.h"

#include "pio_ws2812/ws2812.h"

#include <algorithm>

namespace Divacon::Peripherals {

TouchSliderLeds::TouchSliderLeds(const Config &config)
    : m_config(config), m_touched(0), m_background_brightness(config.brightness), m_touched_brightness({}) {
    m_frame = std::vector<uint32_t>(32 * config.leds_per_segment, ws2812_rgb_to_u32pixel(0, 0, 0));
    ws2812_init(config.led_pin, m_config.is_rgbw);
}

void TouchSliderLeds::setBrightness(uint8_t brightness) { m_config.brightness = brightness; }

void TouchSliderLeds::setTouched(uint32_t touched) { m_touched = touched; }

static uint32_t get_dimmed_pixel(TouchSliderLeds::Config::Color color, uint8_t dim) {
    float dim_factor = dim / 255.;
    return ws2812_rgb_to_gamma_corrected_u32pixel(color.r * dim_factor, color.g * dim_factor, color.b * dim_factor);
}

void TouchSliderLeds::update() {
    static uint32_t last_frame = 0;

    const static uint32_t interval = 10; // TODO shorter?
    const static uint8_t brightness_step = 8;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((last_frame + interval) <= now) {
        if (m_touched == 0) {
            m_background_brightness =
                std::min(m_background_brightness + brightness_step, static_cast<int>(m_config.brightness));
        } else {
            m_background_brightness = std::max(m_background_brightness - brightness_step, 0);
        }

        for (uint8_t bit = 0; bit < 32; ++bit) {
            uint32_t segment_color = get_dimmed_pixel(m_config.background_color, m_background_brightness);

            if (m_touched) {
                auto is_touched = [this](uint8_t bit) {
                    if (m_config.reverse) {
                        return m_touched & (0x80000000 >> bit);
                    } else {
                        return m_touched & (0x00000001 << bit);
                    }
                };

                if (is_touched(bit)) {
                    m_touched_brightness[bit] = m_config.brightness;
                } else {
                    m_touched_brightness[bit] = std::max(m_touched_brightness[bit] - brightness_step, 0);
                }

                if (m_touched_brightness[bit] != 0) {
                    segment_color = get_dimmed_pixel(m_config.touched_color, m_touched_brightness[bit]);
                }
            } else {
                m_touched_brightness[bit] = 0;
            }

            for (int led = 0; led < m_config.leds_per_segment; ++led) {
                m_frame.at((bit * m_config.leds_per_segment) + led) = segment_color;
            }
        }

        ws2812_put_frame(m_frame.data(), m_frame.size());

        last_frame = now;
    }
}

} // namespace Divacon::Peripherals