# M.I.N.I.G.E.K.I : a portable ONGEKI controller

A portable implementation of an ONGEKI controller, with all necessary buttons, two macros, RGB LED support, USB-C.

## Hardware setup

- Raspberry RP2040 microcontroller
- 10 hotswappable Cherry MX switches (2x Start, 2x Wall, 2x R+G+B), 2 Macro switches
- 10 RGB daisy-chained WS2812-compatible LEDs (Under every switch except Start, and two under the board)
- USB Type-C connector for both data and power. 

## Software capabilities

- HID Gamepad mode or HID Mouse/Keyboard mode
    - Switchable on boot
- Software addressable RGB via HID
- Auto-rollback to static RGB LEDs if no HID is detected
    - LED can be disabled on boot
- High polling rate
- Switch debouncing
- 8-bit slider position

## License and Open Source licensing

minigeki is under GPLv3 in compliance with the usage of existing GPLv3 code. License information can be found in the LICENSE file.

A list of open source components and their licenses can be found in the LICENSING file.

## Extra credit and thank you

- CrazyRedMachine
- megumi_akasaka
- SpeedyPotato
