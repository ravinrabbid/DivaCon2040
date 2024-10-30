#ifndef _USB_DEVICE_DRIVER_H_
#define _USB_DEVICE_DRIVER_H_

#include "device/usbd_pvt.h"

#include <stdint.h>

#define USBD_MANUFACTURER "DivaCon2040"
#define USBD_PRODUCT_BASE "Project Diva Controller"

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
    USB_MODE_PDLOADER,
    USB_MODE_KEYBOARD,
    USB_MODE_MIDI,
    USB_MODE_DEBUG,
} usb_mode_t;

enum {
    USBD_STR_LANGUAGE,
    USBD_STR_MANUFACTURER,
    USBD_STR_PRODUCT,
    USBD_STR_SERIAL,
};

typedef struct {
    uint8_t *data;
    uint16_t size;
} usb_report_t;

typedef struct {
    const char *name;
    const usbd_class_driver_t *app_driver;
    // Descriptors
    const tusb_desc_device_t *desc_device;
    const uint8_t *desc_cfg;
    const uint8_t *desc_hid_report;
    const uint8_t *desc_bos;
    // Callbacks
    bool (*send_report)(usb_report_t report);
} usbd_driver_t;

typedef enum {
    USB_PLAYER_LED_ID,
    USB_PLAYER_LED_COLOR,
} usb_player_led_type_t;

typedef struct {
    usb_player_led_type_t type;
    union {
        uint8_t id;
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
    };
} usb_player_led_t;

typedef struct {
    bool north;
    bool east;
    bool south;
    bool west;
} usb_button_led_t;

extern char *const usbd_desc_str[];

typedef void (*usbd_player_led_cb_t)(usb_player_led_t);
typedef void (*usbd_slider_led_cb_t)(const uint8_t *, size_t);
typedef void (*usbd_button_led_cb_t)(usb_button_led_t);

void usbd_driver_init(usb_mode_t mode);
void usbd_driver_task();

usb_mode_t usbd_driver_get_mode();

void usbd_driver_send_report(usb_report_t report);

void usbd_driver_set_player_led_cb(usbd_player_led_cb_t cb);
usbd_player_led_cb_t usbd_driver_get_player_led_cb();

void usbd_driver_set_slider_led_cb(usbd_slider_led_cb_t cb);
usbd_slider_led_cb_t usbd_driver_get_slider_led_cb();

void usbd_driver_set_button_led_cb(usbd_button_led_cb_t cb);
usbd_button_led_cb_t usbd_driver_get_button_led_cb();

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_DRIVER_H_
