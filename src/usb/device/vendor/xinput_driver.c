#include "usb/device/vendor/xinput_driver.h"

#include "device/usbd_pvt.h"
#include "tusb.h"

#include <stdlib.h>

const tusb_desc_device_t xinput_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_VENDOR_SPECIFIC,
    .bDeviceSubClass = 0xFF,
    .bDeviceProtocol = 0xFF,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x045E,
    .idProduct = 0x028E,
    .bcdDevice = 0x0114,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_XINPUT,
    USBD_ITF_MAX,
};

#define XINPUT_INTERFACE_SUBCLASS 0x5D
#define XINPUT_INTERFACE_PROTOCOL 0x01
#define XINPUT_DESC_VENDOR 0x21

#define TUD_XINPUT_EP_BUFSIZE 32
#define TUD_XINPUT_EP_OUT 0x01
#define TUD_XINPUT_EP_IN 0x81

#define TUD_XINPUT_DESC_LEN (9 + 16 + 7 + 7)

#define TUD_XINPUT_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize)                                                \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, XINPUT_INTERFACE_SUBCLASS,                      \
        XINPUT_INTERFACE_PROTOCOL, _stridx, 16, XINPUT_DESC_VENDOR, 0x10, 0x01, 0x01, 0x24, 0x81, 0x14, 0x03, 0x00,    \
        0x03, 0x13, 0x01, 0x00, 0x03, 0x00, 7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), \
        1, 7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), 8

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_XINPUT_DESC_LEN)

const uint8_t xinput_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_XINPUT_DESCRIPTOR(USBD_ITF_XINPUT, 0, TUD_XINPUT_EP_OUT, TUD_XINPUT_EP_IN, TUD_XINPUT_EP_BUFSIZE),
};

typedef struct __attribute((packed, aligned(1))) {
    uint8_t type;
    uint8_t size;
    uint8_t led;
    uint8_t rumble_strong;
    uint8_t rumble_light;
    uint8_t _reserved[3];
} hid_xinput_ouput_report_t;

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[TUD_XINPUT_EP_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[TUD_XINPUT_EP_BUFSIZE];
} xinput_interface_t;

CFG_TUSB_MEM_SECTION static xinput_interface_t _xinput_itf;

static bool xinput_ready() {
    uint8_t const ep_in = _xinput_itf.ep_in;

    return tud_ready() && (ep_in != 0) && !usbd_edpt_busy(0, ep_in);
}

bool send_xinput_report(usb_report_t report) {
    if (!xinput_ready()) {
        return false;
    }

    TU_VERIFY(usbd_edpt_claim(0, _xinput_itf.ep_in));

    uint16_t size = tu_min16(report.size, TUD_XINPUT_EP_BUFSIZE);
    memcpy(_xinput_itf.epin_buf, report.data, size);

    return usbd_edpt_xfer(0, _xinput_itf.ep_in, _xinput_itf.epin_buf, size);
}

