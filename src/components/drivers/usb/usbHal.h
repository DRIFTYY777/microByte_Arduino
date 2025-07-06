//
// Created by dhima on 18-06-2025.
//

#ifndef USBHAL_H
#define USBHAL_H
#include <cstdint>
#include <USBCDC.h>

#include "usbHID/usbHID.h"
#include "usbStorage/usbStorage.h"


/*
 * usbCDC - USB Communication Device Class
 * usbHID - USB Human Interface Device
 * usbMSC - USB Mass Storage Class
 * usbSerial - USB Serial Device
 * usbUpdate - USB Firmware Update
 * debug - Debugging USB operations
 */

enum USB_STATE {
    USB_STATE_UNINITIALIZED = 0,
    USB_STATE_INITIALIZED,
    USB_STATE_CONNECTED,
    USB_STATE_SUSPENDED,
    USB_STATE_DISCONNECTED,
    USB_STATE_ERROR
};

enum USB_MODES {
    USB_MODE_NONE = 0,
    USB_MODE_CDC ,
    USB_MODE_HID,
    USB_MODE_MSC,
    USB_MODE_UPDATE,
    USB_MODE_DEBUG,
};


class USBHAL {
private:
    uint8_t usbMode = USB_MODE_NONE;

    /// @brief USB Mass Storage instance
    USBSTORAGE usbStorage;



    void initCDC();
    void deinitCDC();


public:

    /// @brief USB HID instance
    /// This is used for keyboard, mouse, gamepad, etc.
    usbHID usbHid;

    /// @brief USB Communication Device Class instance
    /// This is used for serial communication over USB.
    USBCDC USBSerial;


    /// @brief Initialize the USB HAL subsystem.
    bool init();

    /// @brief Deinitialize the USB Device subsystem. Not the USB Host subsystem.
    void deinit();

    /// @brief Set the USB mode for the HAL subsystem.
    void modes(uint8_t mode);
};


extern USBHAL usbHal;


#endif //USBHAL_H
