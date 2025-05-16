#if !defined(SPI_MANAGER_H)
#define SPI_MANAGER_H

#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool spi_bus_manager_init(spi_host_device_t host, int mosi, int miso, int sclk, size_t max_transfer_sz);
    esp_err_t spi_bus_manager_add_device(spi_host_device_t host, spi_device_interface_config_t *devcfg, spi_device_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // SPI_MANAGER_H)
