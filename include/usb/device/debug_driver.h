#ifndef _USB_DEVICE_DEBUG_DRIVER_H_
#define _USB_DEVICE_DEBUG_DRIVER_H_

#include "usb/device_driver.h"

#define USBD_DEBUG_CDC_NAME "Serial Debug"
#define USBD_DEBUG_RESET_NAME "Picotool Reset"

#ifdef __cplusplus
extern "C" {
#endif

extern const usbd_driver_t debug_device_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_DEBUG_DRIVER_H_