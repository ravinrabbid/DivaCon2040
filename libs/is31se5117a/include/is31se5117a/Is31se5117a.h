#ifndef _IS31SE5117A_IS31SE5117A_H_
#define _IS31SE5117A_IS31SE5117A_H_

#include "hardware/i2c.h"

class Is31se5117a {
  public:
    enum class Register {
        // ------
        // All Pages
        // ------

        CHIP_PART_NUMBER = 0x00,
        CHIP_VERSION_1 = 0x01,
        CHIP_VERSION_2 = 0x02,
        FIRMWARE_VERSION = 0x03,
        RUN_VERSION = 0x04,

        MAIN_CONTROL = 0x05,
        SWITCH_PAGE = 0x06,

        KEY_STATUS_1 = 0x07,
        KEY_STATUS_2 = 0x08,

        BUZZER_REGISTER = 0x09,

        // ------
        // Page 0
        // ------

        KEY0_SIGNAL = 0x0A,
        KEY1_SIGNAL = 0x0B,
        KEY2_SIGNAL = 0x0C,
        KEY3_SIGNAL = 0x0D,
        KEY4_SIGNAL = 0x0E,
        KEY5_SIGNAL = 0x0F,
        KEY6_SIGNAL = 0x10,
        KEY7_SIGNAL = 0x11,
        KEY8_SIGNAL = 0x12,
        KEY9_SIGNAL = 0x13,
        KEY10_SIGNAL = 0x14,
        KEY11_SIGNAL = 0x15,
        KEY12_SIGNAL = 0x16,
        KEY13_SIGNAL = 0x17,
        KEY14_SIGNAL = 0x18,
        KEY15_SIGNAL = 0x19,

        KEY0_RAW_COUNT_H = 0x1A,
        KEY0_RAW_COUNT_L = 0x1B,
        KEY1_RAW_COUNT_H = 0x1C,
        KEY1_RAW_COUNT_L = 0x1D,
        KEY2_RAW_COUNT_H = 0x1E,
        KEY2_RAW_COUNT_L = 0x1F,
        KEY3_RAW_COUNT_H = 0x20,
        KEY3_RAW_COUNT_L = 0x21,
        KEY4_RAW_COUNT_H = 0x22,
        KEY4_RAW_COUNT_L = 0x23,
        KEY5_RAW_COUNT_H = 0x24,
        KEY5_RAW_COUNT_L = 0x25,
        KEY6_RAW_COUNT_H = 0x26,
        KEY6_RAW_COUNT_L = 0x27,
        KEY7_RAW_COUNT_H = 0x28,
        KEY7_RAW_COUNT_L = 0x29,
        KEY8_RAW_COUNT_H = 0x2A,
        KEY8_RAW_COUNT_L = 0x2B,
        KEY9_RAW_COUNT_H = 0x2C,
        KEY9_RAW_COUNT_L = 0x2D,
        KEY10_RAW_COUNT_H = 0x2E,
        KEY10_RAW_COUNT_L = 0x2F,
        KEY11_RAW_COUNT_H = 0x30,
        KEY11_RAW_COUNT_L = 0x31,
        KEY12_RAW_COUNT_H = 0x32,
        KEY12_RAW_COUNT_L = 0x33,
        KEY13_RAW_COUNT_H = 0x34,
        KEY13_RAW_COUNT_L = 0x35,
        KEY14_RAW_COUNT_H = 0x36,
        KEY14_RAW_COUNT_L = 0x37,
        KEY15_RAW_COUNT_H = 0x38,
        KEY15_RAW_COUNT_L = 0x39,

