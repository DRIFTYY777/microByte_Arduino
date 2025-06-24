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

    /// @brief Initialize the USB HID a subsystem with a default device (keyboard).
    void init();

    /// @brief Deinitialize the USB HID subsystem based on the current device.
    void deinit();

    /// @brief Set the current device type for USB HID operations. Like keyboard, mouse, gamepad, etc.
    void device(uint8_t device);

    /* Keyboard Methods */
    /// @brief Send a key press event to the current USB HID device. (INSTANTLY)
    void sendKey(char key);

    /// @brief Send a string to the current USB HID device. (INSTANTLY)
    void sendString(const char* str);

    /// @brief Press a key on the current USB HID device.
    void keyPress(uint8_t key);

    /// @brief Release a key on the current USB HID device.
    void keyRelease(uint8_t key);

    /* Mouse Methods */
    /// @brief Move the mouse cursor by the specified x and y offsets, with optional wheel and pan movements.
    void moveMouse(int8_t x, int8_t y, int8_t wheel = 0, int8_t pan = 0);

    /// @brief Perform a mouse click with the specified button.
    void mouseClick(uint8_t button);

    /// @brief Check if a mouse button is currently pressed.
    void mousePress(uint8_t button);

    /// @brief Release a mouse button.
    void mouseRelease(uint8_t button);

};
#endif //USBHID_H
