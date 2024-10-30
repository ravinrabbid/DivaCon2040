#include "usb/device/vendor/debug_driver.h"

#include "device/usbd_pvt.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdio_usb.h"
#include "pico/usb_reset_interface.h"
#include "tusb.h"

#define USBD_CDC_EP_CMD (0x81)
#define USBD_CDC_EP_OUT (0x02)
#define USBD_CDC_EP_IN (0x82)
#define USBD_CDC_CMD_MAX_SIZE (8)
#define USBD_CDC_IN_OUT_MAX_SIZE (64)

const tusb_desc_device_t debug_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0210,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x2E8A,  // Raspberry Pi
    .idProduct = 0x000A, // stdio_usb to allow reset via picotool
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_CDC,
    USBD_ITF_CDC_DATA,
    USBD_ITF_RPI_RESET,
    USBD_ITF_MAX,
};

#define TUD_RPI_RESET_DESC_LEN 9
#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_RPI_RESET_DESC_LEN)
#define TUD_RPI_RESET_DESCRIPTOR(_itfnum, _stridx)                                                                     \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, RESET_INTERFACE_SUBCLASS,                       \
        RESET_INTERFACE_PROTOCOL, _stridx

const uint8_t debug_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, 0, USBD_CDC_EP_CMD, USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN,
                       USBD_CDC_IN_OUT_MAX_SIZE),
    TUD_RPI_RESET_DESCRIPTOR(USBD_ITF_RPI_RESET, 0),
};

#define TUD_DEBUG_MS_OS_20_DESC_LEN 166

static const uint8_t debug_desc_ms_os_20[] = {
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000),
    U16_TO_U8S_LE(TUD_DEBUG_MS_OS_20_DESC_LEN),

    // Function Subset header: length, type, first interface, reserved, subset length
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), USBD_ITF_RPI_RESET, 0, U16_TO_U8S_LE(0x009C),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(0x0080), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY), U16_TO_U8S_LE(0x0001),
    U16_TO_U8S_LE(0x0028), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUID" in UTF-16
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 'r',
    0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 0x00, 0x00,
    U16_TO_U8S_LE(0x004E), // wPropertyDataLength
                           // Vendor-defined Property Data: {bc7398c1-73cd-4cb7-98b8-913a8fca7bf6}
    '{', 0, 'b', 0, 'c', 0, '7', 0, '3', 0, '9', 0, '8', 0, 'c', 0, '1', 0, '-', 0, '7', 0, '3', 0, 'c', 0, 'd', 0, '-',
    0, '4', 0, 'c', 0, 'b', 0, '7', 0, '-', 0, '9', 0, '8', 0, 'b', 0, '8', 0, '-', 0, '9', 0, '1', 0, '3', 0, 'a', 0,
    '8', 0, 'f', 0, 'c', 0, 'a', 0, '7', 0, 'b', 0, 'f', 0, '6', 0, '}', 0, 0, 0};

#define TUD_PDLOADER_BOS_VENDOR_REQUEST_MICROSOFT 1
#define TUD_DEBUG_BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

const uint8_t debug_desc_bos[] = {
    TUD_BOS_DESCRIPTOR(TUD_DEBUG_BOS_TOTAL_LEN, 1),
    TUD_BOS_MS_OS_20_DESCRIPTOR(TUD_DEBUG_MS_OS_20_DESC_LEN, TUD_PDLOADER_BOS_VENDOR_REQUEST_MICROSOFT),
};

static uint8_t itf_num;

bool send_debug_report(usb_report_t report) {
    stdio_printf((char *)report.data);
    stdio_flush();

    return true;
}

static void debug_init(void) {}

static void debug_reset(uint8_t rhport) {
    (void)rhport;
    itf_num = 0;
}

static uint16_t debug_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    (void)rhport;

    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
                  RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
                  RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol,
              0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len, 0);

    itf_num = itf_desc->bInterfaceNumber;
    return drv_len;
}

// Support for parameterized reset via vendor interface control request
bool debug_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    // nothing to do with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP)
        return true;

    if (request->bRequest == 1 && request->wIndex == 7) {
        // Get Microsoft OS 2.0 compatible descriptor
        return tud_control_xfer(rhport, request, (void *)(uintptr_t)debug_desc_ms_os_20, sizeof(debug_desc_ms_os_20));
    } else if (request->wIndex == itf_num) {
        if (request->bRequest == RESET_REQUEST_BOOTSEL) {
            reset_usb_boot(0, (request->wValue & 0x7f) | PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
            // does not return, otherwise we'd return true
        }

        if (request->bRequest == RESET_REQUEST_FLASH) {
            watchdog_reboot(0, 0, PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS);
            return true;
        }
    }

    return false;
}

static bool debug_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void)rhport;
    (void)ep_addr;
    (void)result;
    (void)xferred_bytes;

    return true;
}

static usbd_class_driver_t const debug_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "DEBUG",
#endif
    .init = debug_init,
    .reset = debug_reset,
    .open = debug_open,
    .control_xfer_cb = debug_control_xfer_cb,
    .xfer_cb = debug_xfer_cb,
    .sof = NULL};

const usbd_driver_t debug_device_driver = {
    .name = "Debug",
    .app_driver = &debug_app_driver,
    .desc_device = &debug_desc_device,
    .desc_cfg = debug_desc_cfg,
    .desc_bos = debug_desc_bos,
    .send_report = send_debug_report,
};

// Support for default BOOTSEL reset by changing baud rate
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding) {
    (void)itf;

    if (p_line_coding->bit_rate == PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE) {
        reset_usb_boot(0, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
    }
}