        KEY0_BASELINE_H = 0x3A,
        KEY0_BASELINE_L = 0x3B,
        KEY1_BASELINE_H = 0x3C,
        KEY1_BASELINE_L = 0x3D,
        KEY2_BASELINE_H = 0x3E,
        KEY2_BASELINE_L = 0x3F,
        KEY3_BASELINE_H = 0x40,
        KEY3_BASELINE_L = 0x41,
        KEY4_BASELINE_H = 0x42,
        KEY4_BASELINE_L = 0x43,
        KEY5_BASELINE_H = 0x44,
        KEY5_BASELINE_L = 0x45,
        KEY6_BASELINE_H = 0x46,
        KEY6_BASELINE_L = 0x47,
        KEY7_BASELINE_H = 0x48,
        KEY7_BASELINE_L = 0x49,
        KEY8_BASELINE_H = 0x4A,
        KEY8_BASELINE_L = 0x4B,
        KEY9_BASELINE_H = 0x4C,
        KEY9_BASELINE_L = 0x4D,
        KEY10_BASELINE_H = 0x4E,
        KEY10_BASELINE_L = 0x4F,
        KEY11_BASELINE_H = 0x50,
        KEY11_BASELINE_L = 0x51,
        KEY12_BASELINE_H = 0x52,
        KEY12_BASELINE_L = 0x53,
        KEY13_BASELINE_H = 0x54,
        KEY13_BASELINE_L = 0x55,
        KEY14_BASELINE_H = 0x56,
        KEY14_BASELINE_L = 0x57,
        KEY15_BASELINE_H = 0x58,
        KEY15_BASELINE_L = 0x59,

        RAW_COUNT_FILTER = 0x5A,
        BASELINE_IIR_RATIO = 0x5B,
        LOCK_THRESHOLD_1 = 0x5C,
        LOCK_THRESHOLD_2 = 0x5D,
        LOCK_SCAN_CYCLE = 0x5E,
        RAW_COUNT_DIFFERENCE_LIMIT = 0x5F,
        MULTI_TOUCH_KEY_CONFIGURE = 0x60,
        MAX_DURATION_TIME = 0x61,
        INTERRUPT_CONFIGURATION = 0x62,
        INTERRUPT_REPEAT_TIME = 0x63,

        KEY_PIN_SELECT_1 = 0x64,
        KEY_PIN_SELECT_2 = 0x65,
        SHIELD_PIN_SELECT_1 = 0x66,
        SHIELD_PIN_SELECT_2 = 0x67,
        INT_PIN_SELECT_1 = 0x68,
        INT_PIN_SELECT_2 = 0x69,
        BUZZER_PIN_SELECT_1 = 0x6A,
        BUZZER_PIN_SELECT_2 = 0x6B,
        POW_PIN_SELECT_1 = 0x6C,
        POW_PIN_SELECT_2 = 0x6D,
        GPIO_PIN_SELECT_1 = 0x6E,
        GPIO_PIN_SELECT_2 = 0x6F,
        SLIDER1_KEY_SELECT_1 = 0x70,
        SLIDER1_KEY_SELECT_2 = 0x71,
        SLIDER2_KEY_SELECT_1 = 0x72,
        SLIDER2_KEY_SELECT_2 = 0x73,

        TKIII_CONTROL_1 = 0x74,
        TKIII_CONTROL_2 = 0x75,
        TKIII_CONTROL_3 = 0x76,
        TKIII_CCHG = 0x77,
        TKIII_PUD = 0x78,

        SYSTEM_CLOCK_SELECT = 0x79,
        SPREAD_SPECTRUM = 0x7A,

        AUTO_SLEEP_MODE = 0x7B,
        SLEEP_MODE_CONTROL = 0x7C,
        WAKE_UP_KEY_SELECT_1 = 0x7D,
        WAKE_UP_KEY_SELECT_2 = 0x7E,
        WAKE_UP_THRESHOLD = 0x7F,
        TKIII_SLEEP_MODE_CCHG = 0x80,
        TKIII_SLEEP_MODE_PUD = 0x81,
        SLP_RAW_1 = 0x82,
        SLP_RAW_2 = 0x83,
        SLP_BASELINE_1 = 0x84,
        SLP_BASELINE_2 = 0x85,

        // ... Sliders
        // ... Self-Test

        // ------
        // Page 1
        // ------

