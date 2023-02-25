#ifndef _USB_HID_PS3_DRIVER_H_
#define _USB_HID_PS3_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_PS3_NAME "Dualshock3 Emulation"

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
    // uint16_t buttons;
    // uint8_t hat;
    // uint8_t lx;
    // uint8_t ly;
    // uint8_t rx;
    // uint8_t ry;
    // uint8_t padding_3[13];
    // uint8_t two_1;
    // uint8_t padding_4;
    // uint8_t two_2;
    // uint8_t padding_5;
    // uint8_t two_3;
    // uint8_t padding_6;
    // uint8_t two_4;
} hid_ps3_report_t;

// uint8_t a = sizeof(hid_ps3_report_t);

extern const tusb_desc_device_t ds3_desc_device;
extern const uint8_t ps3_desc_cfg[];
extern const uint8_t ps3_desc_hid_report[];

bool send_hid_ps3_report(usb_report_t report);
uint16_t hid_ps3_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen);
void hid_ps3_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize);

#ifdef __cplusplus
}
#endif

#endif // _USB_HID_PS3_DRIVER_H_