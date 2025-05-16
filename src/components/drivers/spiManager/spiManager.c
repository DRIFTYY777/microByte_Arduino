// spi_bus_manager.c
#include "spiManager.h"
#include "esp_log.h"

static const char *TAG = "SPI_BUS_MANAGER";

static bool spi_bus_initialized[SOC_SPI_PERIPH_NUM] = {false};

bool spi_bus_manager_init(spi_host_device_t host, int mosi, int miso, int sclk, size_t max_transfer_sz)
{

    if (spi_bus_initialized[host])
    {
        ESP_LOGI(TAG, "SPI%d already initialized", host);
        return true;
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = max_transfer_sz,
        .flags = 0};

    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI%d: %s", host, esp_err_to_name(ret));
        return false;
    }

    spi_bus_initialized[host] = true;
    return true;
}

esp_err_t spi_bus_manager_add_device(spi_host_device_t host, spi_device_interface_config_t *devcfg, spi_device_handle_t *handle)
{
    return spi_bus_add_device(host, devcfg, handle);
}
