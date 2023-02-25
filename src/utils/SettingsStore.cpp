#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"

#include "pico/multicore.h"

namespace Divacon::Utils {

static uint8_t read_byte(uint32_t offset) { return *(reinterpret_cast<uint8_t *>(XIP_BASE + offset)); }

SettingsStore::SettingsStore()
    : m_store_cache({m_magic_byte,
                     Config::Default::usb_mode,
                     Config::Default::touch_slider_config.mode,
                     Config::Default::touch_slider_leds_config.brightness,
                     {}}),
      m_dirty(true) {

    uint32_t current_page = m_flash_offset + m_flash_size - m_store_size;
    bool found_valid = false;
    for (uint8_t i = 0; i < m_store_pages; ++i) {
        if (read_byte(current_page) == m_magic_byte) {
            found_valid = true;
            break;
        } else {
            current_page -= m_store_size;
        }
    }

    if (found_valid) {
        m_store_cache = *(reinterpret_cast<Storecache *>(XIP_BASE + current_page));
        m_dirty = false;
    }
}

void SettingsStore::setUsbMode(usb_mode_t mode) {
    if (mode != m_store_cache.usb_mode) {
        m_store_cache.usb_mode = mode;
        m_dirty = true;
    }
}

usb_mode_t SettingsStore::getUsbMode() { return m_store_cache.usb_mode; }

void SettingsStore::setSliderMode(Peripherals::TouchSlider::Config::Mode mode) {
    if (m_store_cache.slider_mode != mode) {
        m_store_cache.slider_mode = mode;
        m_dirty = true;
    }
}
Peripherals::TouchSlider::Config::Mode SettingsStore::getSliderMode() { return m_store_cache.slider_mode; }

void SettingsStore::setLedBrightness(uint8_t brightness) {
    if (m_store_cache.led_brightness != brightness) {
        m_store_cache.led_brightness = brightness;
        m_dirty = true;
    }
}
uint8_t SettingsStore::getLedBrightness() { return m_store_cache.led_brightness; }

void SettingsStore::store() {
    if (m_dirty) {
        multicore_lockout_start_timeout_us(0xfffffffffffffff); // TODO SDK Bug: change to blocking with next SDK relase
        uint32_t interrupts = save_and_disable_interrupts();

        uint32_t current_page = m_flash_offset;
        bool do_erase = true;
        for (uint8_t i = 0; i < m_store_pages; ++i) {
            if (read_byte(current_page) == 0xFF) {
                do_erase = false;
                break;
            } else {
                current_page += m_store_size;
            }
        }

        if (do_erase) {
            flash_range_erase(m_flash_offset, m_flash_size);
            current_page = m_flash_offset;
        }

        flash_range_program(current_page, reinterpret_cast<uint8_t *>(&m_store_cache), sizeof(m_store_cache));

        m_dirty = false;

        restore_interrupts(interrupts);
        multicore_lockout_end_timeout_us(0xfffffffffffffff); // TODO SDK Bug: change to blocking with next SDK relase
    }
}

} // namespace Divacon::Utils
