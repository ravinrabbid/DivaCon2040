#ifndef _MPR121_MPR121_H_
#define _MPR121_MPR121_H_

#include "hardware/i2c.h"

class Mpr121 {
  public:
    enum class Register {
        TOUCHSTATUS_L = 0x00,
        TOUCHSTATUS_H = 0x01,
        FILTDATA_0L = 0x04,
        FILTDATA_0H = 0x05,
        BASELINE_0 = 0x1E,
        MHDR = 0x2B,
        NHDR = 0x2C,
        NCLR = 0x2D,
        FDLR = 0x2E,
        MHDF = 0x2F,
        NHDF = 0x30,
        NCLF = 0x31,
        FDLF = 0x32,
        NHDT = 0x33,
        NCLT = 0x34,
        FDLT = 0x35,

        TOUCHTH_0 = 0x41,
        RELEASETH_0 = 0x42,
        DEBOUNCE = 0x5B,
        CONFIG1 = 0x5C,
        CONFIG2 = 0x5D,
        CHARGECURR_0 = 0x5F,
        CHARGETIME_1 = 0x6C,
        ECR = 0x5E,
        AUTOCONFIG0 = 0x7B,
        AUTOCONFIG1 = 0x7C,
        UPLIMIT = 0x7D,
        LOWLIMIT = 0x7E,
        TARGETLIMIT = 0x7F,

        GPIODIR = 0x76,
        GPIOEN = 0x77,
        GPIOSET = 0x78,
        GPIOCLR = 0x79,
        GPIOTOGGLE = 0x7A,

        SOFTRESET = 0x80,
    };

  private:
    i2c_inst *m_i2c;
    uint8_t m_address;

  public:
    Mpr121(uint8_t address, i2c_inst *i2c, uint8_t touch_threshold = 12, uint8_t release_threshold = 6,
           bool autoconfig = true);

    uint16_t getTouched();
    bool getTouched(uint8_t input);

    void setThresholds(uint8_t touch_threshold, uint8_t release_threshold);
    void setThreshold(uint8_t input, uint8_t touch_threshold, uint8_t release_threshold);

    uint16_t getBaselineData(uint8_t input);
    uint16_t getFilteredData(uint8_t input);

  private:
    uint8_t readRegister8(Register reg, uint8_t offset = 0);
    uint16_t readRegister16(Register reg, uint8_t offset = 0);
    void writeRegister(Register reg, uint8_t value, uint8_t offset = 0);
};

#endif // _MPR121_MPR121_H_