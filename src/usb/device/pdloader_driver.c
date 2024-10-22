#include "usb/device/pdloader_driver.h"

#include "tusb.h"

const tusb_desc_device_t pdloader_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0210,
    .bDeviceClass = TUSB_CLASS_VENDOR_SPECIFIC,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x0E8F,
    .idProduct = 0x2213,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_PDLOADER,
    USBD_ITF_MAX,
};

#define TUD_PDLOADER_EP_BUFSIZE 64
#define TUD_PDLOADER_EP_OUT 0x03
#define TUD_PDLOADER_EP_IN 0x84

#define TUD_PDLOADER_DESC_LEN (9 + 7 + 7)

#define TUD_PDLOADER_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize, _ep_interval)                                \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, 0x00, 0x00, _stridx, 7, TUSB_DESC_ENDPOINT,     \
        _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval, 7, TUSB_DESC_ENDPOINT, _epin,               \
        TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_PDLOADER_DESC_LEN)

const uint8_t pdloader_desc_cfg[] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_PDLOADER_DESCRIPTOR(USBD_ITF_PDLOADER, USBD_STR_PDLOADER, TUD_PDLOADER_EP_OUT, TUD_PDLOADER_EP_IN,
                            TUD_PDLOADER_EP_BUFSIZE, 1),
};

#define TUD_PDLOADER_MS_OS_20_DESC_LEN 162

const uint8_t pdloader_desc_ms_os_20[] = {
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000),
    U16_TO_U8S_LE(TUD_PDLOADER_MS_OS_20_DESC_LEN),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(TUD_PDLOADER_MS_OS_20_DESC_LEN - 0x0A - 0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
    // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
    U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), 'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I',
    0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00,
    'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    // wPropertyDataLength
    U16_TO_U8S_LE(0x0050),
    // bPropertyData: "{fe459b52-6801-44ae-b579-9e318955de9c}".
    '{', 0x00, 'F', 0x00, 'E', 0x00, '4', 0x00, '5', 0x00, '9', 0x00, 'B', 0x00, '5', 0x00, '2', 0x00, '-', 0x00, '8',
    0x00, '8', 0x00, '0', 0x00, '1', 0x00, '-', 0x00, '4', 0x00, '4', 0x00, 'A', 0x00, 'E', 0x00, '-', 0x00, 'B', 0x00,
    '5', 0x00, '7', 0x00, '9', 0x00, '-', 0x00, '9', 0x00, 'E', 0x00, '3', 0x00, '1', 0x00, '8', 0x00, '9', 0x00, '5',
    0x00, '5', 0x00, 'D', 0x00, 'E', 0x00, '9', 0x00, 'C', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00};

#define TUD_PDLOADER_BOS_VENDOR_REQUEST_MICROSOFT 1
#define TUD_PDLOADER_BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

const uint8_t pdloader_desc_bos[] = {
    TUD_BOS_DESCRIPTOR(TUD_PDLOADER_BOS_TOTAL_LEN, 1),
    TUD_BOS_MS_OS_20_DESCRIPTOR(TUD_PDLOADER_MS_OS_20_DESC_LEN, TUD_PDLOADER_BOS_VENDOR_REQUEST_MICROSOFT),
};

//--------------------------------------------------------------------+
// We implement our own device class here since tinyUSB's vendor class
// current only supports buffered bulk transfers, whereas we want
// unbuffered interrupt transfers.
//--------------------------------------------------------------------+

#define USBD_PDLOADER_READ_BUFFER_LEN 100

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_HID_EP_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_HID_EP_BUFSIZE];
} pdloader_interface_t;

CFG_TUSB_MEM_SECTION static pdloader_interface_t _pdl_itf;

bool pdloader_ready() {
    uint8_t const ep_in = _pdl_itf.ep_in;

    return tud_ready() && (ep_in != 0) && !usbd_edpt_busy(0, ep_in);
}

bool send_pdloader_report(usb_report_t report) {
    if (!pdloader_ready()) {
        return false;
    }

    TU_VERIFY(usbd_edpt_claim(0, _pdl_itf.ep_in));

    uint16_t size = tu_min16(report.size, TUD_PDLOADER_EP_BUFSIZE);
    memcpy(_pdl_itf.epin_buf, report.data, size);

    return usbd_edpt_xfer(0, _pdl_itf.ep_in, _pdl_itf.epin_buf, size);
}

