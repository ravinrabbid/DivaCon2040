#ifndef _USB_HID_DRIVER_H_
#define _USB_HID_DRIVER_H_

#include "usb/device/hid_ps3_driver.h"
#include "usb/device/hid_ps4_driver.h"
#include "usb/device/hid_switch_driver.h"
#include "usb/device/device_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const usbd_class_driver_t hid_app_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_DRIVER_H_