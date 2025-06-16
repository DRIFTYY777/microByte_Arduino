//
// Created by dhima on 16-06-2025.
//

#ifndef I2CMANAGER_H
#define I2CMANAGER_H
#include <hal/gpio_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void I2C_Bus_Manager_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, uint32_t clk_speed);

#ifdef __cplusplus
}
#endif

#endif //I2CMANAGER_H
