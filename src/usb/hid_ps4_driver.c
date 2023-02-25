#include "usb/hid_ps4_driver.h"
#include "usb/usb_driver.h"

#include "class/hid/hid_device.h"

#include "tusb.h"

const tusb_desc_device_t ps4_divacon_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x0F0D,  // HORI
    .idProduct = 0x013C, // PS4-161 aka Project Diva Arcade Controller
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

const tusb_desc_device_t ds4_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x054C,  // Sny
    .idProduct = 0x05C4, // Dualshock 4
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_HID,
    USBD_ITF_MAX,
};

#define USBD_PS4_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
const uint8_t ps4_desc_cfg[] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_PS4_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_HID_INOUT_DESCRIPTOR(USBD_ITF_HID, USBD_STR_PS4, 0, 160, 0x03, 0x84, 64, 1),
};

const uint8_t ps4_desc_hid_report[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,       // Usage (Game Pad)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x32,       //   Usage (Z)
    0x09, 0x35,       //   Usage (Rz)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x04,       //   Report Count (4)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x07,       //   Logical Maximum (7)
    0x35, 0x00,       //   Physical Minimum (0)
    0x46, 0x3B, 0x01, //   Physical Maximum (315)
    0x65, 0x14,       //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,       //   Report Size (4)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,       //   Unit (None)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (0x01)
    0x29, 0x0E,       //   Usage Maximum (0x0E)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x0E,       //   Report Count (14)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,       //   Usage (0x20)
    0x75, 0x06,       //   Report Size (6)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,       //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,       //   Usage (Rx)
    0x09, 0x34,       //   Usage (Ry)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,       //   Usage (0x21)
    0x95, 0x36,       //   Report Count (54)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x05,       //   Report ID (5)
    0x09, 0x22,       //   Usage (0x22)
    0x95, 0x1F,       //   Report Count (31)
    0x91, 0x02,       //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x03,       //   Report ID (3)
    0x0A, 0x21, 0x27, //   Usage (0x2721)
    0x95, 0x2F,       //   Report Count (47)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             // End Collection
    0x06, 0xF0, 0xFF, // Usage Page (Vendor Defined 0xFFF0)
    0x09, 0x40,       // Usage (0x40)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0xF0,       //   Report ID (-16)
    0x09, 0x47,       //   Usage (0x47)
    0x95, 0x3F,       //   Report Count (63)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF1,       //   Report ID (-15)
    0x09, 0x48,       //   Usage (0x48)
    0x95, 0x3F,       //   Report Count (63)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF2,       //   Report ID (-14)
    0x09, 0x49,       //   Usage (0x49)
    0x95, 0x0F,       //   Report Count (15)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF3,       //   Report ID (-13)
    0x0A, 0x01, 0x47, //   Usage (0x4701)
    0x95, 0x07,       //   Report Count (7)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             // End Collection
};

// Unknown, copied from an actual device.
// static const uint8_t ps4_0x03_report[] = {0x3, 0x21, 0x27, 0x4, 0x40, 0x7, 0x2c, 0x56, 0x0, 0x0, 0x0, 0x0,
//                                           0x0, 0x0,  0x0,  0x0, 0x0,  0x0, 0xd,  0xd,  0x0, 0x0, 0x0, 0x0,
//                                           0x0, 0x0,  0x0,  0x0, 0x0,  0x0, 0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
//                                           0x0, 0x0,  0x0,  0x0, 0x0,  0x0, 0x0,  0x0,  0x0, 0x0, 0x0, 0x0};
// static const uint8_t ps4_0xf3_report[] = {0xf3, 0x0, 0x38, 0x38, 0, 0, 0, 0};
static const uint8_t ps4_0x81_report[] = {0,    0xB0, 0x05, 0x94,
                                          0x17, 0x9E, 0xAC}; // MAC address // TODO generate from ID?