static bool receive_xinput_report(uint8_t const *buf, uint32_t size) {
    enum {
        REPORT_RUMBLE = 0x00,
        REPORT_LED = 0x01,
    };
    enum {
        ALL_OFF = 0x00,
        ALL_BLINK = 0x01,
        P1_FLASH_ON = 0x02,
        P2_FLASH_ON = 0x03,
        P3_FLASH_ON = 0x04,
        P4_FLASH_ON = 0x05,
        P1_ON = 0x06,
        P2_ON = 0x07,
        P3_ON = 0x08,
        P4_ON = 0x09,
        ALL_ROTATE = 0x0A,
        CURRENT_BLINK = 0x0B,
        CURRENT_BLINK_SLOW = 0x0C,
        ALL_ALTERNATE = 0x0D,
        ALL_SLOW_BLINK = 0x0E,
        ALL_BLINK_ONCE = 0x0F,
    };

    hid_xinput_ouput_report_t *report = (hid_xinput_ouput_report_t *)buf;

    switch (report->type) {
    case REPORT_RUMBLE:
        // Ignore, we ain't doing that
        return true;
    case REPORT_LED: {
        TU_ASSERT(size >= 3);

        usb_player_led_t player_led = {.type = USB_PLAYER_LED_ID, .id = 0};

        switch (report->led) {
        case ALL_OFF:
            player_led.id = 0x00;
            break;
        case ALL_ROTATE:
        case ALL_ALTERNATE:
            player_led.id = 0x0F;
            break;
        case P1_FLASH_ON:
        case P1_ON:
            player_led.id = 0x01;
            break;
        case P2_FLASH_ON:
        case P2_ON:
            player_led.id = 0x02;
            break;
        case P3_FLASH_ON:
        case P3_ON:
            player_led.id = 0x04;
            break;
        case P4_FLASH_ON:
        case P4_ON:
            player_led.id = 0x08;
            break;
        case ALL_SLOW_BLINK:
            player_led.id = 0x0F;
            break;
        case ALL_BLINK:
        case CURRENT_BLINK:
        case CURRENT_BLINK_SLOW:
        case ALL_BLINK_ONCE:
        default:
            player_led.id = 0x00;
            break;
        }

        usbd_driver_get_player_led_cb()(player_led);
    }
    default:
    }

    return false;
}

static void xinput_reset(uint8_t rhport) {
    (void)rhport;

    tu_memclr(&_xinput_itf, sizeof(_xinput_itf));
}

static void xinput_init(void) { xinput_reset(0); }

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == desc_itf->bInterfaceClass, 0);

    uint16_t const drv_len =
        (uint16_t)(sizeof(tusb_desc_interface_t) + desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t) + 16);
    TU_ASSERT(max_len >= drv_len, 0);

    _xinput_itf.itf_num = desc_itf->bInterfaceNumber;

    // Unknown vendor specific descriptor
    uint8_t const *p_desc = tu_desc_next(desc_itf);
    TU_ASSERT(p_desc[1] == XINPUT_DESC_VENDOR, 0);

    // Endpoint descriptors
    p_desc = tu_desc_next(p_desc);
    TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_INTERRUPT, &_xinput_itf.ep_out,
                                  &_xinput_itf.ep_in),
              0);

    if (_xinput_itf.ep_out) {
        TU_ASSERT(usbd_edpt_xfer(rhport, _xinput_itf.ep_out, _xinput_itf.epout_buf, sizeof(_xinput_itf.epout_buf)), 0);
    }

    return drv_len;
}

bool xinput_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    (void)rhport;

    if (stage != CONTROL_STAGE_SETUP)
        return true;

    // This is mainly to suppress a warning from the linux kernel:
    // https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c#L1756
    //
    // Hopefully nobody expects actual data here.
    if (!(request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR && request->bRequest == 0x01 &&
          request->wIndex == 0x00)) {
        return false;
    }

    uint8_t *dummy_data = calloc(request->wLength, sizeof(uint8_t));
    bool success = tud_control_xfer(rhport, request, (void *)(uintptr_t)dummy_data, request->wLength);
    free(dummy_data);

    return success;
}

static bool xinput_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    TU_ASSERT(result == XFER_RESULT_SUCCESS);

    if (ep_addr == _xinput_itf.ep_out) {
        receive_xinput_report(_xinput_itf.epout_buf, xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, _xinput_itf.ep_out, _xinput_itf.epout_buf, sizeof(_xinput_itf.epout_buf)));
    }

    return true;
}

static const usbd_class_driver_t xinput_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "XINPUT",
#endif
    .init = xinput_init,
    .reset = xinput_reset,
    .open = xinput_open,
    .control_xfer_cb = xinput_control_xfer_cb,
    .xfer_cb = xinput_xfer_cb,
    .sof = NULL};

const usbd_driver_t xinput_device_driver = {
    .name = "XInput",
    .app_driver = &xinput_app_driver,
    .desc_device = &xinput_desc_device,
    .desc_cfg = xinput_desc_cfg,
    .desc_bos = NULL,
    .send_report = send_xinput_report,
};
