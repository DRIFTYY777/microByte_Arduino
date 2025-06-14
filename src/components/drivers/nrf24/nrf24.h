#if !defined(NRF24_H)
#define NRF24_H

#include "driver/spi_master.h"

typedef struct
{
    uint8_t pin_ce; // Chip Enable
    uint8_t pin_csn;  // Chip Select Not
    int8_t pin_irq; // Interrupt pin, -1 if not used

    uint8_t channel; // RF channel (0-125)
    uint8_t status; // Receive status
    uint8_t payload; // Payload size (1-32 bytes)
    uint8_t address_width; // Address width (3-5 bytes)
    uint8_t auto_ack; // Enable auto acknowledgment (0 or 1)
    uint8_t retransmit_delay; // Retransmit delay in microseconds (0-15)
    uint8_t retransmit_count; // Retransmit count (0-15)

    int spi_host; // SPI host (HSPI_HOST or VSPI_HOST)
    int spi_speed; // SPI speed in Hz (e.g., 1000000 for 1MHz)
    spi_device_handle_t spi;

} nrf24_config_t; // NRF24L01 configuration structure

typedef enum
{
    RF24_PA_MIN = 0,
    RF24_PA_LOW,
    RF24_PA_HIGH,
    RF24_PA_MAX,
    RF24_PA_ERROR
} nrfPowerAmplifier_e; // Power Amplifier level

typedef enum
{
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS
} nrfDataRate_e; // Data rate

typedef enum
{
    RF24_CRC_DISABLED = 0,
    RF24_CRC_8,
    RF24_CRC_16
} nrfCrc_e; // CRC size

bool nrf24_init(nrf24_config_t *config);
void nrf2_config(nrf24_config_t *config, uint8_t channel, uint8_t payload_size);

void nrf24_write_byte(nrf24_config_t *config, uint8_t *data, uint8_t length);
void nrf24_read_byte(nrf24_config_t *config, uint8_t *dataIN, uint8_t *dataOUT, uint8_t length);
uint8_t nrf24_DataTransfer(nrf24_config_t *config, uint8_t address);

void nrf24_configRegister(nrf24_config_t *config, uint8_t reg, uint8_t value);
void nrf24_readRegister(nrf24_config_t *config, uint8_t reg, uint8_t *data, uint8_t length);
void nrf24_writeRegister(nrf24_config_t *config, uint8_t reg, uint8_t *data, uint8_t length);

// check device is connected
bool nrf24_isConnected(nrf24_config_t *config);

#endif // NRF24_H
