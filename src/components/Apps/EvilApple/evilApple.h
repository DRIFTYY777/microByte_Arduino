#ifndef EVILAPPLE_H
#define EVILAPPLE_H

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_arduino_version.h>
#include "devices.hpp"

#define MAX_TX_POWER ESP_PWR_LVL_P21 // ESP32C3 ESP32C2 ESP32S3

class EvilApple
{
public:
    void begin();
    void startAdvertising();
    void stopAdvertising();

private:
    BLEAdvertising *pAdvertising; // global variable
    uint32_t delayMilliseconds = 1000;
};

extern EvilApple evilApple;

#endif