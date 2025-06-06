//
// Created by dhima on 06-06-2025.
//

#include "MAX17048G.h"


void MAX17048G::writeRegister(const uint8_t reg, const uint16_t value) {

    i2cPort->beginTransmission(i2cAddress);
    i2cPort->write(reg);
    i2cPort->write((value >> 8) & 0xFF); // Write high byte
    i2cPort->write(value & 0xFF);        // Write low byte
    i2cPort->endTransmission();
}

uint16_t MAX17048G::readRegister(const uint8_t reg) {
    i2cPort->beginTransmission(i2cAddress);
    i2cPort->write(reg);
    i2cPort->endTransmission();

    i2cPort->requestFrom(i2cAddress, static_cast<uint8_t>(2));
    if (i2cPort->available() == 2) {
        uint16_t value = (i2cPort->read() << 8) | i2cPort->read();
        return value;
    }
    return 0; // Return 0 if read fails
}

void MAX17048G::begin() {
    i2cPort->begin();
    reset();
}
