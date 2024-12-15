# DivaConSlider - Cap1188 based touch slider for the DivaCon2040

---

## NOTICE

**DO NOT USE THIS.**

The Cap1188 is way to slow for this project. This pcb is only here for documentation purposes.

---

This board hosts four CAP1188 capacitive touch controllers along with the slider electrodes and LEDs for illumination.

## Notes

- The Display module can be installed optionally.
- Dimensions are not arcade accurate.

## Bill of Materials

| **Reference**                             |             **Description**              |     **Digikey Part**      |                                  **Notes**                                   |
| ----------------------------------------- | :--------------------------------------: | :-----------------------: | :--------------------------------------------------------------------------: |
| U1-U4                                     |         CAP1188 Touch Controller         |           n.a.            |                              CAP1188-1-CP-TR-ND                              |
| D1-D64                                    |           SK6812B SIDE RGB LED           |       1528-4691-ND        |                                                                              |
| C1-C68                                    |       0.1µF SMD Ceramic Capacitor        |      1276-1003-1-ND       |                                                                              |
| R1,R2,R4,R5,R7,R8,R10,R11,R13,R14,R16,R17 |            10kΩ SMD Resistor             | 2019-RK73H2ATTD1002FTR-ND | R16 and R17 are only needed if the Display module does not have i2c pull-ups |
| R3                                        |            150kΩ SMD Resistor            | 2019-RK73H2ATTD1503FCT-ND |                                                                              |
| R6                                        |         120kΩ       SMD Resistor         | 2019-RK73H2ATTD1203FCT-ND |                                                                              |
| R9                                        |         100kΩ       SMD Resistor         | 2019-RK73H2ATTD1003FCT-ND |                                                                              |
| R12                                       |         82kΩ       SMD Resistor          | 2019-RK73H2ATTD8202FCT-ND |                                                                              |
| R15                                       |            470Ω SMD Resistor             | 2019-RK73H2ATTD4700FTR-ND |                                                                              |
| J1,J2,J3                                  |     SWH201 Vertical 4 Pin Header 2mm     |         S9457-ND          |                 S9406-ND plus S9475CT-ND is the counterpart                  |
| J4                                        | 0.96" SSD1306 128x64 OLED Display module | 3190-DLC0096DNOG-W-10-ND  |                                   optional                                   |
