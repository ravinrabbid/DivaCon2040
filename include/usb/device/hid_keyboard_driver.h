#ifndef _USB_HID_KEYBOARD_DRIVER_H_
#define _USB_HID_KEYBOARD_DRIVER_H_

#include "usb/device/device_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_KEYBOARD_NAME "Keyboard Mode"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    uint8_t keycodes[32];
} hid_nkro_keyboard_report_t;

extern const tusb_desc_device_t keyboard_desc_device;
extern const uint8_t keyboard_desc_cfg[];
extern const uint8_t keyboard_desc_hid_report[];

bool send_hid_keyboard_report(usb_report_t report);
uint16_t hid_keyboard_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                                    uint16_t reqlen);
void hid_keyboard_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                                uint16_t bufsize);

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_KEYBOARD_DRIVER_H_