# USB HID

## A class for managing USB Human Interface Device (HID) devices.

---
## Usage

- `void init(uint8_t device_type)` 
  - Initializes the USB HID device with the Keyboard selected default.

 
- `void deinit()`
  - Deinitializes the USB HID device, but keeps the USB host running.


- `void device(uint8_t device)`
  - Sets the type of HID device to be used.
    - `device` can be one of the following:
        - `USB_HID_KEYBOARD`
        - `USB_HID_MOUSE`
        - `USB_HID_JOYSTICK`
        - `USB_HID_GAMEPAD`
        - `USB_HID_CUSTOM`
  

- `void sendKey(char key)`
  - Send a key press event to the host (Instantly) without pressing or releasing.
    - `key` can be a character representing the key to be sent. `("A")`


---

- More functions TODO:
    - `void sendReport(uint8_t* report, size_t length)`
        - Send a HID report to the host.
    
    - `void setReportCallback(void (*callback)(uint8_t* report, size_t length))`
        - Set a callback function to handle incoming HID reports from the host.