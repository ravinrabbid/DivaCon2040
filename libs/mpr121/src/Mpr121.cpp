#include "Mpr121.h"

#include "pico/time.h"

Mpr121::Mpr121(uint8_t address, i2c_inst *i2c, uint8_t touch_threshold, uint8_t release_threshold, bool autoconfig)
    : m_i2c(i2c), m_address(address) {

    writeRegister(Register::SOFTRESET, 0x63); // Magic byte 0x63 triggers soft reset
    sleep_ms(1);

    writeRegister(Register::ECR, 0x00); // Set stop mode

    setThresholds(touch_threshold, release_threshold);

    // Base Line Filtering Control Rising
    writeRegister(Register::MHDR, 0x01); // Maximum Half Delta
    writeRegister(Register::NHDR, 0x01); // Noise Half Delta
    writeRegister(Register::NCLR, 0x0E); // Noise Count Limit
    writeRegister(Register::FDLR, 0x00); // Filter Delay Count Limit

    // Base Line Filtering Control Falling
    writeRegister(Register::MHDF, 0x01); // Maximum Half Delta
    writeRegister(Register::NHDF, 0x05); // Noise Half Delta
    writeRegister(Register::NCLF, 0x01); // Noise Count Limit
    writeRegister(Register::FDLF, 0x40); // Filter Delay Count Limit

    // Base Line Filtering Control Touched
    writeRegister(Register::NHDT, 0x00); // Noise Half Delta
    writeRegister(Register::NCLT, 0x00); // Noise Count Limit
    writeRegister(Register::FDLT, 0x00); // Filter Delay Count Limit

    writeRegister(Register::DEBOUNCE, 0);   // Debounce off
    writeRegister(Register::CONFIG1, 0x10); // 6 samples to first level filter, 16uA electrode charge
    writeRegister(Register::CONFIG2, 0x20); // 0.5uS charge time, 4 samples to second level filter, 1ms sample interval

    if (autoconfig) {
        writeRegister(Register::AUTOCONFIG0, 0x0B); // Enable Auto-(Re)Config, baseline value to 5MSBs

        // Auto-config configuration for Vdd = 3.3V
        writeRegister(Register::UPLIMIT, 200);     // ((Vdd - 0.7)/Vdd) * 256
        writeRegister(Register::TARGETLIMIT, 180); // UPLIMIT * 0.9
        writeRegister(Register::LOWLIMIT, 130);    // UPLIMIT * 0.65
    }

    // enable all electrodes and set run Mode
    writeRegister(Register::ECR, 0x8F);
}

uint16_t Mpr121::getTouched() {
    uint16_t touched = readRegister16(Register::TOUCHSTATUS_L);

    return touched & 0x0FFF;
}
bool Mpr121::getTouched(uint8_t input) {
    if (input > 12) {
        return false;
    }

    return getTouched() & (1 << input);
}

void Mpr121::setThresholds(uint8_t touch_threshold, uint8_t release_threshold) {
    for (uint8_t i = 0; i < 12; ++i) {
        setThreshold(i, touch_threshold, release_threshold);
    }
}

void Mpr121::setThreshold(uint8_t input, uint8_t touch_threshold, uint8_t release_threshold) {
    writeRegister(Register::TOUCHTH_0, touch_threshold, 2 * input);
    writeRegister(Register::RELEASETH_0, release_threshold, 2 * input);
}

uint16_t Mpr121::getBaselineData(uint8_t input) {
    if (input > 12) {
        return 0;
    }

    return (readRegister8(Register::BASELINE_0, input) << 2);
}

uint16_t Mpr121::getFilteredData(uint8_t input) {
    if (input > 12) {
        return 0;
    };
    return readRegister16(Register::FILTDATA_0L, input * 2);
}

uint8_t Mpr121::readRegister8(Mpr121::Register reg, uint8_t offset) {
    uint8_t result;

    uint8_t reg_addr = static_cast<uint8_t>(reg) + offset;
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, &result, 1, false);

    return result;
}

uint16_t Mpr121::readRegister16(Mpr121::Register reg, uint8_t offset) {
    uint8_t result[2];

    uint8_t reg_addr = static_cast<uint8_t>(reg) + offset;
    i2c_write_blocking(m_i2c, m_address, &reg_addr, 1, true);
    i2c_read_blocking(m_i2c, m_address, result, 2, false);

    return static_cast<uint16_t>(result[1]) << 8 | static_cast<uint16_t>(result[0]);
}

void Mpr121::writeRegister(Mpr121::Register reg, uint8_t value, uint8_t offset) {
    // Only ECR and GPIO related registers can be written in 'Run' mode.
    bool need_stop = (reg != Register::ECR) && (reg != Register::GPIODIR) && (reg != Register::GPIOEN) &&
                     (reg != Register::GPIOSET) && (reg != Register::GPIOCLR) && (reg != Register::GPIOTOGGLE);

    auto do_write_register = [&](Mpr121::Register reg, uint8_t value, uint8_t offset = 0) {
        uint8_t reg_addr = static_cast<uint8_t>(reg) + offset;
        uint8_t data[] = {reg_addr, value};

        i2c_write_blocking(m_i2c, m_address, data, 2, false);
    };

    if (need_stop) {
        // Backup ECR
        uint8_t ecr = readRegister8(Register::ECR);

        // Issue stop
        do_write_register(Register::ECR, 0x00);

        // Write actual data
        do_write_register(reg, value, offset);

        // Restore ECR
        do_write_register(Register::ECR, ecr);
    } else {
        do_write_register(reg, value, offset);
    }
}