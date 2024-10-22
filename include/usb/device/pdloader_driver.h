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

extern const usbd_driver_t pdloader_device_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_PDLOADER_DRIVER_H_