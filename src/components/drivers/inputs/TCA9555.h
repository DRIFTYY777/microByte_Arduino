#ifndef TCA9555_H
#define TCA9555_H

#include <cstdint>
#include <stddef.h>


#define CONFIG_REG 0x06 // Configuration register
#define READ_REG 0x00 // Input register

#define I2C_ACK_VAL 0   // Typically 0 for ACK
#define I2C_NACK_VAL 1  // Typically 1 for NACK


class TCA9555
{
private:
    static int8_t TCA955_I2C_write(uint8_t I2C_bus, uint8_t *data, size_t size);
    static int8_t TCA955_I2C_read(uint8_t I2C_bus, uint8_t *data, size_t size);

public:
    bool TCA955_init(void);
    bool TCA9555_pinMode(uint8_t pin);
    int16_t TCA9555_readInputs(void);
};

extern TCA9555 tca9555;


#endif // TCA9555_H
