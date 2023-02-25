#ifndef _USB_XINPUT_DRIVER_H_
#define _USB_XINPUT_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_XINPUT_NAME "XInput Gamepad"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t report_size;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t lt;
    uint8_t rt;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
    uint8_t _reserved[6];
} xinput_report_t;

bool receive_xinput_report(void);
bool send_xinput_report(usb_report_t report);

extern const tusb_desc_device_t xinput_desc_device;
extern const uint8_t xinput_desc_cfg[];
extern const usbd_class_driver_t xinput_app_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_XINPUT_DRIVER_H_