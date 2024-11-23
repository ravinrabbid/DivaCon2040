#include "Cap1188.h"

Cap1188::Cap1188(uint8_t address, i2c_inst *i2c, uint8_t threshold, Sensitivity sensitivity, Gain gain)
    : m_i2c(i2c), m_address(address) {

    setGain(gain);
    setSensitivity(sensitivity);
    setThreshold(threshold);

    // Configuration
    // - 7: Disable SMBus timeout for I2C compliance
    // - 6: Disable WAKE pin
    // - 5: Discard noisy samples (non default)
    // - 4: Enable noise filter
    // - 3: Enable touches to be held indefinitely
    // - 2:0: Unused
    writeRegister(Register::CONFIGURATION, 0x00);

    // Enable all inputs
    writeRegister(Register::SENSOR_INPUT_ENABLE, 0xFF);

    // Shorten sampling cycle
    // - 6:4: Sample count (4)
    // - 3:2: Sampling time (640us)
    // - 1:0: Cycle time (35ms)
    // Time: (Count * SampleTime) + 4.4ms
    // Minimum cycle time is 35ms, we can't get below.
    writeRegister(Register::AVERAGING_AND_SAMPLING_CONFIG, 0b00100100);

    // Disable touch and hold repeat
    writeRegister(Register::REPEAT_RATE_ENABLE, 0x00);

    // Allow multi-touch
    writeRegister(Register::MULTIPLE_TOUCH_CONFIGURATION, 0x00);

    clearInterrupt();
}

uint8_t Cap1188::getTouched() {
    // Interrupt needs to be cleared first to get a proper reading
    clearInterrupt();

    const auto touched = readRegister(Register::SENSOR_INPUT_STATUS);

    return touched;
}

bool Cap1188::getTouched(uint8_t input) {
    if (input > 7) {
        return false;
    }

    return getTouched() & (1 << input);
}

void Cap1188::setGain(Gain gain) {
    const auto main_ctrl = readRegister(Register::MAIN_CONTROL);

    writeRegister(Register::SENSITIVITY_CONTROL, (main_ctrl & 0xC0) | (static_cast<uint8_t>(gain) << 6));
}

void Cap1188::setSensitivity(Cap1188::Sensitivity sensitivity) {
    // Data sheet 6.5: The BASE_SHIFT[3:0] bits normally do not need to be updated.
    //                 These settings will not affect touch detection or sensitivity.

    const auto sens_ctrl = readRegister(Register::SENSITIVITY_CONTROL);

    writeRegister(Register::SENSITIVITY_CONTROL, (sens_ctrl & 0x38) | (static_cast<uint8_t>(sensitivity) << 4));
}

void Cap1188::setThreshold(uint8_t threshold) {
    // By default writing SENSOR_INPUT_1_THRESHOLD will set all inputs
    if (threshold > 127) {
        threshold = 127;
    }

    writeRegister(Register::SENSOR_INPUT_1_THRESHOLD, threshold);
}

int8_t Cap1188::getDeltaCount(uint8_t input) {
    if (input > 7) {
        return false;
    }

    return readRegister(Register::SENSOR_INPUT_1_DELTA_COUNT, input);
}

void Cap1188::clearInterrupt() {
    const auto main_ctrl = readRegister(Register::MAIN_CONTROL);

    writeRegister(Register::MAIN_CONTROL, main_ctrl & ~0x01);
}

uint8_t Cap1188::readRegister(Cap1188::Register reg, uint8_t offset) {
    uint8_t result;
    const uint8_t reg_addr = static_cast<uint8_t>(reg) + offset;

    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, &result, 1, false);

    return result;
}

void Cap1188::writeRegister(Cap1188::Register reg, uint8_t value, uint8_t offset) {
    const uint8_t reg_addr = static_cast<uint8_t>(reg) + offset;
    const uint8_t data[] = {reg_addr, value};

    i2c_write_blocking(m_i2c, m_address, data, 2, false);
}