static const uint8_t ps4_0xa3_report[] = {0xA3, 0x41, 0x75, 0x67, 0x20, 0x20, 0x33, 0x20, 0x32, 0x30, 0x31, 0x33, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x30, 0x37, 0x3A, 0x30, 0x31, 0x3A, 0x31, 0x32, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x31, 0x03, 0x00,
                                          0x00, 0x00, 0x49, 0x00, 0x05, 0x00, 0x00, 0x80, 0x03, 0x00};
// static const uint8_t ps4_0x02_report[] = {0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x87, 0x22, 0x7B, 0xDD, 0xB2,
// 0x22,
//                                           0x47, 0xDD, 0xBD, 0x22, 0x43, 0xDD, 0x1C, 0x02, 0x1C, 0x02, 0x7F, 0x1E,
//                                           0x2E, 0xDF, 0x60, 0x1F, 0x4C, 0xE0, 0x3A, 0x1D, 0xC6, 0xDE, 0x08, 0x00};

static const struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    int16_t gyro_pitch_bias;
    int16_t gyro_yaw_bias;
    int16_t gyro_roll_bias;
    int16_t gyro_pitch_plus;
    int16_t gyro_pitch_minus;
    int16_t gyro_yaw_plus;
    int16_t gyro_yaw_minus;
    int16_t gyro_roll_plus;
    int16_t gyro_roll_minus;
    int16_t gyro_speed_plus;
    int16_t gyro_speed_minus;
    int16_t acc_x_plus;
    int16_t acc_x_minus;
    int16_t acc_y_plus;
    int16_t acc_y_minus;
    int16_t acc_z_plus;
    int16_t acc_z_minus;
    uint8_t unknown[2];
} ps4_calibration_data = {
    .report_id = 0x02,
    .gyro_pitch_bias = 0,
    .gyro_yaw_bias = 0,
    .gyro_roll_bias = 0,
    .gyro_pitch_plus = INT16_MAX,
    .gyro_pitch_minus = 0,
    .gyro_yaw_plus = INT16_MAX,
    .gyro_yaw_minus = 0,
    .gyro_roll_plus = INT16_MAX,
    .gyro_roll_minus = 0,
    .gyro_speed_plus = 2048,
    .gyro_speed_minus = 0,
    .acc_x_plus = INT16_MAX,
    .acc_x_minus = 0,
    .acc_y_plus = INT16_MAX,
    .acc_y_minus = 0,
    .acc_z_plus = INT16_MAX,
    .acc_z_minus = 0,
    .unknown = {0x08, 0x00},
};

static hid_ps4_report_t last_report = {};

bool send_hid_ps4_report(usb_report_t report) {
    bool result = false;
    if (tud_hid_ready()) {
        result = tud_hid_report(0, report.data, report.size);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(hid_ps4_report_t)));

    return result;
}

uint16_t hid_ps4_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)itf;
    // (void)report_id;
    (void)report_type;
    // (void)buffer;
    (void)reqlen;

    if (report_type == HID_REPORT_TYPE_INPUT) {
        memcpy(&buffer, &last_report, sizeof(hid_ps4_report_t));
        return sizeof(hid_ps4_report_t);
    } else if (report_type == HID_REPORT_TYPE_FEATURE) {
        // if (report_id == 0x03) {
        //     memcpy(&buffer, &ps4_0x03_report, sizeof(ps4_0x03_report));
        //     return sizeof(ps4_0x03_report);
        // } else if (report_id == 0xF3) {
        //     memcpy(&buffer, &ps4_0xf3_report, sizeof(ps4_0xf3_report));
        //     return sizeof(ps4_0xf3_report);
        // } else
        if (report_id == 0x81) {
            memcpy(&buffer, &ps4_0x81_report, sizeof(ps4_0x81_report));
            return sizeof(ps4_0x81_report);
        } else if (report_id == 0xa3) {
            memcpy(&buffer, &ps4_0xa3_report, sizeof(ps4_0xa3_report));
            return sizeof(ps4_0xa3_report);
        } else if (report_id == 0x02) {
            memcpy(&buffer, &ps4_calibration_data, sizeof(ps4_calibration_data));
            return sizeof(ps4_calibration_data);
        }
    }

    return 0;
}

void hid_ps4_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)bufsize;
    (void)buffer;
}
