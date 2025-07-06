//
// Created by dhima on 16-06-2025.
//

#ifndef NRF24_H
#define NRF24_H


#include <hal/gpio_types.h>
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int cePin;// CE Pin controls RX / TX, default 8.
    int csnPin;//CSN Pin Chip Select Not, default 7.

    uint8_t PTX;  //In sending mode.

    uint8_t channel;//Channel 0 - 127 or 0 - 84 in the US.
    uint8_t payload;// Payload width in bytes default 16 max 32.

    // spi_host_device_t spi_host;
    spi_host_device_t spi_host;

    int spi_speed;

    spi_device_handle_t spi;

    uint8_t status;// Receive status
} NRF24_t;


/**
 * Power Amplifier level.
 *
 * For use with setPALevel()
 */
typedef enum {
    RF24_PA_MIN = 0,
    RF24_PA_LOW,
    RF24_PA_HIGH,
    RF24_PA_MAX,
    RF24_PA_ERROR
} rf24_pa_dbm_e;

/**
 * Data rate.  How fast data moves through the air.
 *
 * For use with setDataRate()
 */
typedef enum {
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS
} rf24_datarate_e;

/**
 * CRC Length.  How big (if any) of a CRC is included.
 *
 * For use with setCRCLength()
 */
typedef enum {
    RF24_CRC_DISABLED = 0,
    RF24_CRC_8,
    RF24_CRC_16
} rf24_crclength_e;


bool Nrf24_init(NRF24_t *dev);
void      Nrf24_deinit(NRF24_t *dev);
bool      spi_write_byte(NRF24_t * dev, uint8_t* Dataout, size_t DataLength );
bool      spi_read_byte(NRF24_t * dev, uint8_t* Datain, uint8_t* Dataout, size_t DataLength );
uint8_t   spi_transfer(NRF24_t * dev, uint8_t address);
void      spi_csnLow(NRF24_t * dev);
void      spi_csnHi(NRF24_t * dev);
void      Nrf24_config(NRF24_t * dev, uint8_t channel, uint8_t payload);
bool     Nrf24_isConnected(NRF24_t * dev);

void      Nrf24_send(NRF24_t * dev, uint8_t *value);
bool      Nrf24_send1(NRF24_t * dev, uint8_t *value);

void      Nrf24_enableNoAckFeature(NRF24_t * dev);
void      Nrf24_sendNoAck(NRF24_t * dev, uint8_t *value);
esp_err_t Nrf24_setRADDR(NRF24_t * dev, uint8_t * adr);
esp_err_t Nrf24_setTADDR(NRF24_t * dev, uint8_t * adr);
void      Nrf24_addRADDR(NRF24_t * dev, uint8_t pipe, uint8_t adr);
bool      Nrf24_dataReady(NRF24_t * dev);
uint8_t   Nrf24_getDataPipe(NRF24_t * dev);
bool      Nrf24_isSending(NRF24_t * dev);
bool      Nrf24_isSend(NRF24_t * dev, int timeout);
bool      Nrf24_rxFifoEmpty(NRF24_t * dev);
bool      Nrf24_txFifoEmpty(NRF24_t * dev);
void      Nrf24_getData(NRF24_t * dev, uint8_t * data);
uint8_t   Nrf24_getStatus(NRF24_t * dev);
void      Nrf24_configRegister(NRF24_t * dev, uint8_t reg, uint8_t value);
void      Nrf24_readRegister(NRF24_t * dev, uint8_t reg, uint8_t * value, uint8_t len);
void      Nrf24_writeRegister(NRF24_t * dev, uint8_t reg, uint8_t * value, uint8_t len);
void      Nrf24_powerUpRx(NRF24_t * dev);
void      Nrf24_powerUpTx(NRF24_t * dev);
void      Nrf24_powerDown(NRF24_t * dev);
void      Nrf24_SetOutputRF_PWR(NRF24_t * dev, uint8_t val);
void      Nrf24_SetSpeedDataRates(NRF24_t * dev, uint8_t val);
void      Nrf24_setRetransmitDelay(NRF24_t * dev, uint8_t val);
void      Nrf24_setRetransmitCount(NRF24_t * dev, uint8_t val);
void      Nrf24_ceHi(NRF24_t * dev);
void      Nrf24_ceLow(NRF24_t * dev);
void      Nrf24_flushRx(NRF24_t * dev);
void      Nrf24_printDetails(NRF24_t * dev);
void      Nrf24_print_status(uint8_t status);
void      Nrf24_print_address_register(NRF24_t * dev, const char* name, uint8_t reg, uint8_t qty);
void      Nrf24_print_byte_register(NRF24_t * dev, const char* name, uint8_t reg, uint8_t qty);
uint8_t   Nrf24_getDataRate(NRF24_t * dev);
char *    Nrf24_getDataRateString(NRF24_t * dev);
uint8_t   Nrf24_getCRCLength(NRF24_t * dev);
uint8_t   Nrf24_getPALevel(NRF24_t * dev);
char *    Nrf24_getPALevelString(NRF24_t * dev);
uint8_t   Nrf24_getRetransmitDelay(NRF24_t * dev);
uint8_t   Nrf24_getRetransmitCount(NRF24_t * dev);
uint8_t   Nrf24_getChannle(NRF24_t * dev);
uint8_t   Nrf24_getPayload(NRF24_t * dev);


bool Nrf24_setChannel(NRF24_t * dev, uint8_t channel);
bool Nrf24_transmitMode(NRF24_t * dev);

void Nrf24_clearStatus(NRF24_t * dev);
bool Nrf24_receiveMode(NRF24_t * dev);

void educational_rf_demonstration(NRF24_t* nrf);
void educational_channel_scanner(NRF24_t* nrf);


uint8_t Nrf24_scanNetworks(NRF24_t* nrf);

char Nrf24_getNetworkName(NRF24_t* nrf, uint8_t index);


#ifdef __cplusplus
}
#endif



#endif //NRF24_H
