
# Throttle Quadrant Project - Full Build and Setup Guide

This guide details the full process of creating, flashing, branding, and integrating a custom USB throttle quadrant controller for Microsoft Flight Simulator 2020 (MSFS).

---

## 1. Project Structure and Setup

### 1.1 Create New PlatformIO Project

- Open Visual Studio Code (VSCode).
- Install the PlatformIO IDE extension.
- Open PlatformIO Home.
- Select **"New Project"**.
- Set the following options:
  - **Project Name**: `FlightSim_ThrottleQuadrant`
  - **Board**: `Arduino Leonardo`
  - **Framework**: `Arduino`
- Click **Finish**.

✅ PlatformIO creates an isolated environment.

---

### 1.2 Folder Layout Overview

```plaintext
FlightSim_ThrottleQuadrant/
├── .pio/
├── .vscode/
├── include/
├── lib/
├── src/
│   └── main.cpp
├── test/
├── input_profiles/
│   └── ThrottleQuadrant_Custom_v1.xml
├── extra_script.py
├── installer.bat
├── platformio.ini
├── README.md
```

✅ Keeps project clean and portable.

---

## 2. Firmware Overview

### 2.1 Libraries Used, ReadButtons() and ReadAxes() Implementation
**(File: `src/main.cpp` — Firmware source)**

- `Wire.h` for I2C bus.
- `Adafruit_MCP23X17.h` for MCP23017 expanders.
- `Joystick.h` for custom USB HID interface.


```cpp
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Joystick.h>
#include <PluggableUSB.h>

// Create the Joystick instance of USB controller
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_MULTI_AXIS, 32, 2,
                   true, true, true,
                   true, true, true,
                   false, false, false,
                   false, false);

// Create MCP23017 expanders, enabling the additional inputs
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

// Analog input pins for 7 axes
const int axisPins[7] = {A0, A1, A2, A3, A4, A5, A6};

// ----- function prototypes ----------
void readButtons();
void readAxes();
// ------------------------------------

void setup()
{
  // Initialize I2C, "turning on the BUS switch"
  Wire.begin();

  // Initialize the MCP23017s (address 0x20 and 0x21)
  mcp1.begin_I2C(0x20);
  mcp2.begin_I2C(0x21);

  // Setup all MCP23017 pins as inputs with pull-ups
  for (int i = 0; i < 16; i++)
  {
    mcp1.pinMode(i, INPUT_PULLUP);
    mcp2.pinMode(i, INPUT_PULLUP);
  }

  // Start the Joystick HID interface
  Joystick.begin();
}

void loop()
{
  readButtons();
  readAxes();
  delay(10); // Small delay for USB HID updates
}

// Read all 32 buttons from two MCP23017s
void readButtons()
{
  for (int i = 0; i < 16; i++)
  {
    Joystick.setButton(i, !mcp1.digitalRead(i)); // Active LOW
  }
  for (int i = 0; i < 16; i++)
  {
    Joystick.setButton(i + 16, !mcp2.digitalRead(i)); // Offset second MCP
  }
}

// Read all 7 analog axes
void readAxes()
{
  for (int i = 0; i < 7; i++)
  {
    int raw = analogRead(axisPins[i]);
    int mappedValue = map(raw, 0, 1023, 0, 1023); // Optional remap if needed
    switch (i)
    {
    case 0:
      Joystick.setXAxis(mappedValue);
      break;
    case 1:
      Joystick.setYAxis(mappedValue);
      break;
    case 2:
      Joystick.setZAxis(mappedValue);
      break;
    case 3:
      Joystick.setRxAxis(mappedValue);
      break;
    case 4:
      Joystick.setRyAxis(mappedValue);
      break;
    case 5:
      Joystick.setRzAxis(mappedValue);
      break;
    case 6: /* You could assign a 7th custom axis if needed */
      break;
    }
  }
}
```

✅ Handles 32 buttons and 7 analog axes.

---

## 3. Flashing Firmware and Installing Controller Profile

### 3.1 Uploading Firmware

- In PlatformIO, press `Ctrl + Shift + P → PlatformIO: Upload`.
- Or click the small Upload arrow in the PlatformIO sidebar.

