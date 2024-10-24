#include "usb/device/midi_driver.h"

#include "class/midi/midi_device.h"

#include "tusb.h"

const tusb_desc_device_t midi_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x1209,
    .idProduct = 0x3902,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_MIDI,
    USBD_ITF_MIDI_STREAMING,
    USBD_ITF_MAX,
};

#define EPNUM_MIDI_OUT 0x01
#define EPNUM_MIDI_IN 0x01

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

const uint8_t midi_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_MIDI_DESCRIPTOR(USBD_ITF_MIDI, 0, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), CFG_TUD_MIDI_EP_BUFSIZE),
};

static midi_report_t last_report = {};

static void write_midi_message(uint8_t status, uint8_t byte1, uint8_t byte2) {
    uint8_t midi_message[3] = {status, byte1, byte2};
    tud_midi_stream_write(0, midi_message, sizeof(midi_message));
}

static void set_note(uint8_t channel, uint8_t pitch, bool on) {
    uint8_t status = on ? (0x90 | channel) : (0x80 | channel);
    uint8_t velocity = on ? 127 : 0;
    write_midi_message(status, pitch, velocity);
}

static void set_pitchbend(uint8_t channel, uint8_t value) {
    uint8_t status = 0xE0 | channel;
    write_midi_message(status, 0, value);
}

static void set_controlchange(uint8_t channel, uint8_t controller, uint8_t value) {
    uint8_t status = 0xB0 | channel;
    write_midi_message(status, controller, value);
}

bool send_midi_report(usb_report_t report) {
    static uint8_t percussion_channel = 9;
    static uint8_t slider_channel = 0;

    midi_report_t *midi_report = (midi_report_t *)report.data;

    if (midi_report->kick != last_report.kick) {
        set_note(percussion_channel, 35, midi_report->kick);
    }
    if (midi_report->snare != last_report.snare) {
        set_note(percussion_channel, 38, midi_report->snare);
    }
    if (midi_report->hihat_closed != last_report.hihat_closed) {
        set_note(percussion_channel, 42, midi_report->hihat_closed);
    }
    if (midi_report->hihat_open != last_report.hihat_open) {
        set_note(percussion_channel, 46, midi_report->hihat_open);
    }

    if (midi_report->damper != last_report.damper) {
        set_controlchange(slider_channel, 64, midi_report->damper ? 127 : 0);
    }
    if (midi_report->portamento != last_report.portamento) {
        set_controlchange(slider_channel, 65, midi_report->portamento ? 127 : 0);
    }

    // Turn off all notes on shift change, otherwise notes
    // which are currently on won't get a off event.
    if (midi_report->shift != last_report.shift) {
        set_controlchange(slider_channel, 123, 0);
        last_report.touched = 0;
    }

    for (int i = 0; i < 32; ++i) {
        bool last_active = last_report.touched & (1 << (31 - i));
        bool active = midi_report->touched & (1 << (31 - i));

        if (active != last_active) {
            set_note(slider_channel, tu_min8(i + midi_report->shift, 127), active);
        }
    }

    if (midi_report->pitch_bend != last_report.pitch_bend) {
        set_pitchbend(slider_channel, midi_report->pitch_bend);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(midi_report_t)));

    return true;
}

void tud_midi_rx_cb(uint8_t itf) {
    (void)itf;

    // Read and discard incoming data to avoid blocking the sender
    uint8_t packet[4];
    while (tud_midi_available()) {
        tud_midi_packet_read(packet);
    }
}

const usbd_class_driver_t midi_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "MIDI",
#endif
    .init = midid_init,
    .reset = midid_reset,
    .open = midid_open,
    .control_xfer_cb = midid_control_xfer_cb,
    .xfer_cb = midid_xfer_cb,
    .sof = NULL};

const usbd_driver_t midi_device_driver = {
    .app_driver = &midi_app_driver,
    .desc_device = &midi_desc_device,
    .desc_cfg = midi_desc_cfg,
    .desc_bos = NULL,
    .send_report = send_midi_report,
};
