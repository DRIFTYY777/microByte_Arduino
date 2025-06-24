//
// Created by dhima on 24-06-2025.
//

#include "usbHID.h"

#include <esp_log.h>

static const char *TAG = "USBHID";

void usbHID::initKeyboard()
{
    Keyboard.begin();
}
void usbHID::initMouse()
{
    Mouse.begin();
}
void usbHID::initGamepad()
{
    Gamepad.begin();
}
void usbHID::deinitKeyboard()
{
    Keyboard.end();
}
void usbHID::deinitMouse()
{
    Mouse.end();
}
void usbHID::deinitGamepad()
{
    Gamepad.end();
}

void usbHID::init()
{
    // by default init the keyboard
    initKeyboard();
    current_device = DEVICE_TYPE_KEYBOARD; // Set the current device to keyboard
}

void usbHID::deinit()
{
    switch (current_device)
    {
        case DEVICE_TYPE_KEYBOARD:
            // Deinitialize USB HID Keyboard
            deinitKeyboard();
            break;
        case DEVICE_TYPE_MOUSE:
            // Deinitialize USB HID Mouse
            deinitMouse();
            break;
        case DEVICE_TYPE_GAMEPAD:
            // Deinitialize USB HID Gamepad
            deinitGamepad();
            break;
        case DEVICE_TYPE_CUSTOM:
            // Deinitialize USB HID Custom Device
            //deinitCustomDevice();
            break;
        default:
            ESP_LOGE(TAG, "Invalid device type for deinitialization");
            break;
    }
    current_device = 0; // Reset a device type after deinitialization
}


void usbHID::device(const uint8_t device)
{
    deinit(); // Deinitialize the current device before switching

    switch (device)
    {
    case DEVICE_TYPE_KEYBOARD:
        // Initialize USB HID Keyboard
        initKeyboard();
        current_device = DEVICE_TYPE_KEYBOARD;
        ESP_LOGE(TAG, "Keyboard is initialized");
        break;
    case DEVICE_TYPE_MOUSE:
        // Initialize USB HID Mouse
        initMouse();
        current_device = DEVICE_TYPE_MOUSE;
        ESP_LOGE(TAG, "Mouse is initialized");
        break;
    case DEVICE_TYPE_GAMEPAD:
        // Initialize USB HID Gamepad
        initGamepad();
        current_device = DEVICE_TYPE_GAMEPAD;
        ESP_LOGE(TAG, "Gamepad is initialized");
        break;
    case DEVICE_TYPE_CUSTOM:
        // Initialize USB HID Custom Device
        current_device = DEVICE_TYPE_CUSTOM;
        ESP_LOGE(TAG, "Custom HID device initialization not implemented yet");
        break;
    default:
        ESP_LOGE(TAG, "Invalid device type");
        break;
    }
}

void usbHID::sendKey(const char key)
{
    Keyboard.write(key);
}

void usbHID::sendString(const char* str)
{
    // send string
    while (*str) {
        Keyboard.write(*str++);
    }
}

void usbHID::keyPress(uint8_t key)
{
    Keyboard.press(key);
}

void usbHID::keyRelease(uint8_t key)
{
    Keyboard.release(key);
}

void usbHID::moveMouse(int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
    Mouse.move(x, y, wheel, pan);
}

void usbHID::mouseClick(uint8_t button)
{
    Mouse.click(button);
}

void usbHID::mousePress(uint8_t button)
{
    Mouse.press(button); // press LEFT by default
}

void usbHID::mouseRelease(uint8_t button)
{
    Mouse.release(button); // press LEFT by default
}

