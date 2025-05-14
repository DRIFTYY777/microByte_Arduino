#if !defined(NRF24_H)
#define NRF24_H

#include "driver/spi_master.h"

typedef struct
{
    uint8_t pin_ce;
    uint8_t pin_csn;
    uint8_t pin_mosi;
    uint8_t pin_miso;
    uint8_t pin_sck;
    uint8_t pin_irq;
    uint8_t spi_host;
    spi_device_handle_t spi;
    uint8_t channel;
    uint8_t payload_size;
    uint8_t address_width;
    uint8_t auto_ack;
    uint8_t retransmit_delay;
    uint8_t retransmit_count;
} nrf24_config_t;

bool nrf24_init(nrf24_config_t *config);

#endif // NRF24_H
