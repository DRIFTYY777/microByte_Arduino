//
// Created by dhima on 16-06-2025.
//

#include "I2cManager.h"

#include <esp_err.h>

void I2C_Bus_Manager_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, uint32_t clk_speed) {
    // init I2C bus with specified SDA, SCL pins and clock speed

    i2c_master_bus_config_t  i2c_config = {
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .master.clk_speed = clk_speed,
        .mode = I2C_MODE_MASTER,
    };



}
