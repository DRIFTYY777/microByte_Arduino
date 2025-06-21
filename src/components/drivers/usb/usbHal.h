//
// Created by dhima on 18-06-2025.
//

#ifndef USBHAL_H
#define USBHAL_H
#include <cstdint>

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
    USB_MODE_SERIAL,
    USB_MODE_UPDATE,
    USB_MODE_DEBUG,
};

enum USB_HID_DEVICE {
    USB_HID_KEYBOARD = 0,
    USB_HID_MOUSE,
    USB_HID_GAMEPAD,
    USB_HID_GENERIC
};



class USBHAL {



};

extern USBHAL usbHal;


#endif //USBHAL_H
