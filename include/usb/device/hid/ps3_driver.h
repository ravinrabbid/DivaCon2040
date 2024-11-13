#ifndef _USB_DEVICE_HID_PS3_DRIVER_H_
#define _USB_DEVICE_HID_PS3_DRIVER_H_

#include "usb/device_driver.h"

#include "class/hid/hid_device.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t padding;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t buttons3;
    uint8_t padding1;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t padding2[8];
    uint8_t lt;
    uint8_t rt;
    uint8_t padding3[9];
    uint8_t unknown_0x02_1;
    uint8_t battery;
    uint8_t unknown_0x12;
    uint8_t padding4[4];
    uint8_t unknown[5];
    uint16_t acc_x;
    uint16_t acc_z;
    uint16_t acc_y;
    uint8_t padding5;
    uint8_t unknown_0x02_2;
} hid_ps3_report_t;

extern const usbd_driver_t hid_ds3_device_driver;

extern const uint8_t ps3_desc_hid_report[];

uint16_t hid_ps3_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen);
void hid_ps3_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize);

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_HID_PS3_DRIVER_H_