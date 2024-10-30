#ifndef _USB_DEVICE_VENDOR_XINPUT_DRIVER_H_
#define _USB_DEVICE_VENDOR_XINPUT_DRIVER_H_

#include "usb/device_driver.h"

#include "tusb.h"

#include <stdint.h>

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

extern const usbd_driver_t xinput_device_driver;

bool xinput_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_VENDOR_XINPUT_DRIVER_H_