#include "usb/xinput_driver.h"
#include "usb/usb_driver.h"

#include "tusb.h"

#define XINPUT_OUT_SIZE 32
#define XINPUT_INTERFACE_SUBCLASS (0x5D)
#define XINPUT_INTERFACE_PROTOCOL (0x01)
#define TUD_XINPUT_DESC_LEN (39)

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

#define TUD_XINPUT_DESCRIPTOR(_itfnum, _stridx)                                                                        \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, XINPUT_INTERFACE_SUBCLASS,                      \
        XINPUT_INTERFACE_PROTOCOL, _stridx, 0x10, 0x21, 0x10, 0x01, 0x01, 0x24, 0x81, 0x14, 0x03, 0x00, 0x03, 0x13,    \
        0x01, 0x00, 0x03, 0x00, 0x07, 0x05, 0x81, 0x03, 0x20, 0x00, 0x01, 0x07, 0x05, 0x01, 0x03, 0x20, 0x00, 0x08

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_XINPUT_DESC_LEN)

const uint8_t xinput_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_XINPUT_DESCRIPTOR(USBD_ITF_XINPUT, USBD_STR_XINPUT),
};

static uint8_t endpoint_in = 0;
static uint8_t endpoint_out = 0;
static uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};

bool receive_xinput_report(void) {
    bool success = false;

    if (tud_ready() && (endpoint_out != 0) && (!usbd_edpt_busy(0, endpoint_out))) {
        usbd_edpt_claim(0, endpoint_out);                                              // Take control of OUT endpoint
        success = usbd_edpt_xfer(0, endpoint_out, xinput_out_buffer, XINPUT_OUT_SIZE); // Retrieve report buffer
        usbd_edpt_release(0, endpoint_out); // Release control of OUT endpoint
    }
    return success;
}

bool send_xinput_report(usb_report_t report) {
    bool success = false;

    if (tud_ready() &&                                          // Is the device ready?
        (endpoint_in != 0) && (!usbd_edpt_busy(0, endpoint_in)) // Is the IN endpoint available?
    ) {
        usbd_edpt_claim(0, endpoint_in);                                    // Take control of IN endpoint
        success = usbd_edpt_xfer(0, endpoint_in, report.data, report.size); // Send report buffer
        usbd_edpt_release(0, endpoint_in);                                  // Release control of IN endpoint
    }

    return success;
}

static void xinput_init(void) {}

static void xinput_reset(uint8_t rhport) { (void)rhport; }

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length) {
    uint16_t driver_length =
        sizeof(tusb_desc_interface_t) + (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t)) + 16;

    TU_VERIFY(max_length >= driver_length, 0);

    uint8_t const *current_descriptor = tu_desc_next(itf_descriptor);
    uint8_t found_endpoints = 0;
    while ((found_endpoints < itf_descriptor->bNumEndpoints) && (driver_length <= max_length)) {
        tusb_desc_endpoint_t const *endpoint_descriptor = (tusb_desc_endpoint_t const *)current_descriptor;
        if (TUSB_DESC_ENDPOINT == tu_desc_type(endpoint_descriptor)) {
            TU_ASSERT(usbd_edpt_open(rhport, endpoint_descriptor));

            if (tu_edpt_dir(endpoint_descriptor->bEndpointAddress) == TUSB_DIR_IN)
                endpoint_in = endpoint_descriptor->bEndpointAddress;
            else
                endpoint_out = endpoint_descriptor->bEndpointAddress;

            ++found_endpoints;
        }

        current_descriptor = tu_desc_next(current_descriptor);
    }
    return driver_length;
}

static bool xinput_control_xfer_callback(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    (void)rhport;
    (void)stage;
    (void)request;

    return true;
}

static bool xinput_xfer_callback(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void)rhport;
    (void)xferred_bytes;

    if (result == XFER_RESULT_SUCCESS && ep_addr == endpoint_out) {
        if (xinput_out_buffer[0] == 0x01) { // 0x00 is rumble, 0x01 is led
            usb_player_led_t player_led = {.type = USB_PLAYER_LED_ID, .id = 0};

            switch (xinput_out_buffer[2]) {
            case 0x02:
            case 0x06:
                player_led.id = 0x01;
                break;
            case 0x03:
            case 0x07:
                player_led.id = 0x02;
                break;
            case 0x04:
            case 0x08:
                player_led.id = 0x04;
                break;
            case 0x05:
            case 0x09:
                player_led.id = 0x08;
                break;
            default:
            }
            usb_driver_get_player_led_cb()(player_led);
        }
    }
    return true;
}

const usbd_class_driver_t xinput_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "XINPUT",
#endif
    .init = xinput_init,
    .reset = xinput_reset,
    .open = xinput_open,
    .control_xfer_cb = xinput_control_xfer_callback,
    .xfer_cb = xinput_xfer_callback,
    .sof = NULL};
