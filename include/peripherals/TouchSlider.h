#ifndef _PERIPHERALS_TOUCHSLIDER_H_
#define _PERIPHERALS_TOUCHSLIDER_H_

#include "utils/InputState.h"

#include "usb/device_driver.h"

#include <cap1188/Cap1188.h>
#include <is31se5117a/Is31se5117a.h>
#include <mpr121/Mpr121.h>

#include "hardware/i2c.h"

#include <array>
#include <memory>
#include <stdint.h>
#include <variant>

namespace Divacon::Peripherals {

class TouchSlider {
  public:
    struct Mpr121Config {
        uint8_t i2c_addresses[3];

        uint8_t touch_threshold;
        uint8_t release_threshold;
    };

    struct Cap1188Config {
        uint8_t i2c_addresses[4];

        uint8_t threshold;
        Cap1188::Sensitivity sensitivity;
    };

    struct Is31se5117aConfig {
        uint8_t i2c_addresses[2];

        uint8_t threshold;
        uint8_t hysteresis;
    };

    struct Config {
        uint8_t sda_pin;
        uint8_t scl_pin;
        i2c_inst_t *i2c_block;
        uint i2c_speed_hz;

        std::variant<Mpr121Config, Cap1188Config, Is31se5117aConfig> touch_config;
    };

  private:
    class TouchControllerInterface {
      public:
        virtual uint32_t read() = 0;
    };

    class TouchControllerMpr121 : public TouchControllerInterface {
      private:
        std::array<std::unique_ptr<Mpr121>, 3> m_mpr121;

      public:
        TouchControllerMpr121(const Mpr121Config &config, i2c_inst *i2c);

        virtual uint32_t read() final;
    };

    class TouchControllerCap1188 : public TouchControllerInterface {
      private:
        std::array<std::unique_ptr<Cap1188>, 4> m_cap1188;

      public:
        TouchControllerCap1188(const Cap1188Config &config, i2c_inst *i2c);

        virtual uint32_t read() final;
    };

    class TouchControllerIs31se5117a : public TouchControllerInterface {
      private:
        std::array<std::unique_ptr<Is31se5117a>, 2> m_is31se5117a;

      public:
        TouchControllerIs31se5117a(const Is31se5117aConfig &config, i2c_inst *i2c);

        virtual uint32_t read() final;
    };

  private:
    Config m_config;
    usb_mode_t m_mode;
    uint32_t m_touched;

    std::unique_ptr<TouchControllerInterface> m_touch_controller;

    void read();

    void updateInputStateArcade(Utils::InputState &input_state);
    void updateInputStateStick(Utils::InputState &input_state);

  public:
    TouchSlider(const Config &config, usb_mode_t mode);

    void updateInputState(Utils::InputState &input_state);
};

} // namespace Divacon::Peripherals

#endif // _PERIPHERALS_TOUCHSLIDER_H_