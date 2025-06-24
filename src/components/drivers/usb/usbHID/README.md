# USB HID

## A class for managing USB Human Interface Device (HID) devices.

---
## Usage

- `void init(uint8_t device_type)` 
  - Initializes the USB HID device with the specified device type.
  - `device_type` can be one of the following:
    - `USB_HID_KEYBOARD`
    - `USB_HID_MOUSE`
    - `USB_HID_CUSTOM`
    - `USB_HID_GAMEPAD`
    - `USB_HID_CONSUMER`
  
- `void deinit()`
  - Deinitializes the USB HID device, but keeps the USB host running.

---


- More functions TODO:
    - `void sendReport(uint8_t* report, size_t length)`
        - Send a HID report to the host.
    
    - `void setReportCallback(void (*callback)(uint8_t* report, size_t length))`
        - Set a callback function to handle incoming HID reports from the host.