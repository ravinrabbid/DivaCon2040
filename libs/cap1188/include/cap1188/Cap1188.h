#ifndef _CAP1188_CAP1188_H_
#define _CAP1188_CAP1188_H_

#include "hardware/i2c.h"

class Cap1188 {
  public:
    enum class Register {
        MAIN_CONTROL = 0x00,

        GENERAL_STATUS = 0x02,
        SENSOR_INPUT_STATUS = 0x03,
        LED_STATUS = 0x04,
        NOISE_FLAG_STATUS = 0x0A,

        SENSOR_INPUT_1_DELTA_COUNT = 0x10,
        SENSOR_INPUT_2_DELTA_COUNT = 0x11,
        SENSOR_INPUT_3_DELTA_COUNT = 0x12,
        SENSOR_INPUT_4_DELTA_COUNT = 0x13,
        SENSOR_INPUT_5_DELTA_COUNT = 0x14,
        SENSOR_INPUT_6_DELTA_COUNT = 0x15,
        SENSOR_INPUT_7_DELTA_COUNT = 0x16,
        SENSOR_INPUT_8_DELTA_COUNT = 0x17,

        SENSITIVITY_CONTROL = 0x1F,
        CONFIGURATION = 0x20,
        SENSOR_INPUT_ENABLE = 0x21,
        SENSOR_INPUT_CONFIGURATION = 0x22,
        SENSOR_INPUT_CONFIGURATION_2 = 0x23,
        AVERAGING_AND_SAMPLING_CONFIG = 0x24,
        CALIBRATION_ACTIVATE = 0x26,
        INTERRUPT_ENABLE = 0x27,
        REPEAT_RATE_ENABLE = 0x28,
        MULTIPLE_TOUCH_CONFIGURATION = 0x2A,
        MULTIPLE_TOUCH_PATTERN_CONFIGURATION = 0x2B,
        MULTIPLE_TOUCH_PATTERN = 0x2D,
        RECALIBRATION_CONFIGURATION = 0x2F,

        SENSOR_INPUT_1_THRESHOLD = 0x30,
        SENSOR_INPUT_2_THRESHOLD = 0x31,
        SENSOR_INPUT_3_THRESHOLD = 0x32,
        SENSOR_INPUT_4_THRESHOLD = 0x33,
        SENSOR_INPUT_5_THRESHOLD = 0x34,
        SENSOR_INPUT_6_THRESHOLD = 0x35,
        SENSOR_INPUT_7_THRESHOLD = 0x36,
        SENSOR_INPUT_8_THRESHOLD = 0x37,
        SENSOR_INPUT_NOISE_THRESHOLD = 0x38,

        STANDBY_CHANNEL = 0x40,
        STANDBY_CONFIGURATION = 0x41,
        STANDBY_SENSITIVITY = 0x42,
        STANDBY_THRESHOLD = 0x43,

        CONFIGURATION_2 = 0x44,

        SENSOR_INPUT_1_BASE_COUNT = 0x50,
        SENSOR_INPUT_2_BASE_COUNT = 0x51,
        SENSOR_INPUT_3_BASE_COUNT = 0x52,
        SENSOR_INPUT_4_BASE_COUNT = 0x53,
        SENSOR_INPUT_5_BASE_COUNT = 0x54,
        SENSOR_INPUT_6_BASE_COUNT = 0x55,
        SENSOR_INPUT_7_BASE_COUNT = 0x56,
        SENSOR_INPUT_8_BASE_COUNT = 0x57,

        LED_OUTPUT_TYPE = 0x71,
        SENSOR_INPUT_LED_LINKING = 0x72,
        LED_POLARITY = 0x73,
        LED_OUTPUT_CONTROL = 0x74,
        LINKED_LED_TRANSISTION_CONTROL = 0x77,
        LED_MIRROR_CONTROL = 0x79,
        LED_BEHAVIOUR_1 = 0x81,
        LED_BEHAVIOUR_2 = 0x82,
        LED_PULSE_1_PERIOD = 0x84,
        LED_PULSE_2_PERIOD = 0x85,
        LED_BREATHE_PERIOD = 0x86,
        LED_CONFIG = 0x88,
        LED_PULSE_1_DUTY_CYCLE = 0x90,
        LED_PULSE_2_DUTY_CYCLE = 0x91,
        LED_BREATHE_DUTY_CYCLE = 0x92,
        LED_DIRECT_DUTY_CYCLE = 0x93,
        LED_DIRECT_RAMP_RATES = 0x94,
        LED_OFF_DELAY = 0x95,

        SENSOR_INPUT_1_CALIBRATION = 0xB1,
        SENSOR_INPUT_2_CALIBRATION = 0xB2,
        SENSOR_INPUT_3_CALIBRATION = 0xB3,
        SENSOR_INPUT_4_CALIBRATION = 0xB4,
        SENSOR_INPUT_5_CALIBRATION = 0xB5,
        SENSOR_INPUT_6_CALIBRATION = 0xB6,
        SENSOR_INPUT_7_CALIBRATION = 0xB7,
        SENSOR_INPUT_8_CALIBRATION = 0xB8,

        SENSOR_INPUT_CALIBRATION_LSB_1 = 0xB9,
        SENSOR_INPUT_CALIBRATION_LSB_2 = 0xBA,

        PRODUCT_ID = 0xFD,
        MANUFACTURER_ID = 0xFE,
        REVISION = 0xFF,
    };

    enum class Sensitivity {
        S128 = 0x00, // Most sensitive
        S64 = 0x01,
        S32 = 0x02, // default
        S16 = 0x03,
        S8 = 0x04,
        S4 = 0x05,
        S2 = 0x06,
        S1 = 0x07, // Least sensitive
    };

    enum class Gain {
        G1 = 0x00,
        G2 = 0x01,
        G4 = 0x02,
        G8 = 0x03,
    };

  private:
    i2c_inst *m_i2c;
    uint8_t m_address;

  public:
    Cap1188(uint8_t address, i2c_inst *i2c, uint8_t threshold = 64, Sensitivity sensitivity = Sensitivity::S32,
            Gain gain = Gain::G1);

    uint8_t getTouched();
    bool getTouched(uint8_t input);

    void setGain(Gain gain);
    void setSensitivity(Sensitivity sensitivity);
    void setThreshold(uint8_t threshold);

    int8_t getDeltaCount(uint8_t input);

    void clearInterrupt();

  private:
    uint8_t readRegister(Register reg, uint8_t offset = 0);
    void writeRegister(Register reg, uint8_t value, uint8_t offset = 0);
};

#endif // _CAP1188_CAP1188_H_