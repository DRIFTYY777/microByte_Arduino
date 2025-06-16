//
// Created by dhima on 16-06-2025.
//

#include "I2cManager.h"

#include <esp_err.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "I2C_MANAGER";


static i2c_bus_state_t i2c_buses[I2C_NUM_MAX] = {0};


