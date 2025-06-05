#include "nrf24.h"
#include "nrf24_regs.h"
#include "driver/gpio.h"
#include <components/drivers/spiManager/spiManager.h>
#include "esp_log.h"
#include <string.h>

#define _BV(x) (1 << (x))

static const char *TAG = "NRF24_driver";

bool spi_bus_initialized[SOC_SPI_PERIPH_NUM];

char rf24_data_rates[][8] = {"1Mbps", "2Mbps", "250Kbps"};
const char rf24_crc_length[][10] = {"Disabled", "8 bits", "16 bits"};
char rf24_pa_dbm[][8] = {"PA_MIN", "PA_LOW", "PA_HIGH", "PA_MAX"};

bool nrf24_init(nrf24_config_t *config)
{
    config->channel = 76; // Default channel
    config->status = 0;   // Default status
    config->payload = 8;  // Default payload size

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
        config->spi = NULL; // Prevent use of an invalid handle
        return false;
    }
    if (!config->spi)
    {
        ESP_LOGE(TAG, "SPI handle is NULL!");
        return false;
    }

    ESP_LOGI(TAG, "SPI Bus configured correctly.");

    return true;
}

void nrf2_config(nrf24_config_t *config, uint8_t channel, uint8_t payload_size)
{
    config->channel = channel;
    config->payload = payload_size;
    nrf24_configRegister(config, RF_CH, config->channel);    // Set RF channel
    nrf24_configRegister(config, RX_PW_P0, config->payload); // Set length of incoming payload
    nrf24_configRegister(config, RX_PW_P1, config->payload);
}

void nrf24_write_byte(nrf24_config_t *config, uint8_t *data, uint8_t length)
{
    spi_transaction_t spiTransaction;
    if (length > 0)
    {
        memset(&spiTransaction, 0, sizeof(spi_transaction_t));
        spiTransaction.length = length * 8; // due to 8Bit mode
        spiTransaction.tx_buffer = data;
        spiTransaction.rx_buffer = NULL;
        spi_device_transmit(config->spi, &spiTransaction);
    }
}

void nrf24_read_byte(nrf24_config_t *config, uint8_t *dataIN, uint8_t *dataOUT, uint8_t length)
{
    spi_transaction_t spiTransaction;
    if (length > 0)
    {
        memset(&spiTransaction, 0, sizeof(spi_transaction_t));
        spiTransaction.length = length * 8; //  8Bit mode
        spiTransaction.tx_buffer = dataOUT; // ass pointer, not address
        spiTransaction.rx_buffer = dataIN;  // ass pointer, not address
        spi_device_transmit(config->spi, &spiTransaction);
    }
}

uint8_t nrf24_DataTransfer(nrf24_config_t *config, uint8_t address)
{
    // get data and return it;
    uint8_t dataIN[1];
    uint8_t dataOUT[1];
    dataOUT[0] = address;
    nrf24_read_byte(config, dataIN, dataOUT, 1);
    return dataIN[0];
}

void nrf24_configRegister(nrf24_config_t *config, uint8_t reg, uint8_t value)
{
    gpio_set_level(config->pin_csn, 0);
    nrf24_DataTransfer(config, W_REGISTER | (REGISTER_MASK & reg));
    nrf24_DataTransfer(config, value);
    gpio_set_level(config->pin_csn, 1);
}

void nrf24_readRegister(nrf24_config_t *config, uint8_t reg, uint8_t *data, uint8_t length)
{
    gpio_set_level(config->pin_csn, 0);
    nrf24_DataTransfer(config, R_REGISTER | (REGISTER_MASK & reg));
    uint8_t dummy[length];
    memset(dummy, 0xFF, length);                  // or 0x00
    nrf24_read_byte(config, data, dummy, length); // FIX: use dummy buffer
    gpio_set_level(config->pin_csn, 1);
}

void nrf24_writeRegister(nrf24_config_t *config, uint8_t reg, uint8_t *data, uint8_t length)
{
    gpio_set_level(config->pin_csn, 0);
    nrf24_DataTransfer(config, W_REGISTER | (REGISTER_MASK & reg));
    nrf24_write_byte(config, data, length);
    gpio_set_level(config->pin_csn, 1);
}

bool nrf24_isConnected(nrf24_config_t *config)
{
    uint8_t status = nrf24_DataTransfer(config, 0xFF);
    if (status == 0xFF)
    {
        return false;
    }
    return true;
}