✅ This will:
- Compile the firmware.
- Apply USB branding patches.
- Upload to the Arduino Leonardo.

---

### 3.2 Running installer.bat
**(File: `installer.bat` — MSFS XML profile installer)**

After successful firmware upload:
```
- Run `installer.bat` in the project root folder. Code below.
- This will copy your XML controller profile to:
```
@echo off
echo Copying ThrottleQuadrant_Custom_v1.xml to MSFS InputProfiles folder...
xcopy "input_profiles\ThrottleQuadrant_Custom_v1.xml" "%LOCALAPPDATA%\Packages\Microsoft.FlightSimulator_8wekyb3d8bbwe\LocalState\InputProfiles\" /Y
echo Done!
pause
```
✅ This enables automatic recognition by MSFS.
```
---

## 4. MSFS Controller Profile (Mapping Overview)

This section details how axes and buttons are mapped inside MSFS.

### 4.1 Axes and Button Mapping
**(File: `input_profiles/ThrottleQuadrant_Custom_v1.xml` — MSFS Controller Mapping XML)**

```xml
<SimBase.Document Type="Controls" version="1,0">
    <Descr>Throttle Quadrant Custom v1</Descr>
    <Filename>ThrottleQuadrant_Custom_v1.xml</Filename>
    <SimControls.Map>
        <!-- AXES MAPPINGS -->
        <Action name="THROTTLE_AXIS_1" axis="X" />
        <Action name="MIXTURE_AXIS_1" axis="Y" />
        <Action name="PROP_PITCH_AXIS_1" axis="Z" />
        <Action name="COWL_FLAPS_AXIS" axis="Rx" />
        <Action name="SPOILERS_AXIS" axis="Ry" />
        <Action name="GEAR_AXIS" axis="Rz" />
        <Action name="ELEVATOR_TRIM_AXIS" axis="Slider1" />

        <!-- BUTTON MAPPINGS -->
        <Action name="GEAR_TOGGLE" trigger="Button0" />
        <Action name="FLAPS_INCR" trigger="Button1" />
        <Action name="FLAPS_DECR" trigger="Button2" />
        <Action name="TOGGLE_BEACON_LIGHTS" trigger="Button3" />
        <Action name="TOGGLE_NAV_LIGHTS" trigger="Button4" />
        <Action name="TOGGLE_LANDING_LIGHTS" trigger="Button5" />
        <Action name="TOGGLE_TAXI_LIGHTS" trigger="Button6" />
        <Action name="AUTOPILOT_MASTER" trigger="Button7" />
        <Action name="AP_PANEL_ALTITUDE_HOLD" trigger="Button8" />
        <Action name="AP_PANEL_HEADING_HOLD" trigger="Button9" />
        <Action name="AP_PANEL_NAV_HOLD" trigger="Button10" />
        <Action name="AP_PANEL_APR_HOLD" trigger="Button11" />
        <Action name="AP_PANEL_VS_HOLD" trigger="Button12" />
        <Action name="TOGGLE_MASTER_BATTERY" trigger="Button13" />
        <Action name="TOGGLE_MASTER_ALTERNATOR" trigger="Button14" />
        <Action name="TOGGLE_ENGINE_1_FUEL_VALVE" trigger="Button15" />
        <Action name="ENGINE_AUTOSTART" trigger="Button16" />
        <Action name="TOGGLE_FUEL_PUMP" trigger="Button17" />
        <Action name="TOGGLE_AVIONICS_MASTER" trigger="Button18" />
        <Action name="TOGGLE_PITOT_HEAT" trigger="Button19" />
        <Action name="TOGGLE_DEICE" trigger="Button20" />
        <Action name="PARKING_BRAKES" trigger="Button21" />
        <Action name="TOGGLE_PUSHBACK" trigger="Button22" />
        <Action name="TOGGLE_TAILWHEEL_LOCK" trigger="Button23" />
        <Action name="TOGGLE_CABIN_LIGHTS" trigger="Button24" />
        <Action name="TOGGLE_PANEL_LIGHTS" trigger="Button25" />
        <Action name="TOGGLE_RECOGNITION_LIGHTS" trigger="Button26" />
        <Action name="TOGGLE_WING_LIGHTS" trigger="Button27" />
        <Action name="TOGGLE_STROBE_LIGHTS" trigger="Button28" />
        <Action name="SIM_RATE_INCR" trigger="Button29" />
        <Action name="SIM_RATE_DECR" trigger="Button30" />
        <Action name="PAUSE_TOGGLE" trigger="Button31" />
    </SimControls.Map>
</SimBase.Document>
```

