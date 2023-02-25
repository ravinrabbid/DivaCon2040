#ifndef _USB_HID_DRIVER_H_
#define _USB_HID_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_DIRECTINPUT_NAME "DirectInput Gamepad"
#define USBD_SWITCH_NAME "Switch Gamepad"

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
} directinput_report_t;

typedef struct __attribute((packed, aligned(1))) {
    uint16_t buttons;
    uint8_t hat;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t vendor;
} switch_report_t;

bool send_hid_report(usb_report_t report);

extern const tusb_desc_device_t directinput_desc_device;
extern const uint8_t directinput_desc_cfg[];
extern const uint8_t directinput_desc_hid_report[];

extern const tusb_desc_device_t switch_desc_device;
extern const uint8_t switch_desc_cfg[];
extern const uint8_t switch_desc_hid_report[];

extern const usbd_class_driver_t hid_app_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_DRIVER_H_