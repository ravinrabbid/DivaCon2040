#include "utils/InputState.h"

#include <bitset>
#include <iomanip>
#include <sstream>

namespace Divacon::Utils {

InputState::InputState()
    : dpad({false, false, false, false}),                                                                   //
      buttons({false, false, false, false, false, false, false, false, false, false, false, false, false}), //
      sticks({{AnalogStick::center, AnalogStick::center}, {AnalogStick::center, AnalogStick::center}}),     //
      touches(0), m_switch_report({}), m_ps3_report({}), m_ps4_report({}),
      m_xinput_report({0x00, sizeof(xinput_report_t), 0, 0, 0, 0, 0, 0, 0, 0, {}}),
      m_midi_report({false, false, false, false, 60, 0, 64, false, false}) {}

usb_report_t InputState::getReport(usb_mode_t mode) {
    switch (mode) {
    case USB_MODE_SWITCH_DIVACON:
    case USB_MODE_SWITCH_HORIPAD:
        return getSwitchReport();
    case USB_MODE_DUALSHOCK3:
        return getPS3InputReport();
    case USB_MODE_PS4_DIVACON:
    case USB_MODE_DUALSHOCK4:
        return getPS4InputReport();
    case USB_MODE_XBOX360:
        return getXinputReport();
    case USB_MODE_MIDI:
        return getMidiReport();
    case USB_MODE_DEBUG:
    default:
        return getDebugReport();
    }
}

InputState::InputMessage InputState::getInputMessage() { return {buttons, touches}; }

static uint8_t getHidHat(const InputState::DPad dpad) {
    if (dpad.up && dpad.right) {
        return 0x01;
    } else if (dpad.down && dpad.right) {
        return 0x03;
    } else if (dpad.down && dpad.left) {
        return 0x05;
    } else if (dpad.up && dpad.left) {
        return 0x07;
    } else if (dpad.up) {
        return 0x00;
    } else if (dpad.right) {
        return 0x02;
    } else if (dpad.down) {
        return 0x04;
    } else if (dpad.left) {
        return 0x06;
    }

    return 0x08;
}

usb_report_t InputState::getSwitchReport() {
    m_switch_report.buttons = 0                                 //
                              | (buttons.west ? (1 << 0) : 0)   //
                              | (buttons.south ? (1 << 1) : 0)  //
                              | (buttons.east ? (1 << 2) : 0)   //
                              | (buttons.north ? (1 << 3) : 0)  //
                              | (buttons.l1 ? (1 << 4) : 0)     //
                              | (buttons.r1 ? (1 << 5) : 0)     //
                              | (buttons.l2 ? (1 << 6) : 0)     //
                              | (buttons.r2 ? (1 << 7) : 0)     //
                              | (buttons.select ? (1 << 8) : 0) //
                              | (buttons.start ? (1 << 9) : 0)  //
                              | (buttons.l3 ? (1 << 10) : 0)    //
                              | (buttons.r3 ? (1 << 11) : 0)    //
                              | (buttons.home ? (1 << 12) : 0);

    m_switch_report.hat = getHidHat(dpad);

    m_switch_report.lx = sticks.left.x;
    m_switch_report.ly = sticks.left.y;
    m_switch_report.rx = sticks.right.x;
    m_switch_report.ry = sticks.right.y;

    return {(uint8_t *)&m_switch_report, sizeof(hid_switch_report_t)};
}

usb_report_t InputState::getPS3InputReport() {
    memset(&m_ps3_report, 0, sizeof(m_ps3_report));

    m_ps3_report.report_id = 0x01;

    m_ps3_report.buttons1 = 0                                 //
                            | (buttons.select ? (1 << 0) : 0) //
                            | (buttons.l3 ? (1 << 1) : 0)     //
                            | (buttons.r3 ? (1 << 2) : 0)     //
                            | (buttons.start ? (1 << 3) : 0)  //
                            | (dpad.up ? (1 << 4) : 0)        //
                            | (dpad.right ? (1 << 5) : 0)     //
                            | (dpad.down ? (1 << 6) : 0)      //
                            | (dpad.left ? (1 << 7) : 0);
    m_ps3_report.buttons2 = 0 | (buttons.l2 ? (1 << 0) : 0)  //
                            | (buttons.r2 ? (1 << 1) : 0)    //
                            | (buttons.l1 ? (1 << 2) : 0)    //
                            | (buttons.r1 ? (1 << 3) : 0)    //
                            | (buttons.north ? (1 << 4) : 0) //
                            | (buttons.east ? (1 << 5) : 0)  //
                            | (buttons.south ? (1 << 6) : 0) //
                            | (buttons.west ? (1 << 7) : 0);
    m_ps3_report.buttons3 = 0 | (buttons.home ? (1 << 0) : 0);

    m_ps3_report.lx = sticks.left.x;
    m_ps3_report.ly = sticks.left.y;
    m_ps3_report.rx = sticks.right.x;
    m_ps3_report.ry = sticks.right.y;

    m_ps3_report.lt = (buttons.l2 ? 0xff : 0);
    m_ps3_report.rt = (buttons.r2 ? 0xff : 0);

    m_ps3_report.unknown_0x02_1 = 0x02;
    m_ps3_report.battery = 0xef;
    m_ps3_report.unknown_0x12 = 0x12;

    m_ps3_report.unknown[0] = 0x12;
    m_ps3_report.unknown[1] = 0xf8;
    m_ps3_report.unknown[2] = 0x77;
    m_ps3_report.unknown[3] = 0x00;
    m_ps3_report.unknown[4] = 0x40;

    m_ps3_report.acc_x = 511;
    m_ps3_report.acc_y = 511;
    m_ps3_report.acc_z = 511;

    m_ps3_report.unknown_0x02_2 = 0x02;

    return {(uint8_t *)&m_ps3_report, sizeof(hid_ps3_report_t)};
}

usb_report_t InputState::getPS4InputReport() {
    static uint8_t report_counter = 0;

    memset(&m_ps4_report, 0, sizeof(m_ps4_report));

    m_ps4_report.report_id = 0x01;

    m_ps4_report.lx = sticks.left.x;
    m_ps4_report.ly = sticks.left.y;
    m_ps4_report.rx = sticks.right.x;
    m_ps4_report.ry = sticks.right.y;

    m_ps4_report.buttons1 = getHidHat(dpad)                  //
                            | (buttons.west ? (1 << 4) : 0)  //
                            | (buttons.south ? (1 << 5) : 0) //
                            | (buttons.east ? (1 << 6) : 0)  //
                            | (buttons.north ? (1 << 7) : 0);
    m_ps4_report.buttons2 = 0                             //
                            | (buttons.l1 ? (1 << 0) : 0) //
                            | (buttons.r1 ? (1 << 1) : 0) //
                            | (buttons.l2 ? (1 << 2) : 0) //
                            | (buttons.r2 ? (1 << 3) : 0) //
                            // | (buttons.select ? (1 << 4) : 0) // Using Touchpad click instead
                            | (buttons.start ? (1 << 5) : 0) //
                            | (buttons.l3 ? (1 << 6) : 0)    //
                            | (buttons.r3 ? (1 << 7) : 0);
    m_ps4_report.buttons3 = (report_counter << 2)           //
                            | (buttons.home ? (1 << 0) : 0) //
                            | (buttons.select ? (1 << 1) : 0);

    m_ps4_report.lt = (buttons.l2 ? 0xFF : 0);
    m_ps4_report.rt = (buttons.r2 ? 0xFF : 0);

    m_ps4_report.battery = 0 | (1 << 4) | 11; // Cable connected and fully charged
    m_ps4_report.peripheral = 0x01;
    m_ps4_report.touch_report_count = 0;

    // This method actually gets called more often than the report is sent,
    // so counters are not consecutive ... let's see if this turns out to
    // be a problem.
    report_counter++;
    if (report_counter > (UINT8_MAX >> 2)) {
        report_counter = 0;
    }

    return {(uint8_t *)&m_ps4_report, sizeof(hid_ps4_report_t)};
}

usb_report_t InputState::getXinputReport() {
    m_xinput_report.buttons1 = 0                                 //
                               | (dpad.up ? (1 << 0) : 0)        //
                               | (dpad.down ? (1 << 1) : 0)      //
                               | (dpad.left ? (1 << 2) : 0)      //
                               | (dpad.right ? (1 << 3) : 0)     //
                               | (buttons.start ? (1 << 4) : 0)  //
                               | (buttons.select ? (1 << 5) : 0) //
                               | (buttons.l3 ? (1 << 6) : 0)     //
                               | (buttons.r3 ? (1 << 7) : 0);

    m_xinput_report.buttons2 = 0                                //
                               | (buttons.l1 ? (1 << 0) : 0)    //
                               | (buttons.r1 ? (1 << 1) : 0)    //
                               | (buttons.home ? (1 << 2) : 0)  //
                               | (buttons.south ? (1 << 4) : 0) //
                               | (buttons.east ? (1 << 5) : 0)  //
                               | (buttons.west ? (1 << 6) : 0)  //
                               | (buttons.north ? (1 << 7) : 0);

    m_xinput_report.lt = (buttons.l2 ? 0xFF : 0);
    m_xinput_report.rt = (buttons.r2 ? 0xFF : 0);

    m_xinput_report.lx = static_cast<int16_t>(((sticks.left.x << 8) | sticks.left.x) + INT16_MIN);
    m_xinput_report.ly = static_cast<int16_t>(~((sticks.left.y << 8) | sticks.left.y) + INT16_MIN);
    m_xinput_report.rx = static_cast<int16_t>(((sticks.right.x << 8) | sticks.right.x) + INT16_MIN);
    m_xinput_report.ry = static_cast<int16_t>(~((sticks.right.y << 8) | sticks.right.y) + INT16_MIN);

    return {(uint8_t *)&m_xinput_report, sizeof(xinput_report_t)};
}

usb_report_t InputState::getMidiReport() {
    static bool last_shift_down = false;
    static bool last_shift_up = false;

    m_midi_report.kick = buttons.north;
    m_midi_report.snare = buttons.west;
    m_midi_report.hihat_closed = buttons.south;
    m_midi_report.hihat_open = buttons.east;

    if (!last_shift_up && dpad.right) {
        m_midi_report.shift = std::min(m_midi_report.shift + 12, 96);
    }
    if (!last_shift_down && dpad.left) {
        m_midi_report.shift = std::max(m_midi_report.shift - 12, 0);
    }
    last_shift_up = dpad.right;
    last_shift_down = dpad.left;

    if (dpad.up) {
        m_midi_report.pitch_bend = 80;
    } else if (dpad.down) {
        m_midi_report.pitch_bend = 48;
    } else {
        m_midi_report.pitch_bend = 64;
    }

    m_midi_report.touched = touches;

    m_midi_report.damper = buttons.l1 || buttons.r1;
    m_midi_report.portamento = buttons.l2 || buttons.r2;

    return {(uint8_t *)&m_midi_report, sizeof(midi_report_t)};
}

usb_report_t InputState::getDebugReport() {
    std::stringstream out;

    out << "Dpad: "                                                                   //
        << (dpad.up ? "U" : " ") << (dpad.down ? "D" : " ")                           //
        << (dpad.left ? "L" : " ") << (dpad.right ? "R" : " ") << " "                 //
        << "Buttons: "                                                                //
        << "N: " << buttons.north << " "                                              //
        << "E: " << buttons.east << " "                                               //
        << "S: " << buttons.south << " "                                              //
        << "W: " << buttons.west << " "                                               //
        << "L1: " << buttons.l1 << " "                                                //
        << "L2: " << buttons.l2 << " "                                                //
        << "L3: " << buttons.l3 << " "                                                //
        << "R1: " << buttons.r1 << " "                                                //
        << "R2: " << buttons.r2 << " "                                                //
        << "R3: " << buttons.r3 << " "                                                //
        << "START: " << buttons.start << " "                                          //
        << "SELECT: " << buttons.select << " "                                        //
        << "HOME: " << buttons.home << " "                                            //
        << "LX: " << std::setw(3) << static_cast<unsigned int>(sticks.left.x) << " "  //
        << "LY: " << std::setw(3) << static_cast<unsigned int>(sticks.left.y) << " "  //
        << "RX: " << std::setw(3) << static_cast<unsigned int>(sticks.right.x) << " " //
        << "RY: " << std::setw(3) << static_cast<unsigned int>(sticks.right.y) << " " //
        << "TOUCH: " << std::bitset<32>(touches)                                      //
        << "\r";

    m_debug_report = out.str();

    return {(uint8_t *)m_debug_report.c_str(), static_cast<uint16_t>(m_debug_report.size() + 1)};
}

bool InputState::checkHotkey() {
    static uint32_t hold_since = 0;
    static bool hold_active = false;
    static const uint32_t hold_timeout = 2000;

    if (buttons.start && buttons.select) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (!hold_active) {
            hold_active = true;
            hold_since = now;
        } else if ((now - hold_since) > hold_timeout) {
            hold_active = false;
            return true;
        }
    } else {
        hold_active = false;
    }
    return false;
}

} // namespace Divacon::Utils
