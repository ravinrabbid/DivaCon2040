#include "usb/device/device_driver.h"
#include "usb/device/debug_driver.h"
#include "usb/device/hid_driver.h"
#include "usb/device/midi_driver.h"
#include "usb/device/pdloader_driver.h"
#include "usb/device/xinput_driver.h"

#include "bsp/board.h"
#include "pico/unique_id.h"

static usb_mode_t usbd_mode = USB_MODE_DEBUG;
static usbd_player_led_cb_t usbd_player_led_cb = NULL;
static usbd_slider_led_cb_t usbd_slider_led_cb = NULL;

// TODO make those a struct
static const tusb_desc_device_t *usbd_desc_device = NULL;
static const uint8_t *usbd_desc_cfg = NULL;
static const uint8_t *usbd_desc_hid_report = NULL;
static const uint8_t *usbd_desc_bos = NULL;
static const usbd_class_driver_t *usbd_app_driver = NULL;
static bool (*usbd_send_report)(usb_report_t report) = NULL;
static bool (*usbd_receive_report)() = NULL;
static bool (*usbd_vendor_control_xfer_cb)(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) = NULL;

#define USBD_SERIAL_STR_SIZE (PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1 + 3)
static char usbd_serial_str[USBD_SERIAL_STR_SIZE] = {};

char *const usbd_desc_str[] = {
    [USBD_STR_MANUFACTURER] = USBD_MANUFACTURER,  //
    [USBD_STR_PRODUCT] = USBD_PRODUCT,            //
    [USBD_STR_SERIAL] = usbd_serial_str,          //
    [USBD_STR_SWITCH] = USBD_SWITCH_NAME,         //
    [USBD_STR_PS3] = USBD_PS3_NAME,               //
    [USBD_STR_PS4] = USBD_PS4_NAME,               //
    [USBD_STR_XINPUT] = USBD_XINPUT_NAME,         //
    [USBD_STR_PDLOADER] = USBD_PDLOADER_NAME,     //
    [USBD_STR_KEYBOARD] = USBD_KEYBOARD_NAME,     //
    [USBD_STR_MIDI] = USBD_MIDI_NAME,             //
    [USBD_STR_CDC] = USBD_DEBUG_CDC_NAME,         //
    [USBD_STR_RPI_RESET] = USBD_DEBUG_RESET_NAME, //
};

void usb_device_driver_init(usb_mode_t mode) {
    usbd_mode = mode;

    switch (mode) {
    case USB_MODE_SWITCH_DIVACON:
        usbd_desc_device = &switch_divacon_desc_device;
        usbd_desc_cfg = switch_desc_cfg;
        usbd_desc_hid_report = switch_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_switch_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_SWITCH_HORIPAD:
        usbd_desc_device = &switch_horipad_desc_device;
        usbd_desc_cfg = switch_desc_cfg;
        usbd_desc_hid_report = switch_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_switch_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_DUALSHOCK3:
        usbd_desc_device = &ds3_desc_device;
        usbd_desc_cfg = ps3_desc_cfg;
        usbd_desc_hid_report = ps3_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_ps3_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_PS4_DIVACON:
        usbd_desc_device = &ps4_divacon_desc_device;
        usbd_desc_cfg = ps4_desc_cfg;
        usbd_desc_hid_report = ps4_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_ps4_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_DUALSHOCK4:
        usbd_desc_device = &ds4_desc_device;
        usbd_desc_cfg = ps4_desc_cfg;
        usbd_desc_hid_report = ps4_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_ps4_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_XBOX360:
        usbd_desc_device = &xinput_desc_device;
        usbd_desc_cfg = xinput_desc_cfg;
        usbd_app_driver = &xinput_app_driver;
        usbd_send_report = send_xinput_report;
        usbd_receive_report = receive_xinput_report;
        break;
    case USB_MODE_PDLOADER:
        usbd_desc_device = &pdloader_desc_device;
        usbd_desc_cfg = pdloader_desc_cfg;
        usbd_app_driver = &pdloader_app_driver;
        usbd_send_report = send_pdloader_report;
        usbd_receive_report = NULL;
        usbd_desc_bos = pdloader_desc_bos;
        usbd_vendor_control_xfer_cb = pdloader_control_xfer_cb;
        break;
    case USB_MODE_KEYBOARD:
        usbd_desc_device = &keyboard_desc_device;
        usbd_desc_cfg = keyboard_desc_cfg;
        usbd_desc_hid_report = keyboard_desc_hid_report;
        usbd_app_driver = &hid_app_driver;
        usbd_send_report = send_hid_keyboard_report;
        usbd_receive_report = NULL;
        break;
    case USB_MODE_MIDI:
        usbd_desc_device = &midi_desc_device;
        usbd_desc_cfg = midi_desc_cfg;
        usbd_app_driver = &midi_app_driver;
        usbd_send_report = send_midi_report;
        usbd_receive_report = receive_midi_report;
        break;
    case USB_MODE_DEBUG:
        usbd_desc_device = &debug_desc_device;
        usbd_desc_cfg = debug_desc_cfg;
        usbd_app_driver = &debug_app_driver;
        usbd_send_report = send_debug_report;
        usbd_receive_report = NULL;
        break;
    }

    tud_init(BOARD_TUD_RHPORT);
}

void usb_device_driver_task() { tud_task(); }

usb_mode_t usb_device_driver_get_mode() { return usbd_mode; }

void usb_device_driver_send_and_receive_report(usb_report_t report) {
    static const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) {
        return;
    }
    start_ms += interval_ms;

    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    if (usbd_send_report) {
        usbd_send_report(report);
    }

    if (usbd_receive_report) {
        usbd_receive_report();
    }
}

void usb_device_driver_set_player_led_cb(usbd_player_led_cb_t cb) { usbd_player_led_cb = cb; };

void usb_device_driver_set_slider_led_cb(usbd_slider_led_cb_t cb) { usbd_slider_led_cb = cb; };

usbd_player_led_cb_t usb_device_driver_get_player_led_cb() { return usbd_player_led_cb; };

usbd_slider_led_cb_t usb_device_driver_get_slider_led_cb() { return usbd_slider_led_cb; };

const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)usbd_desc_device; }

const uint8_t *tud_descriptor_configuration_cb(uint8_t __unused index) { return usbd_desc_cfg; }

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
#define DESC_STR_MAX (20)
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

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;

    return usbd_desc_hid_report;
}

uint8_t const *tud_descriptor_bos_cb(void) { return usbd_desc_bos; }

// Implement TinyUSB internal callback since vendor control requests are not forwarded to custom drivers.
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    if (!usbd_vendor_control_xfer_cb) {
        return false;
    }

    return usbd_vendor_control_xfer_cb(rhport, stage, request);
}

// Implement callback to add our custom driver
const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return usbd_app_driver;
}
