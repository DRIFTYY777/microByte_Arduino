//
// Created by dhima on 07-06-2025.
//

#ifndef PCA9555_H
#define PCA9555_H


#include <cstdint>

class PCA9555 {
private:
#define INPUT_PORT_0 0x00
#define INPUT_PORT_1 0x01
#define OUTPUT_PORT_0 0x02
#define OUTPUT_PORT_1 0x03
#define POLARITY_INV_0 0x04
#define POLARITY_INV_1 0x05
#define CONFIG_0 0x06
#define CONFIG_1 0x07

#define NORMAL 0x00
#define INVERTED 0xFF

#define INPUT 0x00
#define OUTPUT 0xFF
public:
    bool init();
    static bool pinMode(uint8_t pin, bool isInput);
    static int16_t read();
};

extern PCA9555 pca9555;


#endif //PCA9555_H
