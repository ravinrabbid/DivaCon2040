#ifndef _USB_HID_PS4_DRIVER_H_
#define _USB_HID_PS4_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_PS4_NAME "Dualshock4 Emulation"

#ifdef __cplusplus
extern "C" {
#endif

// https://www.psdevwiki.com/ps4/DS4-USB
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
    uint16_t timestamp;
    uint8_t battery;
    uint16_t gyrox;
    uint16_t gyroy;
    uint16_t gyroz;
    int16_t accelx;
    int16_t accely;
    int16_t accelz;
    uint8_t unknown1[5];
    uint8_t extension;
    uint8_t unknown2[2];
    uint8_t touchpad_event_active;
    uint8_t touchpad_counter;
    uint8_t touchpad1_touches;
    uint8_t touchpad1_position[3];
    uint8_t touchpad2_touches;
    uint8_t touchpad2_position[3];
    uint8_t unknown3[21];
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