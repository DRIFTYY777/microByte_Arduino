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

void usbHID::init(uint8_t device_type)
{
    switch (device_type)
    {
        case DEVICE_TYPE_KEYBOARD:
            // Initialize USB HID Keyboard
            initKeyboard();
            current_device = DEVICE_TYPE_KEYBOARD;
            ESP_LOGE(TAG, "Keyboard initialization not implemented yet");
            break;
        case DEVICE_TYPE_MOUSE:
            // Initialize USB HID Mouse
            initMouse();
            current_device = DEVICE_TYPE_MOUSE;
            ESP_LOGE(TAG, "Mouse initialization not implemented yet");
            break;
        case DEVICE_TYPE_GAMEPAD:
            // Initialize USB HID Gamepad
            initGamepad();
            current_device = DEVICE_TYPE_GAMEPAD;
            ESP_LOGE(TAG, "Gamepad initialization not implemented yet");
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

void usbHID::sendKey(const char key)
{
    Keyboard.write(key);
}

//usbHID usbHid;

