#include "usbHal.h"
#include "USB.h"

#include "usbStorage/usbStorage.h"

#include "HardwareSerial.h"

bool USBHAL::init()
{
    // this function initializes the USB subsystem
    if (!USB.begin())
    {
        Serial.println("USB initialization failed!");
        return false;
    }
    Serial.println("USB initialized successfully");
    usbMode = USB_MODE_NONE;
    return true;
}

void USBHAL::deinit()
{
    switch (usbMode)
    {
        case USB_MODE_CDC:
            //USB.endCDC();
            Serial.println("USB CDC deinitialized");
            break;
        case USB_MODE_HID:
            usbHid.deinit(); // Deinitialize HID
            Serial.println("USB HID deinitialized");
            break;
        case USB_MODE_MSC:
            usbStorage.deinit(); // Deinitialize USB Mass Storage
            Serial.println("USB MSC deinitialized");
            break;
        case USB_MODE_UPDATE:
            //USB.endUpdate();
            Serial.println("USB Update deinitialized");
            break;
        case USB_MODE_DEBUG:
            //USB.endDebug();
            Serial.println("USB Debug deinitialized");
            break;
        default:
            Serial.println("USB deinitialized");
            break;
    }
    usbMode = USB_MODE_NONE; // Reset USB mode
}

void USBHAL::modes(uint8_t mode)
{
    // Set the USB mode
    switch (mode)
    {
    case USB_MODE_CDC:
        //USB.setMode(USB_MODE_CDC);
        Serial.println("USB Mode set to CDC");
        usbMode = USB_MODE_CDC;
        break;
    case USB_MODE_HID:
        usbHid.init(DEVICE_TYPE_KEYBOARD); // Initialize HID with keyboard as default
        Serial.println("USB Mode set to HID");
        usbMode = USB_MODE_HID;
        break;
    case USB_MODE_MSC:
        usbStorage.init(); // Initialize USB Mass Storage
        Serial.println("USB Mode set to MSC");
        usbMode = USB_MODE_MSC;
        break;
    case USB_MODE_UPDATE:
        //USB.setMode(USB_MODE_UPDATE);
        Serial.println("USB Mode set to Update");
        usbMode = USB_MODE_UPDATE;
        break;
    case USB_MODE_DEBUG:
        //USB.setMode(USB_MODE_DEBUG);
        Serial.println("USB Mode set to Debug");
        usbMode = USB_MODE_DEBUG;
        break;
    default:
        //USB.setMode(USB_MODE_NONE);
        Serial.println("USB Mode set to None");
        usbMode = USB_MODE_NONE;
        break;
    }
}


USBHAL usbHal;
