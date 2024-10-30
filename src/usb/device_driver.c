#include "usb/device_driver.h"

#include "usb/device/hid/keyboard_driver.h"
#include "usb/device/hid/ps3_driver.h"
#include "usb/device/hid/ps4_driver.h"
#include "usb/device/hid/switch_driver.h"
#include "usb/device/midi_driver.h"
#include "usb/device/vendor/debug_driver.h"
#include "usb/device/vendor/pdloader_driver.h"
#include "usb/device/vendor/xinput_driver.h"

#include "bsp/board.h"
#include "pico/unique_id.h"
#include "tusb.h"

#include <string.h>

#define DESC_STR_MAX (127)

static usb_mode_t usbd_mode = USB_MODE_DEBUG;
static usbd_driver_t usbd_driver = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static usbd_player_led_cb_t usbd_player_led_cb = NULL;
static usbd_slider_led_cb_t usbd_slider_led_cb = NULL;
static usbd_button_led_cb_t usbd_button_led_cb = NULL;

#define USBD_SERIAL_STR_SIZE (PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1 + 3)
static char usbd_serial_str[USBD_SERIAL_STR_SIZE] = {};
static char usbd_product_str[DESC_STR_MAX] = {};

char *const usbd_desc_str[] = {
    [USBD_STR_MANUFACTURER] = USBD_MANUFACTURER, //
    [USBD_STR_PRODUCT] = usbd_product_str,       //
    [USBD_STR_SERIAL] = usbd_serial_str,         //
};

void usbd_driver_init(usb_mode_t mode) {
    usbd_mode = mode;

    switch (mode) {
    case USB_MODE_SWITCH_DIVACON:
        usbd_driver = hid_switch_divacon_device_driver;
        break;
    case USB_MODE_SWITCH_HORIPAD:
        usbd_driver = hid_switch_horipad_device_driver;
        break;
    case USB_MODE_DUALSHOCK3:
        usbd_driver = hid_ds3_device_driver;
        break;
    case USB_MODE_PS4_DIVACON:
        usbd_driver = hid_ps4_divacon_device_driver;
        break;
    case USB_MODE_DUALSHOCK4:
        usbd_driver = hid_ds4_device_driver;
        break;
    case USB_MODE_XBOX360:
        usbd_driver = xinput_device_driver;
        break;
    case USB_MODE_PDLOADER:
        usbd_driver = pdloader_device_driver;
        break;
    case USB_MODE_KEYBOARD:
        usbd_driver = hid_keyboard_device_driver;
        break;
    case USB_MODE_MIDI:
        usbd_driver = midi_device_driver;
        break;
    case USB_MODE_DEBUG:
        usbd_driver = debug_device_driver;
        break;
    }

    tud_init(BOARD_TUD_RHPORT);
}

void usbd_driver_task() { tud_task(); }

usb_mode_t usbd_driver_get_mode() { return usbd_mode; }

void usbd_driver_send_report(usb_report_t report) {
    static const uint64_t interval_us = 900;
    static uint64_t start_us = 0;

    if (to_us_since_boot(get_absolute_time()) - start_us <= interval_us) {
        return;
    }
    start_us += interval_us;

    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    if (usbd_driver.send_report) {
        usbd_driver.send_report(report);
    }
}

void usbd_driver_set_player_led_cb(usbd_player_led_cb_t cb) { usbd_player_led_cb = cb; };
void usbd_driver_set_slider_led_cb(usbd_slider_led_cb_t cb) { usbd_slider_led_cb = cb; };
void usbd_driver_set_button_led_cb(usbd_button_led_cb_t cb) { usbd_button_led_cb = cb; };

usbd_player_led_cb_t usbd_driver_get_player_led_cb() { return usbd_player_led_cb; };
usbd_slider_led_cb_t usbd_driver_get_slider_led_cb() { return usbd_slider_led_cb; };
usbd_button_led_cb_t usbd_driver_get_button_led_cb() { return usbd_button_led_cb; };

const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)usbd_driver.desc_device; }

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;

    return usbd_driver.desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    static uint16_t desc_str[DESC_STR_MAX];

    // Assign the SN using the unique flash id
    if (!usbd_serial_str[0]) {
        pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
        usbd_serial_str[USBD_SERIAL_STR_SIZE - 4] = '-';
        usbd_serial_str[USBD_SERIAL_STR_SIZE - 3] = '0' + ((usbd_mode / 10) % 10);
        usbd_serial_str[USBD_SERIAL_STR_SIZE - 2] = '0' + (usbd_mode % 10);
        usbd_serial_str[USBD_SERIAL_STR_SIZE - 1] = '\0';
    }

    if (!usbd_product_str[0]) {
        strcpy(usbd_product_str, USBD_PRODUCT_BASE);
        strcat(usbd_product_str, " (");
        strcat(usbd_product_str, usbd_driver.name);
        strcat(usbd_product_str, ")");
    }

    uint8_t len;
    if (index == USBD_STR_LANGUAGE) {
        desc_str[1] = 0x0409; // Supported language is English
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
            return NULL;
        }
        const char *str = usbd_desc_str[index];
        for (len = 0; len < DESC_STR_MAX - 1 && str[len]; ++len) {
            desc_str[1 + len] = str[len];
        }
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));

    return desc_str;
}

uint8_t const *tud_descriptor_bos_cb(void) { return usbd_driver.desc_bos; }

// Implement callback to add our custom driver
const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return usbd_driver.app_driver;
}
