## Hello, and thank you for your interest!

Created by Gustavo Parra under Apollo Flight Systems with the assistance of ChatGPT. Project modeled after 3D print project here, but was lacking any formal documentation on code or hardware development.
https://makerworld.com/en/models/21113-throttle-with-7-axis-and-32-buttons

---

# MoonDog Throttle Quadrant

DIY 7-Axis, 32-Button USB Flight Controller for Microsoft Flight Simulator 2020  
Designed for builders, pilots, and engineers who want ultimate modularity and custom control.

---

## About

The MoonDog Throttle Quadrant is a fully open-source DIY USB controller designed for Microsoft Flight Simulator 2020.  
It provides 7 analog axes and 32 digital buttons using simple, accessible components — ideal for home cockpits, simulation enthusiasts, and experimental controls.

Built to be modular and expandable, this project empowers you to design your own throttle layouts, racing controls, or immersive cockpit systems.


---

## Features

- 🛫 7 fully mappable analog axes (throttle, mixture, prop, trim, flaps, etc.)
- 🎮 32 physical button inputs (momentary or toggle)
- 🔌 USB HID-compliant Joystick device (no extra drivers needed)
- 🛠️ Modular architecture — easily adaptable to racing, simulation, robotics
- 🗂️ Preconfigured MSFS 2020 input profile included

---

## Hardware Requirements

- Arduino Leonardo (ATmega32U4)
- 2x MCP23017 I2C GPIO Expanders
- Potentiometers (1 per analog axis)
- Pushbuttons or toggle switches (up to 32 total)
- USB Micro cable
- Breadboard / Wires / Connectors
- Optional: Enclosure, knobs, mounting hardware
- Optional: 3D Printed Enclosure  
  - [🔗 3D Print Files Repository (Coming Soon)](https://your-link-here.com)

---

## Software Setup

- Visual Studio Code
- PlatformIO IDE Extension
- Arduino Joystick Library (mheironimus/ArduinoJoystickLibrary)
- Adafruit MCP23017 Library (adafruit/Adafruit-MCP23017-Arduino-Library)

All dependencies are automatically handled via PlatformIO.

---

## Building and Uploading

1. Clone this repository.
2. Open the project folder in Visual Studio Code.
3. Use PlatformIO to build and upload firmware to the Arduino Leonardo.
4. Run `installer.bat` to copy the MSFS profile automatically.
5. Start Microsoft Flight Simulator — your device will auto-recognize!

---

## License

This project is licensed under the MIT License.  
Please see the LICENSE file for full details.

---

## Credits

Developed by Gustavo Parra  
Apollo Flight Systems — 2025
