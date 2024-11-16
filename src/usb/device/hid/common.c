#include "usb/device/hid/common.h"

#include "usb/device/hid/keyboard_driver.h"
#include "usb/device/hid/ps3_driver.h"
#include "usb/device/hid/ps4_driver.h"
#include "usb/device/hid/switch_driver.h"
#include "usb/device_driver.h"

#include "class/hid/hid_device.h"
#include "tusb.h"

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    switch (usbd_driver_get_mode()) {
    case USB_MODE_SWITCH_DIVACON:
    case USB_MODE_SWITCH_HORIPAD:
        return hid_switch_get_report_cb(itf, report_id, report_type, buffer, reqlen);
    case USB_MODE_DUALSHOCK3:
        return hid_ps3_get_report_cb(itf, report_id, report_type, buffer, reqlen);
    case USB_MODE_PS4_DIVACON:
    case USB_MODE_PS4_COMPAT:
    case USB_MODE_DUALSHOCK4:
        return hid_ps4_get_report_cb(itf, report_id, report_type, buffer, reqlen);
    case USB_MODE_KEYBOARD:
        return hid_keyboard_get_report_cb(itf, report_id, report_type, buffer, reqlen);
    default:
    }

    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    switch (usbd_driver_get_mode()) {
    case USB_MODE_SWITCH_DIVACON:
    case USB_MODE_SWITCH_HORIPAD:
        hid_switch_set_report_cb(itf, report_id, report_type, buffer, bufsize);
        break;
    case USB_MODE_DUALSHOCK3:
        hid_ps3_set_report_cb(itf, report_id, report_type, buffer, bufsize);
        break;
    case USB_MODE_PS4_DIVACON:
    case USB_MODE_PS4_COMPAT:
    case USB_MODE_DUALSHOCK4:
        hid_ps4_set_report_cb(itf, report_id, report_type, buffer, bufsize);
        break;
    case USB_MODE_KEYBOARD:
        hid_keyboard_set_report_cb(itf, report_id, report_type, buffer, bufsize);
        break;
    default:
    }
}

bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    // Magic byte sequence to enable PS button on PS3
    static const uint8_t magic_init_bytes[8] = {0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

    if (stage == CONTROL_STAGE_SETUP && request->bmRequestType == 0xA1 &&
        request->bRequest == HID_REQ_CONTROL_GET_REPORT && request->wValue == 0x0300) {
        return tud_hid_report(0, magic_init_bytes, sizeof(magic_init_bytes));
    } else {
        return hidd_control_xfer_cb(rhport, stage, request);
    }
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;

    switch (usbd_driver_get_mode()) {
    case USB_MODE_SWITCH_DIVACON:
    case USB_MODE_SWITCH_HORIPAD:
        return switch_desc_hid_report;
    case USB_MODE_DUALSHOCK3:
        return ps3_desc_hid_report;
    case USB_MODE_PS4_DIVACON:
    case USB_MODE_PS4_COMPAT:
    case USB_MODE_DUALSHOCK4:
        return ps4_desc_hid_report;
    case USB_MODE_KEYBOARD:
        return keyboard_desc_hid_report;
    default:
    }

    return NULL;
}

const usbd_class_driver_t hid_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "HID",
#endif
    .init = hidd_init,
    .reset = hidd_reset,
    .open = hidd_open,
    .control_xfer_cb = hid_control_xfer_cb,
    .xfer_cb = hidd_xfer_cb,
    .sof = NULL};
