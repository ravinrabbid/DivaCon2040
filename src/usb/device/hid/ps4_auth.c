#include "usb/device/hid/ps4_auth.h"

#include "usb/device_driver.h"

#include "mbedtls/pk.h"

#include <string.h>

#define CHALLENGE_CHUNK_LENGTH (56)

typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t challenge_id;
    uint8_t challenge_seq;
    uint8_t _unknown_0x00;
    uint8_t challenge_data[CHALLENGE_CHUNK_LENGTH];
    uint32_t checksum;
} hid_ps4_challenge_report_t;

typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t challenge_id;
    uint8_t signing_state;
    uint8_t _unknown_0x00[9];
    uint32_t checksum;
} hid_ps4_challenge_state_report_t;

typedef struct __attribute((packed, aligned(1))) {
    uint8_t signature[PS4_AUTH_SIGNATURE_LENGTH];
    uint8_t serial[PS4_AUTH_SERIAL_LENGTH];
    uint8_t key_n[PS4_AUTH_SIGNATURE_LENGTH];
    uint8_t key_e[PS4_AUTH_SIGNATURE_LENGTH];
    uint8_t ca_signature[PS4_AUTH_SIGNATURE_LENGTH];
} ps4_auth_challenge_response_t;

typedef struct {
    bool initialized;
    uint8_t challenge_id;
    uint8_t challenge_data[PS4_AUTH_CHALLENGE_LENGTH];
    ps4_auth_challenge_response_t challenge_response;
    uint8_t challenge_response_read_seq;
    bool challenge_response_ready;
} ps4_auth_state_t;

static ps4_auth_sign_cb_t ps4_auth_sign_cb = NULL;

static const uint8_t ps4_0xf3_report[] = {0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00};

static const uint32_t crc32_table[] = {0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
                                       0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                                       0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

static ps4_auth_state_t auth_state = {};

static uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = ~0L;

    for (size_t i = 0; i < length; i++) {
        uint8_t table_idx = 0;

        table_idx = crc ^ (data[i] >> (0 * 4));
        crc = *(uint32_t *)(crc32_table + (table_idx & 0x0f)) ^ (crc >> 4);
        table_idx = crc ^ (data[i] >> (1 * 4));
        crc = *(uint32_t *)(crc32_table + (table_idx & 0x0f)) ^ (crc >> 4);
    }

    return ~crc;
}

void ps4_auth_init(const char *private_key, size_t private_key_len, const uint8_t serial[PS4_AUTH_SERIAL_LENGTH],
                   const uint8_t ca_signature[PS4_AUTH_SIGNATURE_LENGTH], ps4_auth_sign_cb_t sign_cb) {
    mbedtls_pk_context pk_context;

    mbedtls_pk_init(&pk_context);

    if (mbedtls_pk_parse_key(&pk_context, (unsigned char *)private_key, private_key_len, NULL, 0)) {
        auth_state.initialized = false;
        return;
    }

    if (mbedtls_rsa_export_raw(mbedtls_pk_rsa(pk_context),                                                       //
                               auth_state.challenge_response.key_n, sizeof(auth_state.challenge_response.key_n), //
                               NULL, 0,                                                                          //
                               NULL, 0,                                                                          //
                               NULL, 0,                                                                          //
                               auth_state.challenge_response.key_e, sizeof(auth_state.challenge_response.key_e))) {
        auth_state.initialized = false;
        return;
    }

    memcpy(auth_state.challenge_response.serial, serial, sizeof(auth_state.challenge_response.serial));
    memcpy(auth_state.challenge_response.ca_signature, ca_signature,
           sizeof(auth_state.challenge_response.ca_signature));

    ps4_auth_sign_cb = sign_cb;

    mbedtls_pk_free(&pk_context);

    auth_state.initialized = true;
}

void ps4_auth_reset() {
    auth_state.challenge_id = 0;
    auth_state.challenge_response_read_seq = 0;
    auth_state.challenge_response_ready = false;

    memset(auth_state.challenge_data, 0, sizeof(auth_state.challenge_data));
    memset(auth_state.challenge_response.signature, 0, sizeof(auth_state.challenge_response.signature));
}

