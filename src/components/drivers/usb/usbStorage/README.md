# USB MSC 

A class for managing USB Mass Storage Class (MSC) devices.

---

## Usage

There are 2 public methods `init()` and `deinit()`.
- `init()` Only initializes the USB MSC device. don't forget to call `USB.begin()` before calling this method.
- `deinit()` Deinitializes the USB MSC (ONLY). but still keeps the USB host running.

---

## Note
- This class is deeply integrated with the SD card driver
- `sd_card.readRAW_` these function basically redirect of `SD.readRAW` function from arduino.
- `sd_card.writeRAW_` these function basically redirect of `SD.writeRAW` function from arduino.

---