        KEY0_FINGER_THRESHOLD = 0x010A,
        KEY1_FINGER_THRESHOLD = 0x010B,
        KEY2_FINGER_THRESHOLD = 0x010C,
        KEY3_FINGER_THRESHOLD = 0x010D,
        KEY4_FINGER_THRESHOLD = 0x010E,
        KEY5_FINGER_THRESHOLD = 0x010F,
        KEY6_FINGER_THRESHOLD = 0x0110,
        KEY7_FINGER_THRESHOLD = 0x0111,
        KEY8_FINGER_THRESHOLD = 0x0112,
        KEY9_FINGER_THRESHOLD = 0x0113,
        KEY10_FINGER_THRESHOLD = 0x0114,
        KEY11_FINGER_THRESHOLD = 0x0115,
        KEY12_FINGER_THRESHOLD = 0x0116,
        KEY13_FINGER_THRESHOLD = 0x0117,
        KEY14_FINGER_THRESHOLD = 0x0118,
        KEY15_FINGER_THRESHOLD = 0x0119,

        KEY0_NOISE_THRESHOLD = 0x011A,
        KEY1_NOISE_THRESHOLD = 0x011B,
        KEY2_NOISE_THRESHOLD = 0x011C,
        KEY3_NOISE_THRESHOLD = 0x011D,
        KEY4_NOISE_THRESHOLD = 0x011E,
        KEY5_NOISE_THRESHOLD = 0x011F,
        KEY6_NOISE_THRESHOLD = 0x0120,
        KEY7_NOISE_THRESHOLD = 0x0121,
        KEY8_NOISE_THRESHOLD = 0x0122,
        KEY9_NOISE_THRESHOLD = 0x0123,
        KEY10_NOISE_THRESHOLD = 0x0124,
        KEY11_NOISE_THRESHOLD = 0x0125,
        KEY12_NOISE_THRESHOLD = 0x0126,
        KEY13_NOISE_THRESHOLD = 0x0127,
        KEY14_NOISE_THRESHOLD = 0x0128,
        KEY15_NOISE_THRESHOLD = 0x0129,

        KEY0_NEGATIVE_NOISE_THRESHOLD = 0x012A,
        KEY1_NEGATIVE_NOISE_THRESHOLD = 0x012B,
        KEY2_NEGATIVE_NOISE_THRESHOLD = 0x012C,
        KEY3_NEGATIVE_NOISE_THRESHOLD = 0x012D,
        KEY4_NEGATIVE_NOISE_THRESHOLD = 0x012E,
        KEY5_NEGATIVE_NOISE_THRESHOLD = 0x012F,
        KEY6_NEGATIVE_NOISE_THRESHOLD = 0x0130,
        KEY7_NEGATIVE_NOISE_THRESHOLD = 0x0131,
        KEY8_NEGATIVE_NOISE_THRESHOLD = 0x0132,
        KEY9_NEGATIVE_NOISE_THRESHOLD = 0x0133,
        KEY10_NEGATIVE_NOISE_THRESHOLD = 0x0134,
        KEY11_NEGATIVE_NOISE_THRESHOLD = 0x0135,
        KEY12_NEGATIVE_NOISE_THRESHOLD = 0x0136,
        KEY13_NEGATIVE_NOISE_THRESHOLD = 0x0137,
        KEY14_NEGATIVE_NOISE_THRESHOLD = 0x0138,
        KEY15_NEGATIVE_NOISE_THRESHOLD = 0x0139,

        KEY0_LOW_BASELINE_RESET = 0x013A,
        KEY1_LOW_BASELINE_RESET = 0x013B,
        KEY2_LOW_BASELINE_RESET = 0x013C,
        KEY3_LOW_BASELINE_RESET = 0x013D,
        KEY4_LOW_BASELINE_RESET = 0x013E,
        KEY5_LOW_BASELINE_RESET = 0x013F,
        KEY6_LOW_BASELINE_RESET = 0x0140,
        KEY7_LOW_BASELINE_RESET = 0x0141,
        KEY8_LOW_BASELINE_RESET = 0x0142,
        KEY9_LOW_BASELINE_RESET = 0x0143,
        KEY10_LOW_BASELINE_RESET = 0x0144,
        KEY11_LOW_BASELINE_RESET = 0x0145,
        KEY12_LOW_BASELINE_RESET = 0x0146,
        KEY13_LOW_BASELINE_RESET = 0x0147,
        KEY14_LOW_BASELINE_RESET = 0x0148,
        KEY15_LOW_BASELINE_RESET = 0x0149,