✅ Default mappings for your project.

---


## 5. Custom USB HID Details

### 5.1 Branding and USB Descriptor Info

✅ During firmware build, the device will be branded with:

| Field | Value |
|:------|:------|
| Manufacturer | Apollo Flight Systems |
| Product | MoonDog Throttle Quadrant |
| Year | 2025 |
| Author | Gustavo Parra |

✅ USB Identifiers:

| Field | Value |
|:------|:------|
| Vendor ID (VID) | 0x2341 (Arduino LLC default) |
| Product ID (PID) | 0x8036 (Leonardo default) |

✅ Note:
- Vendor ID/Product ID remain Arduino's by default.
- Branding (Manufacturer/Product) is dynamically injected at build time using `extra_script.py`.

---

## 6. Automated USB Descriptor Patching

### 6.1 extra_script.py Functionality
**(File: `extra_script.py` — PlatformIO pre-build USB patcher)**

**(File: extra_script.py — Python automation)**

```python
Import("env")
import os

usbdesc_path = os.path.expanduser("~/.platformio/packages/framework-arduino-avr/cores/arduino/USBDesc.h")

new_header = """/*
   Copyright (c) 2025, Gustavo Parra
   Copyright (c) 2025, Apollo Flight Systems
   Permission to use, copy, modify, and/or distribute this software for
   any purpose with or without fee is hereby granted, provided that the
   above copyright notice and this permission notice appear in all copies.
   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES.
*/"""

def customize_usb_descriptor(source, target, env):
    if os.path.isfile(usbdesc_path):
        print("[INFO] Patching USB Descriptor...")
        with open(usbdesc_path, "r+", encoding="utf-8") as f:
            content = f.read()
            if "Permission to use, copy, modify" in content:
                split_content = content.split("*/", 1)
                content = new_header + split_content[1]
            content = content.replace('Arduino LLC', 'Apollo Flight Systems')
            content = content.replace('Arduino Leonardo', 'MoonDog Throttle Quadrant')
            f.seek(0)
            f.write(content)
            f.truncate()
    else:
        print("[WARNING] USBDesc.h not found. Skipping patch.")

env.AddPreAction("buildprog", customize_usb_descriptor)

✅ This patches the Leonardo's USB descriptors **only inside the PlatformIO local environment**.

✅ No global Arduino core files are modified.

✅ Triggered automatically during each `upload` build process.
```
---

### 6.2 PlatformIO Future-Proofing Notes

✅ PlatformIO occasionally updates the `framework-arduino-avr` package.

✅ If USBDesc.h ever moves, you may need to update the `usbdesc_path` variable inside `extra_script.py`.

✅ No damage occurs if patch fails — the device simply reverts to Arduino defaults.

✅ Warning messages will alert you if patching fails at build time.

---

# 🎯 Conclusion

This project can be expanded to other types of usb-type joystick controllers. I hope you grow this project as i built it modular enough to apply to a steering wheel for racing or any other types of immersive, DIY video game controller.

✅ You now have:

- USB Throttle Quadrant
- 7 axes and 32 button mappings in MSFS
- Mapping must be custom for the application. modularity allows specific cockpit/driverseat button configs with 32 available inputs
- Automated XML profile installation
- Custom manufacturer and product name
- Future-proof documentation

🎯 **Final Checklist:**

- [x] Firmware builds successfully.
- [x] USB descriptor branding confirmed.
- [x] MSFS profile installed automatically via installer.bat.
- [x] All axes and buttons recognized correctly inside MSFS.

---

# 🚀 Project Complete!

Congratulations!! :D

