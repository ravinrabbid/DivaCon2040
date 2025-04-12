#ifndef _USB_DEVICE_HID_PS4_AUTH_H_
#define _USB_DEVICE_HID_PS4_AUTH_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PS4_AUTH_CHALLENGE_LENGTH (256)
#define PS4_AUTH_SIGNATURE_LENGTH (256)
#define PS4_AUTH_SERIAL_LENGTH (16)

typedef void (*ps4_auth_sign_cb_t)(const uint8_t[PS4_AUTH_CHALLENGE_LENGTH]);

void ps4_auth_init(const char *private_key, size_t private_key_len, const uint8_t serial[PS4_AUTH_SERIAL_LENGTH],
                   const uint8_t ca_signature[PS4_AUTH_SIGNATURE_LENGTH], ps4_auth_sign_cb_t sign_cb);
void ps4_auth_reset();

void ps4_auth_set_challenge_report(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize);
uint16_t ps4_auth_get_challenge_state_report(uint8_t report_id, uint8_t *buffer);
uint16_t ps4_auth_get_challenge_report(uint8_t report_id, uint8_t *buffer);
uint16_t ps4_auth_get_reset_report(uint8_t report_id, uint8_t *buffer);

void ps4_auth_set_signed_challenge(const uint8_t singed_challenge[PS4_AUTH_CHALLENGE_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_HID_PS4_AUTH_H_