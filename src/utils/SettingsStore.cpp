#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"

#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"

namespace Divacon::Utils {

static uint8_t read_byte(uint32_t offset) { return *(reinterpret_cast<uint8_t *>(XIP_BASE + offset)); }

SettingsStore::SettingsStore()
    : m_store_cache({m_magic_byte,
                     Config::Default::usb_mode,
                     Config::Default::touch_slider_config.mode,
                     Config::Default::touch_slider_leds_config.brightness,
                     {}}),
      m_dirty(true), m_scheduled_reboot(RebootType::None) {

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

        scheduleReboot();
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
        multicore_lockout_start_blocking();
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
        multicore_lockout_end_blocking();
    }

    switch (m_scheduled_reboot) {
    case RebootType::Normal:
        watchdog_enable(1, 1);
        while (1)
            ;
        break;
    case RebootType::Bootsel:
        sleep_ms(100);
        reset_usb_boot(0, 0);
        break;
    case RebootType::None:
        break;
    }
}

void SettingsStore::reset() {
    multicore_lockout_start_blocking();
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(m_flash_offset, m_flash_size);
    
    restore_interrupts(interrupts);
    multicore_lockout_end_blocking();

    m_dirty = false;

    scheduleReboot();
    store();
}

void SettingsStore::scheduleReboot(bool bootsel) {
    if (m_scheduled_reboot != RebootType::Bootsel) {
        m_scheduled_reboot = (bootsel ? RebootType::Bootsel : RebootType::Normal);
    }
}

} // namespace Divacon::Utils
