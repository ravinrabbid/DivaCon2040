#include "usb/device/hid/keyboard_driver.h"

#include "usb/device/hid/common.h"

#include "tusb.h"

const tusb_desc_device_t keyboard_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x1209,
    .idProduct = 0x3901,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 0x01,
};

enum {
    USBD_ITF_HID,
    USBD_ITF_MAX,
};

const uint8_t keyboard_desc_hid_report[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x05, 0x07,       //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,       //   Usage Minimum (0x00)
    0x2A, 0xFF, 0x00, //   Usage Maximum (0xFF)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x96, 0x00, 0x01, //   Report Count (256)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,             // End Collection
};

#define USBD_KEYBOARD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
uint8_t const keyboard_desc_cfg[] = {
    TUD_CONFIG_DESCRIPTOR(0x01, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_KEYBOARD_DESC_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MAX),
    TUD_HID_DESCRIPTOR(USBD_ITF_HID, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(keyboard_desc_hid_report), 0x81,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
};

static hid_keyboard_report_t last_report = {};

bool send_hid_keyboard_report(usb_report_t report) {
    bool result = false;

    if (tud_hid_ready()) {
        result = tud_hid_report(0x01, report.data, report.size);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(hid_nkro_keyboard_report_t)));

    return result;
}

uint16_t hid_keyboard_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                                    uint16_t reqlen) {
    (void)itf;
    (void)report_id;
    (void)reqlen;

    if (report_type == HID_REPORT_TYPE_INPUT) {
        memcpy(buffer, &last_report, sizeof(hid_keyboard_report_t));
        return sizeof(hid_keyboard_report_t);
    }
    return 0;
}

void hid_keyboard_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                                uint16_t bufsize) {
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

const usbd_driver_t hid_keyboard_device_driver = {
    .name = "Keyboard",
    .app_driver = &hid_app_driver,
    .desc_device = &keyboard_desc_device,
    .desc_cfg = keyboard_desc_cfg,
    .desc_bos = NULL,
    .send_report = send_hid_keyboard_report,
};