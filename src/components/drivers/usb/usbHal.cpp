#include "usbHal.h"
#include "USB.h"

#include "HardwareSerial.h"

bool USBHAL::init()
{
    // this function initializes the USB subsystem
    if (USB.begin())
    {
        return true;
    }
    return false;
}

void USBHAL::modes(uint8_t mode)
{
    // Set the USB mode
    switch (mode)
    {
    case USB_MODE_CDC:
        //USB.setMode(USB_MODE_CDC);
        Serial.println("USB Mode set to CDC");
        break;
    case USB_MODE_HID:
        //USB.setMode(USB_MODE_HID);
        Serial.println("USB Mode set to HID");
        break;
    case USB_MODE_MSC:
        //USB.setMode(USB_MODE_MSC);
        Serial.println("USB Mode set to MSC");
        break;
    case USB_MODE_UPDATE:
        //USB.setMode(USB_MODE_UPDATE);
        Serial.println("USB Mode set to Update");
        break;
    case USB_MODE_DEBUG:
        //USB.setMode(USB_MODE_DEBUG);
        Serial.println("USB Mode set to Debug");
        break;
    default:
        //USB.setMode(USB_MODE_NONE);
        Serial.println("USB Mode set to None");
        break;
    }
}


USBHAL usbHal;
