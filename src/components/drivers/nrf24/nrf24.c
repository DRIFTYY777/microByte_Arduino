#include "nrf24.h"
#include "driver/gpio.h"
#include <Arduino.h>
#include <components/drivers/spiManager/spiManager.h>

static const char *TAG = "NRF24_driver";

bool spi_bus_initialized[SOC_SPI_PERIPH_NUM];

bool nrf24_init(nrf24_config_t *config)
{

    config->channel = 76;     // Default channel
    config->status = 0;       // Default status
    config->payload_size = 8; // Default payload size

    // Set the GPIO pins for CE and CSN
    gpio_pad_select_gpio(config->pin_ce);
    gpio_pad_select_gpio(config->pin_csn);
    gpio_set_direction(config->pin_ce, GPIO_MODE_OUTPUT);
    gpio_set_direction(config->pin_csn, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Set RST pin: %i \n Set DC pin: %i", config->pin_ce, config->pin_csn);

    // Set-Up SPI BUS
    if (!spi_bus_initialized[config->spi_host])
    {
        spi_bus_manager_init(config->spi_host, config->pin_mosi, config->pin_miso, config->pin_sck, 0);
        spi_bus_initialized[config->spi_host] = true;
    }
    else
    {
        ESP_LOGI(TAG, "SPI bus already initialized.");
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = config->spi_speed,
        .mode = 0,
        .spics_io_num = config->pin_csn,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY};

    esp_err_t ret = spi_bus_manager_add_device(config->spi_host, &devcfg, &config->spi);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        config->spi = NULL; // Prevent use of invalid handle
        return false;
    }
    if (!config->spi)
    {
        ESP_LOGE(TAG, "SPI handle is NULL!");
        return;
    }

    ESP_LOGI(TAG, "SPI Bus configured correctly.");

    return true;
}