#ifndef _USB_DEBUG_DRIVER_H_
#define _USB_DEBUG_DRIVER_H_

#include "usb/device/device_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_DEBUG_CDC_NAME "Serial Debug"
#define USBD_DEBUG_RESET_NAME "Picotool Reset"

#ifdef __cplusplus
extern "C" {
#endif

bool send_debug_report(usb_report_t report);

extern const tusb_desc_device_t debug_desc_device;
extern const uint8_t debug_desc_cfg[];
extern const usbd_class_driver_t debug_app_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_DEBUG_DRIVER_H_