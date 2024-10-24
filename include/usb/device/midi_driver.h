#ifndef _USB_DEVICE_MIDI_DRIVER_H_
#define _USB_DEVICE_MIDI_DRIVER_H_

#include "usb/device_driver.h"

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

extern const usbd_driver_t midi_device_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_MIDI_DRIVER_H_