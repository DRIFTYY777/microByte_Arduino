//
// Created by dhima on 24-06-2025.
//

#ifndef USBHID_H
#define USBHID_H
#include <cstdint>

#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"
#include "USBHIDGamepad.h"
#include "USBHID.h"

enum DEVICE_TYPE {
    DEVICE_TYPE_KEYBOARD = 0,
    DEVICE_TYPE_MOUSE,
    DEVICE_TYPE_GAMEPAD,
    DEVICE_TYPE_CUSTOM
};

class usbHID {
private:
    uint8_t current_device = 0;

    // USB HID devices
    USBHIDKeyboard Keyboard;
    USBHIDMouse Mouse;
    USBHIDGamepad Gamepad;

    // Initialize different USB HID devices
    void initKeyboard();
    void initMouse();
    void initGamepad();
    //void initCustomDevice( );

    // Deinit the USB HID devices
    void deinitKeyboard();
    void deinitMouse();
    void deinitGamepad();
    //void deinitCustomDevice();

public:
    void init(uint8_t device_type);
    void deinit();

    /* Keyboard methods */
    void sendKey(char key);


};

//extern usbHID usbHid;



#endif //USBHID_H
