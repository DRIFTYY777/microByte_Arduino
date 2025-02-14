
#ifndef MAX17048_H
#define MAX17048_H
#include <cstdint>
#include <stddef.h>

#define MAX17048_ADDR 0x36 // MAX17048 I2C address

#define MAX17048_VCELL 0x02   // Voltage register
#define MAX17048_SOC 0x04     // State of charge register
#define MAX17048_MODE 0x06    // Mode register
#define MAX17048_VERSION 0x08 // Version register
#define MAX17048_HIBRT 0x0A   // Hibernate register
#define MAX17048_CONFIG 0x0C  // Configuration register
#define MAX17048_VALRT 0x14   // Voltage alert register
#define MAX17048_CRATE 0x16   // Approximate charge or discharge rate of the battery
#define MAX17048_RESET 0x18   // Reset command
#define MAX1708_STATUS 0x1A   // Status register
#define MAX17048_COMMAND 0xFE // Command register

class MAX17048
{
public:
    bool MAX17048_init(void);
    uint16_t MAX17048_readVoltage(void);
    uint8_t MAX17048_readSOC(void);
    uint8_t MAX17048_readVersion(void);
    uint8_t MAX17048_readStatus(void);
    void MAX17048_reset(void);

private:
    int8_t MAX17048_I2C_write(uint8_t I2C_bus, uint8_t *data, size_t size);
    int8_t MAX17048_I2C_read(uint8_t I2C_bus, uint8_t *data, size_t size);
    int8_t MAX17048_I2C_writeReg(uint8_t I2C_bus, uint8_t reg, uint8_t *data, size_t size);
};

extern MAX17048 max17048;

#endif