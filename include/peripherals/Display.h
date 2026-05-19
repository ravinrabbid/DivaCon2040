#ifndef PERIPHERALS_DISPLAY_H_
#define PERIPHERALS_DISPLAY_H_

#include "usb/device_driver.h"
#include "utils/InputState.h"
#include "utils/Menu.h"

#include <ssd1306/ssd1306.h>

#include "hardware/i2c.h"

#include <cstdint>
#include <list>
#include <memory>

namespace Divacon::Peripherals {

class Display {
  public:
    struct Config {
        uint8_t sda_pin;
        uint8_t scl_pin;
        i2c_inst_t *i2c_block;
        uint i2c_speed_hz;
        uint8_t i2c_address;
    };

  private:
    enum class State : uint8_t {
        Idle,
        Menu,
    };

    class BpmCounter {
      private:
        class Buffer {
          private:
            std::list<uint16_t> m_buffer;
            size_t m_size_limit;

          public:
            Buffer(size_t size_limit) : m_size_limit(size_limit) {};

            void insert(uint16_t value);
            void clear() { m_buffer.clear(); };

            [[nodiscard]] uint16_t getAvarage() const;
        };

        static constexpr size_t WINDOW_SIZE = 20;
        static constexpr uint32_t DOUBLE_HIT_TIMEOUT_MS = 50;

        uint32_t m_timeout_ms;
        Buffer m_buffer{WINDOW_SIZE};

        Utils::InputState::Buttons m_prev_buttons{};
        uint32_t m_prev_press_time{0};

        uint16_t m_current_bpm{0};

      public:
        BpmCounter(uint32_t timeout_ms) : m_timeout_ms(timeout_ms) {};
        void update(const Utils::InputState::Buttons &buttons);

        [[nodiscard]] uint16_t getBpm() const { return m_current_bpm; };
    };

    Config m_config;
    State m_state{State::Idle};

    uint32_t m_touched{0};
    usb_mode_t m_usb_mode{USB_MODE_DEBUG};
    uint8_t m_player_id{0};
    bool m_auth_busy{false};

    Utils::Menu::State m_menu_state{};

    ssd1306_t m_display{};
    uint32_t m_next_frame_time{0};

    BpmCounter m_bpm_counter{2000};

    void drawIdleScreen();
    void drawMenuScreen();

  public:
    Display(const Config &config);

    void setTouched(uint32_t touched);
    void setButtons(const Utils::InputState::Buttons &buttons);
    void setUsbMode(usb_mode_t mode);
    void setPlayerId(uint8_t player_id);
    void setAuthBusyState(bool busy);

    void setMenuState(const Utils::Menu::State &menu_state);

    void showIdle();
    void showMenu();

    void update(bool force = false);
};

} // namespace Divacon::Peripherals

#endif // PERIPHERALS_DISPLAY_H_