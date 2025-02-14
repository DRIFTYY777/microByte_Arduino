#include "MAX17048.h"
#include <esp_err.h>
#include "driver/i2c.h"
#include <components/system_config/system_config.h>
#include <esp32-hal-log.h>

static const char *TAG = "MAX17048";

bool MAX17048::MAX17048_init(void)
{
    // Init I2C bus as master
    esp_err_t ret;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = MAX17048_CLK_SPEED;
    ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C parameter configuration failed");
        return false;
    }
    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C driver installation failed");
        return false;
    }
    // Check if the device is connected
    if (MAX17048_I2C_write(I2C_NUM_0, 0x00, 1) == -1)
    {
        ESP_LOGE(TAG, "MAX17048 not detected");
        return false;
    }
    ESP_LOGI(TAG, "MAX17048 detected");
    return true;
}

uint16_t MAX17048::MAX17048_readVoltage(void)
{
    uint8_t data[2] = {MAX17048_VCELL, 0};
    MAX17048_I2C_write(I2C_NUM_0, data, 1);
    MAX17048_I2C_read(I2C_NUM_0, data, 2);
    return (data[0] << 4) | (data[1] >> 4);
}

uint8_t MAX17048::MAX17048_readSOC(void)
{
    uint8_t data[1] = {MAX17048_SOC};
    MAX17048_I2C_write(I2C_NUM_0, data, 1);
    MAX17048_I2C_read(I2C_NUM_0, data, 1);
    return data[0];
}

uint8_t MAX17048::MAX17048_readVersion(void)
{
    uint8_t data[1] = {MAX17048_VERSION};
    MAX17048_I2C_write(I2C_NUM_0, data, 1);
    MAX17048_I2C_read(I2C_NUM_0, data, 1);
    return data[0];
}

uint8_t MAX17048::MAX17048_readStatus(void)
{
    uint8_t data[1] = {MAX1708_STATUS};
    MAX17048_I2C_write(I2C_NUM_0, data, 1);
    MAX17048_I2C_read(I2C_NUM_0, data, 1);
    return data[0];
}

void MAX17048::MAX17048_reset(void)
{
    uint8_t data[2] = {MAX17048_COMMAND, MAX17048_RESET};
    MAX17048_I2C_write(I2C_NUM_0, data, 2);
}

int8_t MAX17048::MAX17048_I2C_write(uint8_t I2C_bus, uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write(cmd, data, size, 0x1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_bus, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C write failed");
        return -1;
    }
    return 1;
}

int8_t MAX17048::MAX17048_I2C_read(uint8_t I2C_bus, uint8_t *data, size_t size)
{
    if (size == 0)
    {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_READ, 0x1);
    if (size > 1)
    {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_bus, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C read failed");
        return -1;
    }
    return ret;
}

int8_t MAX17048::MAX17048_I2C_writeReg(uint8_t I2C_bus, uint8_t reg, uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, reg, 0x1);
    i2c_master_write(cmd, data, size, 0x1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_bus, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C write register failed");
        return -1;
    }
    return 1;
}

MAX17048 max17048;
