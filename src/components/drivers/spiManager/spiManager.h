#if !defined(SPI_MANAGER_H)
#define SPI_MANAGER_H

#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C"
{
#endif


    bool spi_bus_manager_init(spi_host_device_t host, int mosi, int miso, int sclk, size_t max_transfer_sz);
    esp_err_t spi_bus_manager_add_device2(spi_host_device_t host, int cs_pin,  spi_device_interface_config_t *devcfg, spi_device_handle_t *handle);
    esp_err_t spi_bus_manager_add_device(spi_host_device_t host, spi_device_interface_config_t *devcfg, spi_device_handle_t *handle);

    // Remove a device from SPI bus
    esp_err_t spi_bus_manager_remove_device(spi_device_handle_t handle);

    // Deinitialize SPI bus
    esp_err_t spi_bus_manager_deinit(spi_host_device_t host);

    // Check if SPI bus is initialized
    bool spi_bus_manager_is_initialized(spi_host_device_t host);

    // Get device count on a bus
    int spi_bus_manager_get_device_count(spi_host_device_t host);

    void set_max_transfer_size(spi_host_device_t host, size_t max_transfer_sz);

    // return all devices on a bus
    esp_err_t spi_bus_manager_get_devices(spi_host_device_t host, spi_device_handle_t *handles, size_t max_count, size_t *count);

    // Utility functions for common SPI operations
    esp_err_t spi_manager_transmit(spi_device_handle_t handle, const uint8_t *tx_data, size_t length);
    esp_err_t spi_manager_receive(spi_device_handle_t handle, uint8_t *rx_data, size_t length);
    esp_err_t spi_manager_transmit_receive(spi_device_handle_t handle, const uint8_t *tx_data, uint8_t *rx_data, size_t length);



#ifdef __cplusplus
}
#endif

#endif // SPI_MANAGER_H)
