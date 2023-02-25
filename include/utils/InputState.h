#ifndef _UTILS_INPUTSTATE_H_
#define _UTILS_INPUTSTATE_H_

#include "usb/hid_driver.h"
#include "usb/usb_driver.h"
#include "usb/xinput_driver.h"

#include <stdint.h>
#include <string>

namespace Divacon::Utils {

struct InputState {
  public:
    struct DPad {
        bool up, down, left, right;
    };

    struct Buttons {
        bool north, east, south, west;
        bool l1, l2, l3;
        bool r1, r2, r3;
        bool start, select, home;
    };

    struct AnalogStick {
        const static uint8_t center = 0x80;

        uint8_t x, y;
    };

    struct InputMessage {
        Buttons buttons;
        uint32_t touches;
    };

  public:
    DPad dpad;
    Buttons buttons;
    struct {
        AnalogStick left = {AnalogStick::center, AnalogStick::center};
        AnalogStick right = {AnalogStick::center, AnalogStick::center};
    } sticks;
    uint32_t touches;

  private:
    hid_switch_report_t m_switch_report;
    hid_ps3_report_t m_ps3_report;
    hid_ps4_report_t m_ps4_report;
    xinput_report_t m_xinput_report;
    std::string m_debug_report;

    usb_report_t getSwitchReport();
    usb_report_t getPS3InputReport();
    usb_report_t getPS4InputReport();
    usb_report_t getXinputReport();
    usb_report_t getDebugReport();

  public:
    InputState();

    usb_report_t getReport(usb_mode_t mode);
    InputMessage getInputMessage();

    bool checkHotkey();
};

} // namespace Divacon::Utils

#endif // _UTILS_INPUTSTATE_H_