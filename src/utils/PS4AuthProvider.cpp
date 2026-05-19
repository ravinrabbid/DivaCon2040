#include "utils/PS4AuthProvider.h"

#include "PS4AuthConfiguration.h"

#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#include "usb/device_driver.h"

#include <cstring>

namespace {
int const_rng(void *p_rng, unsigned char *p, size_t len) {
    (void)p_rng;

    std::memset(p, 0x39, len);

    return 0;
}
} // namespace

namespace Divacon::Utils {

PS4AuthProvider::PS4AuthProvider() {
    if (!Divacon::Config::PS4Auth::config.enabled) {
        return;
    }

    mbedtls_pk_init(&m_pk_context);

    if (mbedtls_pk_parse_key(
            &m_pk_context, reinterpret_cast<const unsigned char *>(Divacon::Config::PS4Auth::config.key_pem.c_str()),
            Divacon::Config::PS4Auth::config.key_pem.size() + 1, nullptr, 0, const_rng, nullptr) != 0) {
        return;
    }

    mbedtls_rsa_set_padding(mbedtls_pk_rsa(m_pk_context), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

    m_key_valid = true;
}

PS4AuthProvider::~PS4AuthProvider() { mbedtls_pk_free(&m_pk_context); }

std::optional<std::array<uint8_t, PS4AuthProvider::SIGNATURE_LENGTH>>
PS4AuthProvider::sign(const std::array<uint8_t, PS4AuthProvider::SIGNATURE_LENGTH> &challenge) {
    if (!m_key_valid) {
        return std::nullopt;
    }

    std::array<uint8_t, 32> hashed_challenge = {};
    auto signed_challenge = std::array<uint8_t, PS4AuthProvider::SIGNATURE_LENGTH>();

    auto *const rsa_context = mbedtls_pk_rsa(m_pk_context);

    if (mbedtls_sha256(challenge.data(), challenge.size(), hashed_challenge.data(), 0) != 0) {
        return std::nullopt;
    }

    if (mbedtls_rsa_rsassa_pss_sign(rsa_context, const_rng, nullptr, MBEDTLS_MD_SHA256, hashed_challenge.size(),
                                    hashed_challenge.data(), signed_challenge.data()) != 0) {
        return std::nullopt;
    }

    return signed_challenge;
}

} // namespace Divacon::Utils