        KEY0_HYSTERESIS = 0x014A,
        KEY1_HYSTERESIS = 0x014B,
        KEY2_HYSTERESIS = 0x014C,
        KEY3_HYSTERESIS = 0x014D,
        KEY4_HYSTERESIS = 0x014E,
        KEY5_HYSTERESIS = 0x014F,
        KEY6_HYSTERESIS = 0x0150,
        KEY7_HYSTERESIS = 0x0151,
        KEY8_HYSTERESIS = 0x0152,
        KEY9_HYSTERESIS = 0x0153,
        KEY10_HYSTERESIS = 0x0154,
        KEY11_HYSTERESIS = 0x0155,
        KEY12_HYSTERESIS = 0x0156,
        KEY13_HYSTERESIS = 0x0157,
        KEY14_HYSTERESIS = 0x0158,
        KEY15_HYSTERESIS = 0x0159,

        KEY0_ON_DEBOUNCE = 0x015A,
        KEY1_ON_DEBOUNCE = 0x015B,
        KEY2_ON_DEBOUNCE = 0x015C,
        KEY3_ON_DEBOUNCE = 0x015D,
        KEY4_ON_DEBOUNCE = 0x015E,
        KEY5_ON_DEBOUNCE = 0x015F,
        KEY6_ON_DEBOUNCE = 0x0160,
        KEY7_ON_DEBOUNCE = 0x0161,
        KEY8_ON_DEBOUNCE = 0x0162,
        KEY9_ON_DEBOUNCE = 0x0163,
        KEY10_ON_DEBOUNCE = 0x0164,
        KEY11_ON_DEBOUNCE = 0x0165,
        KEY12_ON_DEBOUNCE = 0x0166,
        KEY13_ON_DEBOUNCE = 0x0167,
        KEY14_ON_DEBOUNCE = 0x0168,
        KEY15_ON_DEBOUNCE = 0x0169,

        KEY_INTERRUPT_ENABLE_1 = 0x016A,
        KEY_INTERRUPT_ENABLE_2 = 0x016B,

        GPIO_VALUE_1 = 0x016C,
        GPIO_VALUE_2 = 0x016D,
        GPIO_ENABLE_1 = 0x016E,
        GPIO_ENABLE_2 = 0x016F,
        GPIO_MAPPING_1 = 0x0170,
        GPIO_MAPPING_2 = 0x0171,
        GPIO_MAPPING_3 = 0x0172,
        GPIO_MAPPING_4 = 0x0173,
        GPIO_MAPPING_5 = 0x0174,
        GPIO_MAPPING_6 = 0x0175,
        GPIO_MAPPING_7 = 0x0176,
        GPIO_MAPPING_8 = 0x0177,
        GPIO_TOGGLE_EN_1 = 0x0178,
        GPIO_TOGGLE_EN_2 = 0x0179,

        KEY_SCAN_ONCE = 0x017A,
        TABLE_READY_MARK = 0x017B,
    };

  private:
    enum class RegisterPage {
        Page0,
        Page1,
    };

  private:
    i2c_inst *m_i2c;
    uint8_t m_address;

    RegisterPage m_current_page;

  public:
    Is31se5117a(uint8_t address, i2c_inst *i2c, uint8_t threshold, uint8_t hysteresis);

    uint16_t getTouched();
    bool getTouched(uint8_t input);

    void setFingerThresholds(uint8_t threshold);
    void setFingerThreshold(uint8_t input, uint8_t threshold);

    void setHystereses(uint8_t hysteresis);
    void setHysteresis(uint8_t input, uint8_t hysteresis);

    void setDebounceCounts(uint8_t count);
    void setDebounceCount(uint8_t input, uint8_t count);

    uint8_t getSignal(uint8_t input);
    uint16_t getRawCount(uint8_t input);
    uint16_t getBaseline(uint8_t input);

  private:
    void setRegisterPage(uint16_t address);

    uint8_t readRegister8(Register reg, uint8_t offset = 0);
    uint16_t readRegister16(Register reg, uint8_t offset = 0);
    void writeRegister(Register reg, uint8_t value, uint8_t offset = 0);
};

#endif // _IS31SE5117A_IS31SE5117A_H_