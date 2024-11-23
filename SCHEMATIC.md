# Schematic for binary release

## Overview

```text
                               -------------
GND -------- L1 Button -- GP0 -|1    R   40|- VBUS
GND -------- L2 Button -- GP1 -|2    P   39|- VSYS
                          GND -|3    i   38|- GND
GND -------- L3 Button -- GP2 -|4        37|- 3V3_EN -- Not connected
GND -------- R1 Button -- GP3 -|5    P   36|- 3V3
GND -------- R2 Button -- GP4 -|6    i   35|- ADC_VREF -- Not connected
GND -------- R3 Button -- GP5 -|7    c   34|- GP28 -- Slider LEDs DIN -- ...
                          GND -|8    o   33|- GND
GND -- Triangle Button -- GP6 -|9        32|- GP27 -- Select Button ---- GND
GND ---- Square Button -- GP7 -|10       31|- GP26 -- Start Button ----- GND
GND ----- Cross Button -- GP8 -|11       30|- RUN  -- Not connected
GND ---- Circle Button -- GP9 -|12       29|- GP22 -- Home Button ------ GND
                          GND -|13       28|- GND
... ---- Triangle LED -- GP10 -|14       27|- GP21 -- Right Button ----- GND
... ------ Square LED -- GP11 -|15       26|- GP20 -- Left Button ------ GND
... ------- Cross LED -- GP12 -|16       25|- GP19 -- Down Button ------ GND
... ------ Circle LED -- GP13 -|17       24|- GP18 -- Up Button -------- GND
                          GND -|18       23|- GND
... - Display - I2C1 SDA/GP14 -|19       22|- GP17/I2C0 SCL -- Touch Slider
... - Display - I2C1 SCL/GP15 -|20       21|- GP16/I2C0 SDA -- Touch Slider
                               -------------
```

## Power

If you lower the brightness of the LED strip, the 500mA of the USB bus should be sufficient to power everything.

For better reliability you can additionally add an external power supply connected to VSYS. In this case connect all 5V peripherals to VSYS also.
See [Pico Datasheet Capter 3.5](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf) for more details.

## Touch Slider

```text
----------------------------
|        |        |        |
|        |        |        |
|  0..11 |  2..9  |  0..11 | 
----------------------------
    ||       ||       ||
 -------- -------- --------
 | MPR  | | MPR  | | MPR  |
 | 121  | | 121  | | 121  |
 | 0x5A | | 0x5D | | 0x5C |
 -------- -------- --------
   |  |     |  |     |  |
   +--|-----+--|-----+--|-- SDA
      |        |        |
      +--------+--------+-- SCL

```

All MPR121s are connected to the same i2c bus in parallel, set the addresses as shown above.
Power them with 3V3.

For the LEDs use a WS2812 compatible strip. Connect DIN as shown above, power with VBUS (or VSYS if you power your controller externally).

## Button LEDs

The Pico GPIOs can't power a 20mA white LED directly, so I recommend driving them with a transistor.

The circuit below works for the Shinsanwa Switches which come with their own constant current LED driver. I you want to connect your LED directly, you need something to limit the current (e.g. a resistor).

```text
 GPIO --[4.7kOhm]--+----------+
                   |        B |                          -------------
               [4.7kOhm]    _____  BC547B                |           |
                   |      E |   | C                      | Shinsanwa |
 GND  -------------+------<-+   +-------------- LED(-) --| Switch    |
 VBUS/VSYS ------------------------------------ LED(+) --|___________|

```

## Display

Use SSD1306 OLED display with 128x64 resolution and connect it to its own i2c bus as shown above. Set the address to `0x3C`. Power with 3V3.
