# usbHAL

## Overview

`usbHAL` is a hardware abstraction layer for USB functionality on ESP32-S3, providing a unified interface for multiple USB device classes:
- **CDC** (Communication Device Class)
- **HID** (Human Interface Device: keyboard, mouse, gamepad)
- **MSC** (Mass Storage Class)
- **Firmware Update**
- **Debug**

It manages initialization, deinitialization, and mode switching for these USB device types.

---

## Architecture

- **Class:** `USBHAL`
    - Manages USB mode state and delegates to subcomponents.
    - Contains instances of:
        - `usbHID` (HID device management)
        - `usbSTORAGE` (Mass storage management)
- **Subcomponents:**
    - `usbHID`: Handles keyboard, mouse, and gamepad HID devices.
    - `usbSTORAGE`: Handles USB mass storage using SD card as backend.

---

## API Reference

### USBHAL

- **Methods:**
    - `bool init()`: Initialize the USB subsystem.
    - `void deinit()`: Deinitialize the current USB mode only but not the entire USB host.
    - `void modes(uint8_t mode)`: Set the USB mode (CDC, HID, MSC, etc.).

- **Members:**
    - `usbHID usbHid`: HID device manager.
    - `USBSTORAGE usbStorage`: Mass storage manager.

- **Modes:**
    - `USB_MODE_NONE`
    - `USB_MODE_CDC`
    - `USB_MODE_HID`
    - `USB_MODE_MSC`
    - `USB_MODE_UPDATE`
    - `USB_MODE_DEBUG`

### usbHID

- **Methods:**
    - `void init()`: Initialize as keyboard by default.
    - `void deinit()`: Deinitialize current HID device.
    - `void device(uint8_t device)`: Switch HID device type.
    - `void sendKey(char key)`: Send key press to keyboard device.

- **Device Types:**
    - `DEVICE_TYPE_KEYBOARD`
    - `DEVICE_TYPE_MOUSE`
    - `DEVICE_TYPE_GAMEPAD`
    - `DEVICE_TYPE_CUSTOM`

### USBSTORAGE

- **Methods:**
    - `bool init()`: Initialize USB mass storage (SD card backend).
    - `bool deinit()`: Deinitialize mass storage.

---

## Usage Example

```cpp
#include "usbHal.h"

void setup() {
    usbHal.init();                // Initialize USB subsystem
    usbHal.modes(USB_MODE_HID);   // Set mode to HID (keyboard by default)
    usbHal.usbHid.sendKey('A');   // Send 'A' keypress
}

void switchToMSC() {
    usbHal.deinit();              // Deinitialize current mode
    usbHal.modes(USB_MODE_MSC);   // Switch to Mass Storage mode
}
```

---

## Extending

- **Add new USB modes:**  
  Extend the `USB_MODES` enum and update `USBHAL::modes()` and `deinit()` to handle new modes.
- **Add new HID devices:**  
  Extend `DEVICE_TYPE` enum and implement corresponding init/deinit methods in `usbHID`.

---

## File Structure

- `usbHal.h` / `usbHal.cpp`: Main HAL class and logic.
- `usbHID.h` / `usbHID.cpp`: HID device management.
- `usbStorage.h` / `usbStorage.cpp`: Mass storage management.

---

## Dependencies

- ESP-IDF or Arduino-ESP32 core
- SD card driver for mass storage
- USB device stack

---

## Author

Created by DRIFTYY777, 2025

---