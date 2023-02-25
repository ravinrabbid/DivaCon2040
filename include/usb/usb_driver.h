#ifndef _USB_USB_DRIVER_H_
#define _USB_USB_DRIVER_H_

#include "tusb.h"

#include <stdint.h>

#define USBD_MANUFACTURER "Project DivaCon"
#define USBD_PRODUCT "DivaCon rev1"

#define USBD_MAX_POWER_MAX (500)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    USB_MODE_SWITCH_DIVACON,
    USB_MODE_SWITCH_HORIPAD,
    USB_MODE_DUALSHOCK3,
    USB_MODE_PS4_DIVACON,
    USB_MODE_DUALSHOCK4,
    USB_MODE_XBOX360,
    USB_MODE_DEBUG,
} usb_mode_t;

enum {
    USBD_STR_LANGUAGE,
    USBD_STR_MANUFACTURER,
    USBD_STR_PRODUCT,
    USBD_STR_SERIAL,
    USBD_STR_CDC,
    USBD_STR_SWITCH,
    USBD_STR_PS3,
    USBD_STR_PS4,
    USBD_STR_XINPUT,
    USBD_STR_RPI_RESET,
};

typedef struct {
    uint8_t *data;
    uint16_t size;
} usb_report_t;

extern char *const usbd_desc_str[];

typedef void (*usbd_player_led_cb_t)(uint8_t);

void usb_driver_init(usb_mode_t mode);
void usb_driver_task();

usb_mode_t usb_driver_get_mode();

void usb_driver_send_report(usb_report_t report);

void usb_driver_set_player_led_cb(usbd_player_led_cb_t cb);
usbd_player_led_cb_t usb_driver_get_player_led_cb();

#ifdef __cplusplus
}
#endif

#endif // _USB_USB_DRIVER_H_
