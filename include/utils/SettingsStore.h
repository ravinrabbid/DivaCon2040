#ifndef _UTILS_SETTINGSSTORE_H_
#define _UTILS_SETTINGSSTORE_H_

#include "peripherals/TouchSliderLeds.h"
#include "usb/device_driver.h"

#include "hardware/flash.h"

namespace Divacon::Utils {

class SettingsStore {
  private:
    const static uint32_t m_flash_size = FLASH_SECTOR_SIZE;
    const static uint32_t m_flash_offset = PICO_FLASH_SIZE_BYTES - m_flash_size;
    const static uint32_t m_store_size = FLASH_PAGE_SIZE;
    const static uint32_t m_store_pages = m_flash_size / m_store_size;
    const static uint8_t m_magic_byte = 0x39;

    struct __attribute((packed, aligned(1))) Storecache {
        uint8_t in_use;
        usb_mode_t usb_mode;
        uint8_t led_brightness;
        uint8_t led_animation_speed;
        Peripherals::TouchSliderLeds::Config::IdleMode led_idle_mode;
        Peripherals::TouchSliderLeds::Config::TouchedMode led_touched_mode;
        bool led_enable_player_color;
        bool led_enable_pdloader_support;

        uint8_t _padding[m_store_size - sizeof(uint8_t) - sizeof(usb_mode_t) - sizeof(uint8_t) - sizeof(uint8_t) -
                         sizeof(Peripherals::TouchSliderLeds::Config::IdleMode) -
                         sizeof(Peripherals::TouchSliderLeds::Config::TouchedMode) - sizeof(bool) - sizeof(bool)];
    };
    static_assert(sizeof(Storecache) == m_store_size);

    enum class RebootType {
        None,
        Normal,
        Bootsel,
    };

    Storecache m_store_cache;
    bool m_dirty;

    RebootType m_scheduled_reboot;

  private:
    Storecache read();

  public:
    SettingsStore();

    void setUsbMode(usb_mode_t mode);
    usb_mode_t getUsbMode();

    void setLedBrightness(uint8_t brightness);
    uint8_t getLedBrightness();

    void setLedAnimationSpeed(uint8_t speed);
    uint8_t getLedAnimationSpeed();

    void setLedIdleMode(Peripherals::TouchSliderLeds::Config::IdleMode mode);
    Peripherals::TouchSliderLeds::Config::IdleMode getLedIdleMode();

    void setLedTouchedMode(Peripherals::TouchSliderLeds::Config::TouchedMode mode);
    Peripherals::TouchSliderLeds::Config::TouchedMode getLedTouchedMode();

    void setLedEnablePlayerColor(bool do_enable);
    bool getLedEnablePlayerColor();

    void setLedEnablePdloaderSupport(bool do_enable);
    bool getLedEnablePdloaderSupport();

    void scheduleReboot(bool bootsel = false);

    void store();
    void reset();
};
} // namespace Divacon::Utils

#endif // _UTILS_SETTINGSSTORE_H_