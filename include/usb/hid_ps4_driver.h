#ifndef _USB_HID_PS4_DRIVER_H_
#define _USB_HID_PS4_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_PS4_NAME "Dualshock4 Emulation"

#ifdef __cplusplus
extern "C" {
#endif

// https://github.com/torvalds/linux/blob/master/drivers/hid/hid-playstation.c
typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t buttons3;
    uint8_t lt;
    uint8_t rt;
    uint16_t sensor_timestamp;
    uint8_t sensor_temperature;
    uint16_t gyrox;
    uint16_t gyroy;
    uint16_t gyroz;
    int16_t accelx;
    int16_t accely;
    int16_t accelz;
    uint8_t _reserved1[5];
    uint8_t battery;
    uint8_t peripheral;
    uint8_t _reserved2;
    uint8_t touch_report_count;
    uint8_t touch_report1[9];
    uint8_t touch_report2[9];
    uint8_t touch_report3[9];
    uint8_t _reserved3[3];
} hid_ps4_report_t;

extern const tusb_desc_device_t ps4_divacon_desc_device;
extern const tusb_desc_device_t ds4_desc_device;
extern const uint8_t ps4_desc_cfg[];
extern const uint8_t ps4_desc_hid_report[];

bool send_hid_ps4_report(usb_report_t report);
uint16_t hid_ps4_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen);
void hid_ps4_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize);

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_PS4_DRIVER_H_