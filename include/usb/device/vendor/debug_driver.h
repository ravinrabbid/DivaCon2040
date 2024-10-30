#ifndef _USB_DEVICE_VENDOR_DEBUG_DRIVER_H_
#define _USB_DEVICE_VENDOR_DEBUG_DRIVER_H_

#include "usb/device_driver.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const usbd_driver_t debug_device_driver;

bool debug_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_VENDOR_DEBUG_DRIVER_H_