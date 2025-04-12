#!/usr/bin/env python3

import sys
from textwrap import wrap

HEADER_TEMPLATE = """#include "utils/PS4AuthProvider.h"

namespace Divacon::Config::PS4Auth {{

const Utils::PS4AuthProvider::Config config = {{
    true,                                                                                             // Enabled
    {{{serial}}}, // Serial
    {{{sig}}}, // Signature
    R"pem({pem})pem"}};                        // Pem

}} // namespace Divacon::Config::PS4Auth"""


def read_key():
    with open("key.pem", "rt") as key_file:
        key_pem = key_file.read()
        if not key_pem:
            raise Exception("'key.pem' invalid, cannot be empty.")

        return key_pem


def read_serial():
    with open("serial.txt", "rt") as serial_file:
        serial = serial_file.read()
        if len(serial) > 32:
            raise Exception("'serial.txt' invalid, must be shorter than 32.")

        serial = bytes.fromhex(serial.rjust(32, "0"))

        return serial


def read_sig():
    with open("sig.bin", "rb") as sig_file:
        sig = bytes(sig_file.read())
        if len(sig) != 256:
            raise Exception("'sig.bin' invalid, size must be 256 bytes.")

        return sig


def main():
    key_pem = read_key()
    serial = read_serial()
    sig = read_sig()

    with open("PS4AuthConfiguration.h", "w") as header:
        header.write(
            HEADER_TEMPLATE.format(
                serial=", ".join("0x{:02X}".format(x) for x in serial),
                sig="\n     ".join(
                    wrap(", ".join("0x{:02X}".format(x) for x in sig), 115)
                ),
                pem=key_pem,
            )
        )


if __name__ == "__main__":
    sys.exit(main())