// The data PDLoader sends is larger than the Fullspeed USB max size of 64.
// Therefore we need to reassemble the data from multiple packets.
bool receive_pdloader_report(uint8_t const *buf, uint32_t size) {
    static uint8_t reassemble_buffer[USBD_PDLOADER_READ_BUFFER_LEN] = {};
    static size_t reassemble_buffer_write_offset = 0;

    // Restart when we see the magic bytes.
    if (size >= 3 && buf[0] == 0x44 && buf[1] == 0x4c && buf[2] == 0x61) {
        reassemble_buffer_write_offset = 0;
    }

    // Make sure we don't overshoot our buffer and copy over new data
    uint32_t bytes_to_copy = TU_MIN(size, (sizeof(reassemble_buffer) - reassemble_buffer_write_offset));
    memcpy(&reassemble_buffer[reassemble_buffer_write_offset], buf, bytes_to_copy);

    // Remember the new offset for the next packet
    reassemble_buffer_write_offset += bytes_to_copy;

    // If the buffer is full our report is complete and we can use the data and restart
    if (reassemble_buffer_write_offset >= sizeof(reassemble_buffer)) {
        usb_player_led_t player_led = {.type = USB_PLAYER_LED_ID, .id = (uint8_t)(reassemble_buffer[3] >> 4)};
        usbd_driver_get_player_led_cb()(player_led);
        usbd_driver_get_slider_led_cb()(&reassemble_buffer[4], sizeof(reassemble_buffer) - 4);

        reassemble_buffer_write_offset = 0;
    }

    return true;
}

void pdloader_reset(uint8_t rhport) {
    (void)rhport;

    tu_memclr(&_pdl_itf, sizeof(_pdl_itf));
}

void pdloader_init(void) { pdloader_reset(0); }

uint16_t pdloader_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == desc_itf->bInterfaceClass, 0);

    uint16_t const drv_len =
        (uint16_t)(sizeof(tusb_desc_interface_t) + desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
    TU_ASSERT(max_len >= drv_len, 0);
    uint8_t const *p_desc = tu_desc_next(desc_itf);

    _pdl_itf.itf_num = desc_itf->bInterfaceNumber;

    TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_INTERRUPT, &_pdl_itf.ep_out,
                                  &_pdl_itf.ep_in),
              0);

    if (_pdl_itf.ep_out) {
        TU_ASSERT(usbd_edpt_xfer(rhport, _pdl_itf.ep_out, _pdl_itf.epout_buf, sizeof(_pdl_itf.epout_buf)), 0);
    }

    return drv_len;
}

bool pdloader_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    (void)rhport;

    if (stage != CONTROL_STAGE_SETUP)
        return true;

    if (!(request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR &&
          request->bRequest == TUD_PDLOADER_BOS_VENDOR_REQUEST_MICROSOFT && request->wIndex == 7)) {
        return false;
    }

    uint16_t total_len;
    memcpy(&total_len, pdloader_desc_ms_os_20 + 8, 2);

    return tud_control_xfer(rhport, request, (void *)(uintptr_t)pdloader_desc_ms_os_20, total_len);
}

bool pdloader_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    TU_ASSERT(result == XFER_RESULT_SUCCESS);

    if (ep_addr == _pdl_itf.ep_out) {
        receive_pdloader_report(_pdl_itf.epout_buf, xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, _pdl_itf.ep_out, _pdl_itf.epout_buf, sizeof(_pdl_itf.epout_buf)));
    }

    return true;
}

const usbd_class_driver_t pdloader_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "PDLOADER",
#endif
    .init = pdloader_init,
    .reset = pdloader_reset,
    .open = pdloader_open,
    .control_xfer_cb = pdloader_control_xfer_cb,
    .xfer_cb = pdloader_xfer_cb,
    .sof = NULL};

const usbd_driver_t pdloader_device_driver = {
    .app_driver = &pdloader_app_driver,
    .desc_device = &pdloader_desc_device,
    .desc_cfg = pdloader_desc_cfg,
    .desc_hid_report = NULL,
    .desc_bos = pdloader_desc_bos,
    .send_report = send_pdloader_report,
    .vendor_control_xfer_cb = pdloader_control_xfer_cb,
};
