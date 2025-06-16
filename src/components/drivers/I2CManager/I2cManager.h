#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Legacy I2C API compatible with Arduino ESP32
typedef struct {
    bool initialized;
    i2c_port_t port;
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint32_t freq_hz;
    SemaphoreHandle_t mutex;
} i2c_bus_state_t;

// Initialize I2C bus
bool i2c_bus_manager_init(i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t freq_hz);

// Add device (simplified for legacy API)
esp_err_t i2c_bus_manager_add_device(i2c_port_t port, uint8_t device_address);

// Remove device (simplified for legacy API)
esp_err_t i2c_bus_manager_remove_device(i2c_port_t port, uint8_t device_address);

// Deinitialize I2C bus
esp_err_t i2c_bus_manager_deinit(i2c_port_t port);

// Check if bus is initialized
bool i2c_bus_manager_is_initialized(i2c_port_t port);

// Scan I2C bus for devices
esp_err_t i2c_bus_manager_scan(i2c_port_t port, uint8_t *found_devices, size_t max_devices, size_t *found_count);

// Basic I2C operations using legacy API
esp_err_t i2c_manager_write(i2c_port_t port, uint8_t device_addr, const uint8_t *write_buffer, size_t write_size);
esp_err_t i2c_manager_read(i2c_port_t port, uint8_t device_addr, uint8_t *read_buffer, size_t read_size);
esp_err_t i2c_manager_write_read(i2c_port_t port, uint8_t device_addr,
                                 const uint8_t *write_buffer, size_t write_size,
                                 uint8_t *read_buffer, size_t read_size);

// Register operations
esp_err_t i2c_manager_write_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t data);
esp_err_t i2c_manager_read_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t *data);
esp_err_t i2c_manager_write_reg16(i2c_port_t port, uint8_t device_addr, uint16_t reg_addr, uint8_t data);
esp_err_t i2c_manager_read_reg16(i2c_port_t port, uint8_t device_addr, uint16_t reg_addr, uint8_t *data);

// Multi-byte register operations
esp_err_t i2c_manager_write_reg_bytes(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr,
                                      const uint8_t *data, size_t data_len);
esp_err_t i2c_manager_read_reg_bytes(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr,
                                     uint8_t *data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif // I2C_MANAGER_H