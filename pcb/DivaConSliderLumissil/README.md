# DivaConSlider - IS31SE5117A based touch slider for the DivaCon2040

---

## NOTICE

**DO NOT USE THIS yet.**

The IS31SE5117A is pretty new and unexplored by hobbyists. I haven't gotten it to work like described in the datasheet yet.

This still work in progress.

---

This board hosts two IS31SE5117A capacitive touch controllers along with the slider electrodes and LEDs for illumination.

## Notes

- The Display module can be installed optionally.
- Dimensions are not arcade accurate.

## Bill of Materials

| **Reference**          |             **Description**              |       **Digikey Part**       |                                 **Notes**                                  |
| ---------------------- | :--------------------------------------: | :--------------------------: | :------------------------------------------------------------------------: |
| U1                     |      LTC4316 I2C Address Translator      |    505-LTC4316CDD#PBF-ND     |                                                                            |
| U2, U3                 |       IS31SE5117A Touch Controller       | 2521-IS31SE5117A-QFLS3-TR-ND |                                                                            |
| D1-D64                 |           SK6812B SIDE RGB LED           |         1528-4691-ND         |                                                                            |
| C1,C3,C6,C7,C9,C12-C77 |       0.1µF SMD Ceramic Capacitor        |        1276-1000-1-ND        |                                                                            |
| C2,C8                  |        1µF SMD Ceramic Capacitor         |        1276-1184-1-ND        |                                                                            |
| C4,C10                 |       2.2nF SMD Ceramic Capacitor        |        1276-1110-1-ND        |                             Could also be 10nF                             |
| C5,C11                 |       4.7µF SMD Ceramic Capacitor        |        1276-1784-1-ND        |                                                                            |
| R1-R6,R9-R12           |            10kΩ SMD Resistor             |       311-10.0KHRCT-ND       | R3 and R4 are only needed if the Display module does not have i2c pull-ups |
| R7                     |            976kΩ SMD Resistor            |       311-976KHRCT-ND        |                                                                            |
| R8                     |         102kΩ       SMD Resistor         |       311-102KHRCT-ND        |                                                                            |
| R13-R45                |         510Ω       SMD Resistor          |        311-510HRCT-ND        |                                                                            |
| J1,J2,J3               |     SWH201 Vertical 4 Pin Header 2mm     |           S9457-ND           |                S9406-ND plus S9475CT-ND is the counterpart                 |
| J4                     | 0.96" SSD1306 128x64 OLED Display module |   3190-DLC0096DNOG-W-10-ND   |                                  optional                                  |
