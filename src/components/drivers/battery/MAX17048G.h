//
// Created by dhima on 06-06-2025.
//

#ifndef MAX17048G_H
#define MAX17048G_H

#include <Wire.h>

class MAX17048G {

// Registers address Table 1
#define MAX17048G_VCELL 0x02
#define MAX17048G_SOC 0x04
#define MAX17048G_MODE 0x06
#define MAX17048G_VERSION 0x08
#define MAX17048G_HIBRT 0x0A

// Registers address Table 2
#define MAX17048G_CONFIG 0x0C
#define MAX17048G_VALRT 0x14
#define MAX17048G_CRATE 0x16
#define MAX17048G_VRESET 0x18
#define MAX17048G_STATUS 0x1A
#define MAX17048G_CMD 0xFE



private:
    TwoWire *i2cPort; // Pointer to the I2C port
    uint8_t i2cAddress; // I2C address of the MAX17048G

    void writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);

    // Helper function to convert raw voltage to float
    float convertVoltage(uint16_t rawValue);

public:
    explicit MAX17048G(TwoWire *i2cPort = &Wire, const uint8_t i2cAddress = 0x36) : i2cPort(i2cPort), i2cAddress(i2cAddress) {}

    void begin(); // Initialize the MAX17048G
    void reset();
    void setConfig(uint16_t config);
    void adc();
    void hibernate();
    float getVoltage();
    float getSOC(); // State of Charge
    float getCapacity(); // Capacity in mAh
    float getDesignCapacity(); // Design Capacity in mAh
    float getFullCapacity(); // Full Capacity in mAh
    float getAverageCurrent(); // Average Current in mA
    float getCurrent(); // Current in mA
    float getPower(); // Power in mW
    float getTemperature(); // Temperature in Celsius





};



#endif //MAX17048G_H
