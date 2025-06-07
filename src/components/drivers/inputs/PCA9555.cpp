//
// Created by dhima on 07-06-2025.
//

#include "PCA9555.h"
#include <components/system_config/system_config.h>
#include <Wire.h>

static const char *TAG = "PCA9555";

bool PCA9555::init() {
    Wire.begin(); // Use default SDA/SCL or specify pins: Wire.begin(SDA, SCL);
    Wire.beginTransmission(PCA_dev_address);
    const uint8_t error = Wire.endTransmission();
    if (error != 0) {
        ESP_LOGD(TAG, "PCA9555 not detected, error code: %d", error);
        return false;
    }
    ESP_LOGD(TAG, "PCA9555 detected at address 0x%02X", PCA_dev_address);
    return true;
}

bool PCA9555::pinMode(uint8_t pin, bool isInput) {
    if (pin >= 16) {
        ESP_LOGE(TAG, "Pin out of range (0-15)");
        return false;
    }

    // Read current config
    Wire.beginTransmission(PCA_dev_address);
    Wire.write(CONFIG_0);
    Wire.endTransmission(false);
    Wire.requestFrom(PCA_dev_address, 2);
    uint8_t config0 = Wire.read();
    uint8_t config1 = Wire.read();
    if (isInput)
        (pin < 8 ? config0 : config1) |= (1 << (pin % 8));
    else
        (pin < 8 ? config0 : config1) &= ~(1 << (pin % 8));
    // Write back config
    Wire.beginTransmission(PCA_dev_address);
    Wire.write(CONFIG_0);
    Wire.write(config0);
    Wire.write(config1);
    if (Wire.endTransmission() != 0) {
        ESP_LOGE(TAG, "Error setting pinMode for pin %d", pin);
        return false;
    }
    return true;
}

int16_t PCA9555::read() {
    Wire.beginTransmission(PCA_dev_address);
    Wire.write(INPUT_PORT_0);
    Wire.endTransmission(false);
    Wire.requestFrom(PCA_dev_address, 2);
    uint8_t input0 = Wire.read();
    uint8_t input1 = Wire.read();
    return (input1 << 8) | input0;
    ESP_LOGD("PCA9555", "Inputs read: 0x%04X", (input1 << 8) | input0);
}
PCA9555 pca9555;