//
// Created by dhima on 06-06-2025.
//

#ifndef MAX17048G_H
#define MAX17048G_H

#include <Wire.h>


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


class MAX17048G {

private:
    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    uint8_t _address;
    TwoWire* _wire;
    uint8_t _error;
    uint8_t _type;

public:
    MAX17048G(uint8_t address, TwoWire *wire = &Wire);

    bool init();





};



#endif //MAX17048G_H
