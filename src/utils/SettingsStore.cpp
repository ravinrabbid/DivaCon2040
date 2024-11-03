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
                     Config::Default::touch_slider_leds_config.brightness,
                     Config::Default::touch_slider_leds_config.animation_speed,
                     Config::Default::touch_slider_leds_config.idle_mode,
                     Config::Default::touch_slider_leds_config.touched_mode,
                     Config::Default::touch_slider_leds_config.idle_color,
                     Config::Default::touch_slider_leds_config.touched_color,
                     Config::Default::touch_slider_leds_config.enable_player_color,
                     Config::Default::touch_slider_leds_config.enable_pdloader_support,
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

void SettingsStore::setLedBrightness(uint8_t brightness) {
    if (m_store_cache.led_brightness != brightness) {
        m_store_cache.led_brightness = brightness;
        m_dirty = true;
    }
}
uint8_t SettingsStore::getLedBrightness() { return m_store_cache.led_brightness; }

void SettingsStore::setLedAnimationSpeed(uint8_t speed) {
    if (m_store_cache.led_animation_speed != speed) {
        m_store_cache.led_animation_speed = speed;
        m_dirty = true;
    }
}
uint8_t SettingsStore::getLedAnimationSpeed() { return m_store_cache.led_animation_speed; }

void SettingsStore::setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode mode) {
    if (m_store_cache.led_idle_mode != mode) {
        m_store_cache.led_idle_mode = mode;
        m_dirty = true;
    }
}
Peripherals::TouchSliderLeds::Config::IdleMode SettingsStore::getLedIdleMode() { return m_store_cache.led_idle_mode; }

void SettingsStore::setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode mode) {
    if (m_store_cache.led_touched_mode != mode) {
        m_store_cache.led_touched_mode = mode;
        m_dirty = true;
    }
}
Peripherals::TouchSliderLeds::Config::TouchedMode SettingsStore::getLedTouchedMode() {
    return m_store_cache.led_touched_mode;
}

void SettingsStore::setLedIdleColor(Peripherals::TouchSliderLeds::Config::Color color) {
    if (m_store_cache.led_idle_color != color) {
        m_store_cache.led_idle_color = color;
        m_dirty = true;
    }
}
Peripherals::TouchSliderLeds::Config::Color SettingsStore::getLedIdleColor() { return m_store_cache.led_idle_color; }

void SettingsStore::setLedTouchedColor(Peripherals::TouchSliderLeds::Config::Color color) {
    if (m_store_cache.led_touched_color != color) {
        m_store_cache.led_touched_color = color;
        m_dirty = true;
    }
}
Peripherals::TouchSliderLeds::Config::Color SettingsStore::getLedTouchedColor() {
    return m_store_cache.led_touched_color;
}

void SettingsStore::setLedEnablePlayerColor(bool do_enable) {
    if (m_store_cache.led_enable_player_color != do_enable) {
        m_store_cache.led_enable_player_color = do_enable;
        m_dirty = true;
    }
}

bool SettingsStore::getLedEnablePlayerColor() { return m_store_cache.led_enable_player_color; };

void SettingsStore::setLedEnablePdloaderSupport(bool do_enable) {
    if (m_store_cache.led_enable_pdloader_support != do_enable) {
        m_store_cache.led_enable_pdloader_support = do_enable;
        m_dirty = true;
    }
}

bool SettingsStore::getLedEnablePdloaderSupport() { return m_store_cache.led_enable_pdloader_support; };

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

        restore_interrupts_from_disabled(interrupts);
        multicore_lockout_end_blocking();
    }

    switch (m_scheduled_reboot) {
    case RebootType::Normal:
        watchdog_reboot(0, 0, 1);
        break;
    case RebootType::Bootsel:
        sleep_ms(100);
        reset_usb_boot(0, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
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
