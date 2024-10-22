#ifndef _USB_PDLOADER_DRIVER_H_
#define _USB_PDLOADER_DRIVER_H_

#include "usb/device/device_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_PDLOADER_NAME "PD-Loader Arcade Controller"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    uint8_t vendor[3];
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t buttons3_slider1;
    uint8_t slider2;
    uint8_t slider3;
    uint8_t slider4;
    uint8_t slider5;
    uint8_t _unknown[14];
} pdloader_report_t;

bool send_pdloader_report(usb_report_t report);
bool pdloader_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);

extern const tusb_desc_device_t pdloader_desc_device;
extern const uint8_t pdloader_desc_cfg[];
extern const usbd_class_driver_t pdloader_app_driver;

extern const uint8_t pdloader_desc_bos[];

#ifdef __cplusplus
}
#endif

#endif // _USB_PDLOADER_DRIVER_H_