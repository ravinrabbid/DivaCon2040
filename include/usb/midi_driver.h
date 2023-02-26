#ifndef _USB_MIDI_DRIVER_H_
#define _USB_MIDI_DRIVER_H_

#include "usb/usb_driver.h"

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_MIDI_NAME "MIDI Controller"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    bool kick, snare, hihat_closed, hihat_open;
    uint8_t shift;
    uint32_t touched;
    uint8_t pitch_bend;
    bool damper, portamento;
} midi_report_t;

bool receive_midi_report(void);
bool send_midi_report(usb_report_t report);

extern const tusb_desc_device_t midi_desc_device;
extern const uint8_t midi_desc_cfg[];
extern const usbd_class_driver_t midi_app_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_MIDI_DRIVER_H_