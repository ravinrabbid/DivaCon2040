#ifndef _USB_HID_SWITCH_DRIVER_H_
#define _USB_HID_SWITCH_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_SWITCH_NAME "Switch Horipad Emulation"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    uint16_t buttons;
    uint8_t hat;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t vendor;
} hid_switch_report_t;

extern const tusb_desc_device_t switch_divacon_desc_device;
extern const tusb_desc_device_t switch_horipad_desc_device;
extern const uint8_t switch_desc_cfg[];
extern const uint8_t switch_desc_hid_report[];

bool send_hid_switch_report(usb_report_t report);
uint16_t hid_switch_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                                  uint16_t reqlen);
void hid_switch_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                              uint16_t bufsize);

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_SWITCH_DRIVER_H_