void ps4_auth_set_challenge_report(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize) {
    if (!auth_state.initialized) {
        return;
    }

    if (bufsize != (sizeof(hid_ps4_challenge_report_t) - sizeof(report_id))) {
        return;
    }

    hid_ps4_challenge_report_t report = {};

    report.report_id = report_id;
    memcpy(&report.challenge_id, buffer, bufsize);

    if (report.challenge_seq == 0) {
        ps4_auth_reset();
    }

    if (crc32((void *)&report, sizeof(hid_ps4_challenge_report_t) - sizeof(report.checksum)) != report.checksum) {
        return;
    }

    size_t offset = report.challenge_seq * sizeof(report.challenge_data);
    if (offset >= sizeof(auth_state.challenge_data)) {
        return;
    }

    memcpy(&auth_state.challenge_data[offset], report.challenge_data,
           TU_MIN(sizeof(report.challenge_data), sizeof(auth_state.challenge_data) - offset));

    auth_state.challenge_id = report.challenge_id;

    if (sizeof(auth_state.challenge_data) - offset < sizeof(report.challenge_data)) {
        if (ps4_auth_sign_cb) {
            ps4_auth_sign_cb(auth_state.challenge_data);
        }
    }
}

uint16_t ps4_auth_get_challenge_state_report(uint8_t report_id, uint8_t *buffer) {
    if (!auth_state.initialized) {
        return 0;
    }

    hid_ps4_challenge_state_report_t report = {};

    report.report_id = report_id;
    report.challenge_id = auth_state.challenge_id;
    report.signing_state = auth_state.challenge_response_ready ? 0x00 : 0x10;

    report.checksum = crc32((void *)&report, sizeof(hid_ps4_challenge_state_report_t) - sizeof(report.checksum));
    memcpy(buffer, &report.challenge_id, sizeof(hid_ps4_challenge_state_report_t) - sizeof(report.report_id));

    return sizeof(hid_ps4_challenge_state_report_t) - sizeof(report.report_id);
}

uint16_t ps4_auth_get_challenge_report(uint8_t report_id, uint8_t *buffer) {
    if (!auth_state.challenge_response_ready || !auth_state.initialized) {
        return 0;
    }

    hid_ps4_challenge_report_t report = {};

    report.report_id = report_id;
    report.challenge_id = auth_state.challenge_id;
    report.challenge_seq = auth_state.challenge_response_read_seq;

    size_t offset = auth_state.challenge_response_read_seq * sizeof(report.challenge_data);
    if (offset >= sizeof(auth_state.challenge_response)) {
        auth_state.challenge_response_read_seq = 0;
        offset = 0;
    }

    memcpy(report.challenge_data, &((uint8_t *)&auth_state.challenge_response)[offset],
           TU_MIN(sizeof(report.challenge_data), sizeof(auth_state.challenge_response) - offset));

    report.checksum = crc32((void *)&report, sizeof(hid_ps4_challenge_report_t) - sizeof(report.checksum));
    memcpy(buffer, &report.challenge_id, sizeof(hid_ps4_challenge_report_t) - sizeof(report.report_id));

    auth_state.challenge_response_read_seq++;

    return sizeof(hid_ps4_challenge_report_t) - sizeof(report.report_id);
}

uint16_t ps4_auth_get_reset_report(uint8_t report_id, uint8_t *buffer) {
    (void)report_id;

    if (!auth_state.initialized) {
        return 0;
    }

    ps4_auth_reset();

    memcpy(buffer, ps4_0xf3_report, sizeof(ps4_0xf3_report));
    return sizeof(ps4_0xf3_report);
}

void ps4_auth_set_signed_challenge(const uint8_t singed_challenge[PS4_AUTH_CHALLENGE_LENGTH]) {
    memcpy(auth_state.challenge_response.signature, singed_challenge, sizeof(auth_state.challenge_response.signature));

    auth_state.challenge_response_ready = true;
}