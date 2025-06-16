//
// Created by dhima on 16-06-2025.
//

#include "nrf24.h"
#include "nrf24_regs.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"


#define TAG "NRF24"
#include <components/drivers/spiManager/spiManager.h>

// SPI Stuff
// #if CONFIG_SPI2_HOST
// #define HOST_ID SPI2_HOST
// #elif CONFIG_SPI3_HOST
// #define HOST_ID SPI3_HOST
// #endif



char rf24_datarates[][8] = {"1Mbps", "2Mbps", "250Kbps"};
const char rf24_crclength[][10] = {"Disabled", "8 bits", "16 bits"};
char rf24_pa_dbm[][8] = {"PA_MIN", "PA_LOW", "PA_HIGH", "PA_MAX"};

bool Nrf24_init(NRF24_t *dev)
{
	ESP_LOGI(TAG, "CONFIG_CE_GPIO=%d", dev->cePin);
	ESP_LOGI(TAG, "CONFIG_CSN_GPIO=%d", dev->csnPin);

	//gpio_pad_select_gpio(CONFIG_CE_GPIO);
	gpio_reset_pin(static_cast<gpio_num_t>(dev->cePin));
	gpio_set_direction(static_cast<gpio_num_t>(dev->cePin), GPIO_MODE_OUTPUT);
	gpio_set_level(static_cast<gpio_num_t>(dev->cePin), 0);

	//gpio_pad_select_gpio(CONFIG_CSN_GPIO);
	gpio_reset_pin(static_cast<gpio_num_t>(dev->csnPin));
	gpio_set_direction(static_cast<gpio_num_t>(dev->csnPin), GPIO_MODE_OUTPUT);
	gpio_set_level(static_cast<gpio_num_t>(dev->csnPin), 1);




	// Check if SPI bus is already initialized using the manager
	if (!spi_bus_manager_is_initialized(dev->spi_host))
	{
		ESP_LOGI(TAG, "SPI bus not initialized. Please initialize it first using spi_bus_manager_init()");
		return false;
	}
	else
	{
		ESP_LOGI(TAG, "SPI bus already initialized.");
	}

	// // Configure an SPI device using the manager
	// spi_device_interface_config_t devcfg = {
	// 	.clock_speed_hz = dev->spi_speed,
	// 	.mode = 0,
	// 	.spics_io_num = dev->csnPin,
	// 	.queue_size = 7,
	// 	.flags = SPI_DEVICE_NO_DUMMY};


	spi_device_interface_config_t devcfg = {};
	devcfg.clock_speed_hz = dev->spi_speed;
	devcfg.mode = 0;
	devcfg.spics_io_num = dev->csnPin;
	devcfg.queue_size = 7;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;



	const esp_err_t ret = spi_bus_manager_add_device(dev->spi_host,  &devcfg, &dev->spi);

	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
		dev->spi = NULL; // Prevent use of invalid handle
		return false;
	}

	if (!dev->spi)
	{
		ESP_LOGE(TAG, "SPI handle is NULL!");
		return false;
	}

	ESP_LOGI(TAG, "SPI device configured correctly using SPI manager.");


	dev->cePin = dev->cePin;
	dev->csnPin = dev->csnPin;
	dev->channel = 1;
	dev->payload = 16;



	return true;
}

void Nrf24_deinit(NRF24_t *dev) {
	memset(dev, 0, sizeof(NRF24_t));
	// spi_bus_free(dev->spi_host);
}

bool spi_write_byte(NRF24_t * dev, uint8_t* Dataout, size_t DataLength )
{
	spi_transaction_t SPITransaction;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Dataout;
		SPITransaction.rx_buffer = NULL;
		spi_device_transmit( dev->spi, &SPITransaction );
	}

	return true;
}

bool spi_read_byte(NRF24_t * dev, uint8_t* Datain, uint8_t* Dataout, size_t DataLength )
{
	spi_transaction_t SPITransaction;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Dataout;
		SPITransaction.rx_buffer = Datain;
		spi_device_transmit( dev->spi, &SPITransaction );
	}

	return true;
}

uint8_t spi_transfer(NRF24_t * dev, uint8_t address) {
	uint8_t datain[1];
	uint8_t dataout[1];
	dataout[0] = address;
	//spi_write_byte(dev, dataout, 1 );
	spi_read_byte(dev, datain, dataout, 1 );
	return datain[0];
}

void spi_csnHi(NRF24_t * dev) {
	gpio_set_level( static_cast<gpio_num_t>(dev->csnPin), 1 );
}

void spi_csnLow(NRF24_t * dev) {
	gpio_set_level( static_cast<gpio_num_t>(dev->csnPin), 0 );
}

// Sets the important registers in the MiRF module and powers the module
// in receiving mode
// NB: channel and payload must be set now.
void Nrf24_config(NRF24_t * dev, uint8_t channel, uint8_t payload)
{
	dev->channel = channel;
	dev->payload = payload;
	Nrf24_configRegister(dev, RF_CH, dev->channel); // Set RF channel
	Nrf24_configRegister(dev, RX_PW_P0, dev->payload); // Set length of incoming payload
	Nrf24_configRegister(dev, RX_PW_P1, dev->payload);
	Nrf24_powerUpRx(dev); // Start receiver
	Nrf24_flushRx(dev);
}

// Sets the receiving device address
//void Nrf24_setRADDR(NRF24_t * dev, uint8_t * adr)
esp_err_t Nrf24_setRADDR(NRF24_t * dev, uint8_t * adr)
{
	esp_err_t ret = ESP_OK;
	Nrf24_writeRegister(dev, RX_ADDR_P1, adr, mirf_ADDR_LEN);
	uint8_t buffer[5];
	Nrf24_readRegister(dev, RX_ADDR_P1, buffer, sizeof(buffer));
	for (int i=0;i<5;i++) {
		ESP_LOGD(TAG, "adr[%d]=0x%x buffer[%d]=0x%x", i, adr[i], i, buffer[i]);
		if (adr[i] != buffer[i]) ret = ESP_FAIL;
	}
	return ret;
}

// Sets the transmitting device  address
//void Nrf24_setTADDR(NRF24_t * dev, uint8_t * adr)
esp_err_t Nrf24_setTADDR(NRF24_t * dev, uint8_t * adr)
{
	esp_err_t ret = ESP_OK;
	Nrf24_writeRegister(dev, RX_ADDR_P0, adr, mirf_ADDR_LEN); //RX_ADDR_P0 must be set to the sending addr for auto ack to work.
	Nrf24_writeRegister(dev, TX_ADDR, adr, mirf_ADDR_LEN);
	uint8_t buffer[5];
	Nrf24_readRegister(dev, RX_ADDR_P0, buffer, sizeof(buffer));
	for (int i=0;i<5;i++) {
		ESP_LOGD(TAG, "adr[%d]=0x%x buffer[%d]=0x%x", i, adr[i], i, buffer[i]);
		if (adr[i] != buffer[i]) ret = ESP_FAIL;
	}
	return ret;
}

// Add the receiving device address
void Nrf24_addRADDR(NRF24_t * dev, uint8_t pipe, uint8_t adr)
{
	uint8_t value;
	Nrf24_readRegister(dev, EN_RXADDR, &value, 1);

	if (pipe == 2) {
		Nrf24_configRegister(dev, RX_PW_P2, dev->payload);
		Nrf24_configRegister(dev, RX_ADDR_P2, adr);
		value = value | 0x04;
		Nrf24_configRegister(dev, EN_RXADDR, value);
	} else if (pipe == 3) {
		Nrf24_configRegister(dev, RX_PW_P3, dev->payload);
		Nrf24_configRegister(dev, RX_ADDR_P3, adr);
		value = value | 0x08;
		Nrf24_configRegister(dev, EN_RXADDR, value);
	} else if (pipe == 4) {
		Nrf24_configRegister(dev, RX_PW_P4, dev->payload);
		Nrf24_configRegister(dev, RX_ADDR_P4, adr);
		value = value | 0x10;
		Nrf24_configRegister(dev, EN_RXADDR, value);
	} else if (pipe == 5) {
		Nrf24_configRegister(dev, RX_PW_P5, dev->payload);
		Nrf24_configRegister(dev, RX_ADDR_P5, adr);
		value = value | 0x20;
		Nrf24_configRegister(dev, EN_RXADDR, value);
	}
}

// Checks if data is available for reading
extern bool Nrf24_dataReady(NRF24_t * dev)
{
	// See note in getData() function - just checking RX_DR isn't good enough
	uint8_t status = Nrf24_getStatus(dev);
	//printf("Nrf24_dataReady status=0x%x\n", status);
	if ( status & (1 << RX_DR) ) {
		// Save status
		dev->status = status;
		return 1;
	}
	// We can short circuit on RX_DR, but if it's not set, we still need
	// to check the FIFO for any pending packets
	//return !Nrf24_rxFifoEmpty(dev);
	return 0;
}

// Get pipe number for reading
uint8_t Nrf24_getDataPipe(NRF24_t * dev) {
	//uint8_t status = Nrf24_getStatus(dev);
	//printf("dev->status=0x%x\n",dev->status);
	return ((dev->status & 0x0E) >> 1);
}

extern bool Nrf24_rxFifoEmpty(NRF24_t * dev)
{
	uint8_t fifoStatus;
	Nrf24_readRegister(dev, FIFO_STATUS, &fifoStatus, sizeof(fifoStatus));
	return (fifoStatus & (1 << RX_EMPTY));
}

// Reads payload bytes into data array
extern void Nrf24_getData(NRF24_t * dev, uint8_t * data)
{
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, R_RX_PAYLOAD ); // Send cmd to read rx payload
	spi_read_byte(dev, data, data, dev->payload); // Read payload
	spi_csnHi(dev); // Pull up chip select
	// NVI: per product spec, p 67, note c:
	// "The RX_DR IRQ is asserted by a new packet arrival event. The procedure
	// for handling this interrupt should be: 1) read payload through SPI,
	// 2) clear RX_DR IRQ, 3) read FIFO_STATUS to check if there are more
	// payloads available in RX FIFO, 4) if there are more data in RX FIFO,
	// repeat from step 1)."
	// So if we're going to clear RX_DR here, we need to check the RX FIFO
	// in the dataReady() function
	Nrf24_configRegister(dev, STATUS, (1 << RX_DR)); // Reset status register
}

// Clocks only one byte into the given MiRF register
void Nrf24_configRegister(NRF24_t * dev, uint8_t reg, uint8_t value)
{
	spi_csnLow(dev);
	spi_transfer(dev, W_REGISTER | (REGISTER_MASK & reg));
	spi_transfer(dev, value);
	spi_csnHi(dev);
}

// Reads an array of bytes from the given start position in the MiRF registers
void Nrf24_readRegister(NRF24_t * dev, uint8_t reg, uint8_t * value, uint8_t len)
{
	spi_csnLow(dev);
	spi_transfer(dev, R_REGISTER | (REGISTER_MASK & reg));
	spi_read_byte(dev, value, value, len);
	spi_csnHi(dev);
}

// Writes an array of bytes into inte the MiRF registers
void Nrf24_writeRegister(NRF24_t * dev, uint8_t reg, uint8_t * value, uint8_t len)
{
	spi_csnLow(dev);
	spi_transfer(dev, W_REGISTER | (REGISTER_MASK & reg));
	spi_write_byte(dev, value, len);
	spi_csnHi(dev);
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void Nrf24_send(NRF24_t * dev, uint8_t * value)
{
	uint8_t status = Nrf24_getStatus(dev);
	while (dev->PTX) // Wait until last paket is send
	{
		status = Nrf24_getStatus(dev);
		if ((status & ((1 << TX_DS)  | (1 << MAX_RT))))
		{
			dev->PTX = 0;
			break;
		}
	}
	Nrf24_ceLow(dev);
	Nrf24_powerUpTx(dev); // Set to transmitter mode , Power up
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, FLUSH_TX ); // Write cmd to flush tx fifo
	spi_csnHi(dev); // Pull up chip select
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, W_TX_PAYLOAD ); // Write cmd to write payload
	spi_write_byte(dev, value, dev->payload); // Write payload
	spi_csnHi(dev); // Pull up chip select
	Nrf24_ceHi(dev); // Start transmission
}

bool Nrf24_send1(NRF24_t * dev, uint8_t *value) {
	// This function is deprecated, use Nrf24_send() instead.
	ESP_LOGW(TAG, "Nrf24_send() is deprecated, use Nrf24_send() instead.");
	uint8_t status = Nrf24_getStatus(dev);
	while (dev->PTX) // Wait until last paket is sent
	{
		status = Nrf24_getStatus(dev);
		if ((status & ((1 << TX_DS)  | (1 << MAX_RT))))
		{
			dev->PTX = 0;
			break;
		}
	}
	Nrf24_ceLow(dev);
	Nrf24_powerUpTx(dev); // Set to transmitter mode , Power up
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, FLUSH_TX ); // Write cmd to flush tx fifo
	spi_csnHi(dev); // Pull up chip select
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, W_TX_PAYLOAD ); // Write cmd to write payload
	spi_write_byte(dev, value, dev->payload); // Write payload
	spi_csnHi(dev); // Pull up chip select
	Nrf24_ceHi(dev); // Start transmission
	// Wait for sending to finish
	if (Nrf24_isSend(dev, 1000)) { // Wait for sending to finish with a timeout of 1000 ms
		return true; // Sending was successful
	} else {
		ESP_LOGE(TAG, "Sending failed or timed out.");
		return false; // Sending failed or timed out
	}


}



// Sends payload without expecting an ACK from the receiver effectively turning off retransmission of failed payloads.
// See Nrf24l01 "PTX Operation" flowchart in the datasheet.
// NOTE: Make sure to call Nrf24_enableNoAckFeature() before calling this function.
// Is useful when achieving maximum throughput without caring much about losses.
void Nrf24_sendNoAck(NRF24_t * dev, uint8_t * value)
{
	uint8_t status = Nrf24_getStatus(dev);
	while (dev->PTX) // Wait until last paket is sent
	{
		status = Nrf24_getStatus(dev);
		if ((status & ((1 << TX_DS)  | (1 << MAX_RT))))
		{
			dev->PTX = 0;
			break;
		}
	}
	Nrf24_ceLow(dev);
	Nrf24_powerUpTx(dev); // Set to transmitter mode , Power up
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, FLUSH_TX ); // Write cmd to flush tx fifo
	spi_csnHi(dev); // Pull up chip select
	spi_csnLow(dev); // Pull down chip select
	spi_transfer(dev, W_TX_PAYLOAD_NO_ACK ); // Write cmd to write payload
	spi_write_byte(dev, value, dev->payload); // Write payload
	spi_csnHi(dev); // Pull up chip select
	Nrf24_ceHi(dev); // Start transmission
}

// Test if chip is still sending.
// When sending has finished return chip to listening.
bool Nrf24_isSending(NRF24_t * dev) {
	if (dev->PTX)
	{
		uint8_t status = Nrf24_getStatus(dev);
		if ((status & ((1 << TX_DS)  | (1 << MAX_RT)))) {// if sending successful (TX_DS) or max retries exceded (MAX_RT).
			Nrf24_powerUpRx(dev);
			return false;
		}
		return true;
	}
	return false;
}

// Test if Sending has finished or retry is over.
// When sending has finished return trur.
// When reach maximum number of TX retries return false.
bool Nrf24_isSend(NRF24_t * dev, int timeout) {
	uint8_t status;
	TickType_t startTick = xTaskGetTickCount();
	if (dev->PTX) {
		while(1) {
			status = Nrf24_getStatus(dev);
			/*
				if sending successful (TX_DS) or max retries exceded (MAX_RT).
			*/

			if (status & (1 << TX_DS)) { // Data Sent TX FIFO interrup
				Nrf24_powerUpRx(dev);
				return true;
			}

			if (status & (1 << MAX_RT)) { // Maximum number of TX retries interrupt
				ESP_LOGW(TAG, "Maximum number of TX retries interrupt");
				Nrf24_powerUpRx(dev);
				return false;
			}

			// I believe either TX_DS or MAX_RT will always be notified.
			// Therefore, it is unusual for neither to be notified for a period of time.
			// I don't know exactly how to respond.
			TickType_t diffTick = xTaskGetTickCount() - startTick;
			if ( (diffTick * portTICK_PERIOD_MS) > timeout) {
				ESP_LOGE(TAG, "Status register timeout. status=0x%x", status);
				return false;
			}
			vTaskDelay(1);
		}
	}
	return false;
}

// Enables the W_TX_PAYLOAD command
// NOTE: Make sure to call this before using Nrf24_sendNoAck().
// Can be called anytime after the call to Nrf24_init() and preferably only once.
void Nrf24_enableNoAckFeature(NRF24_t * dev)
{
	uint8_t value;

	Nrf24_readRegister(dev, FEATURE, &value, 1);
	value = value | 1;
	Nrf24_configRegister(dev, FEATURE, value);
}



uint8_t Nrf24_getStatus(NRF24_t * dev) {
	uint8_t rv;
	Nrf24_readRegister(dev, STATUS, &rv, 1);
	return rv;
}

void Nrf24_powerUpRx(NRF24_t * dev) {
	dev->PTX = 0;
	Nrf24_ceLow(dev);
	Nrf24_configRegister(dev, CONFIG, mirf_CONFIG | ( (1 << PWR_UP) | (1 << PRIM_RX) ) ); //set device as RX mode
	Nrf24_ceHi(dev);
	Nrf24_configRegister(dev, STATUS, (1 << TX_DS) | (1 << MAX_RT)); //Clear seeded interrupt and max tx number interrupt
}

void Nrf24_flushRx(NRF24_t * dev)
{
	spi_csnLow(dev);
	spi_transfer(dev, FLUSH_RX );
	spi_csnHi(dev);
}

void Nrf24_powerUpTx(NRF24_t * dev) {
	dev->PTX = 1;
	Nrf24_configRegister(dev, CONFIG, mirf_CONFIG | ( (1 << PWR_UP) | (0 << PRIM_RX) ) ); //set device as TX mode
	Nrf24_configRegister(dev, STATUS, (1 << TX_DS) | (1 << MAX_RT)); //Clear seeded interrupt and max tx number interrupt
}

void Nrf24_ceHi(NRF24_t * dev) {
	gpio_set_level( static_cast<gpio_num_t>(dev->cePin), 1 );
}

void Nrf24_ceLow(NRF24_t * dev) {
	gpio_set_level( static_cast<gpio_num_t>(dev->cePin), 0 );
}

void Nrf24_powerDown(NRF24_t * dev)
{
	Nrf24_ceLow(dev);
	Nrf24_configRegister(dev, CONFIG, mirf_CONFIG );
}

//Set tx power : 0=-18dBm,1=-12dBm,2=-6dBm,3=0dBm
void Nrf24_SetOutputRF_PWR(NRF24_t * dev, uint8_t val)
{
	if (val > 3) return;

	uint8_t value;
	Nrf24_readRegister(dev, RF_SETUP, &value, 1);
	value = value & 0xF9;
	value = value | (val<< RF_PWR);
	//Nrf24_configRegister(dev, RF_SETUP,	(val<< RF_PWR) );
	Nrf24_configRegister(dev, RF_SETUP,	value);
}

//Select between the high speed data rates:0=1Mbps, 1=2Mbps, 2=250Kbps
void Nrf24_SetSpeedDataRates(NRF24_t * dev, uint8_t val)
{
	if (val > 2) return;

	uint8_t value;
	Nrf24_readRegister(dev, RF_SETUP, &value, 1);
	if(val == 2)
	{
		value = value | 0x20;
		value = value & 0xF7;
		//Nrf24_configRegister(dev, RF_SETUP,	(1 << RF_DR_LOW) );
		Nrf24_configRegister(dev, RF_SETUP,	value);
	}
	else
	{
		value = value & 0xD7;
		value = value | (val << RF_DR_HIGH);
		//Nrf24_configRegister(dev, RF_SETUP,	(val << RF_DR_HIGH) );
		Nrf24_configRegister(dev, RF_SETUP,	value);
	}
}

//Set Auto Retransmit Delay 0=250us, 1=500us, ... 15=4000us
void Nrf24_setRetransmitDelay(NRF24_t * dev, uint8_t val)
{
	uint8_t value;
	Nrf24_readRegister(dev, SETUP_RETR, &value, 1);
	value = value & 0x0F;
	value = value | (val << ARD);
	Nrf24_configRegister(dev, SETUP_RETR, value);
}

void Nrf24_setRetransmitCount(NRF24_t * dev, uint8_t val)
{
	uint8_t value;
	Nrf24_readRegister(dev, SETUP_RETR, &value, 1);
	value = value & 0xF0;
	value = value | val;
	Nrf24_configRegister(dev, SETUP_RETR, value);
}



void Nrf24_printDetails(NRF24_t * dev)
{

	printf("================ SPI Configuration ================\n" );
	printf("CSN Pin  \t = GPIO%d\n",dev->csnPin);
	printf("CE Pin	\t = GPIO%d\n", dev->cePin);
	// printf("Clock Speed\t = %d\n", SPI_Frequency);
	printf("================ NRF Configuration ================\n");

	Nrf24_print_status(Nrf24_getStatus(dev));

	Nrf24_print_address_register(dev, "RX_ADDR_P0-1", RX_ADDR_P0, 2);
	Nrf24_print_byte_register(dev, "RX_ADDR_P2-5", RX_ADDR_P2, 4);
	Nrf24_print_address_register(dev, "TX_ADDR\t", TX_ADDR, 1);

	Nrf24_print_byte_register(dev, "RX_PW_P0-6", RX_PW_P0, 6);
	Nrf24_print_byte_register(dev, "EN_AA\t", EN_AA, 1);
	Nrf24_print_byte_register(dev, "EN_RXADDR", EN_RXADDR, 1);
	Nrf24_print_byte_register(dev, "RF_CH\t", RF_CH, 1);
	Nrf24_print_byte_register(dev, "RF_SETUP", RF_SETUP, 1);
	Nrf24_print_byte_register(dev, "CONFIG\t", CONFIG, 1);
	Nrf24_print_byte_register(dev, "DYNPD/FEATURE", DYNPD, 2);
	//printf("getDataRate()=%d\n",Nrf24_getDataRate(dev));
	printf("Data Rate\t = %s\n",rf24_datarates[Nrf24_getDataRate(dev)]);
#if 0
	printf_P(PSTR("Model\t\t = "
	PRIPSTR
	"\r\n"),pgm_read_ptr(&rf24_model_e_str_P[isPVariant()]));
#endif
	//printf("getCRCLength()=%d\n",Nrf24_getCRCLength(dev));
	printf("CRC Length\t = %s\n", rf24_crclength[Nrf24_getCRCLength(dev)]);
	//printf("getPALevel()=%d\n",Nrf24_getPALevel(dev));
	printf("PA Power\t = %s\n", rf24_pa_dbm[Nrf24_getPALevel(dev)]);
	const uint8_t retransmit = Nrf24_getRetransmitDelay(dev);
	const int16_t delay = (retransmit+1)*250;
	printf("Retransmit\t = %d us\n", delay);
}

#define _BV(x) (1<<(x))

void Nrf24_print_status(uint8_t status)
{
	printf("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n", status, (status & _BV(RX_DR)) ? 1 : 0,
			(status & _BV(TX_DS)) ? 1 : 0, (status & _BV(MAX_RT)) ? 1 : 0, ((status >> RX_P_NO) & 0x07), (status & _BV(TX_FULL)) ? 1 : 0);
}

void Nrf24_print_address_register(NRF24_t * dev, const char* name, uint8_t reg, uint8_t qty)
{
	printf("%s\t =",name);
	while (qty--) {
		//uint8_t buffer[addr_width];
		uint8_t buffer[5];
		Nrf24_readRegister(dev, reg++, buffer, sizeof(buffer));

		printf(" 0x");
#if 0
		uint8_t* bufptr = buffer + sizeof buffer;
		while (--bufptr >= buffer) {
			printf("%02x", *bufptr);
		}
#endif
		for(int i=0;i<5;i++) {
			printf("%02x", buffer[i]);
		}
	}
	printf("\r\n");
}

void Nrf24_print_byte_register(NRF24_t * dev, const char* name, uint8_t reg, uint8_t qty)
{
	printf("%s\t =", name);
	while (qty--) {
		uint8_t buffer[1];
		Nrf24_readRegister(dev, reg++, buffer, 1);
		printf(" 0x%02x", buffer[0]);
	}
	printf("\r\n");
}

uint8_t Nrf24_getDataRate(NRF24_t * dev)
{
	rf24_datarate_e result;
	uint8_t dr;
	Nrf24_readRegister(dev, RF_SETUP, &dr, sizeof(dr));
	//printf("RF_SETUP=%x\n",dr);
	dr = dr & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

	// switch uses RAM (evil!)
	// Order matters in our case below
	if (dr == _BV(RF_DR_LOW)) {
		// '10' = 250KBPS
		result = RF24_250KBPS;
	} else if (dr == _BV(RF_DR_HIGH)) {
		// '01' = 2MBPS
		result = RF24_2MBPS;
	} else {
		// '00' = 1MBPS
		result = RF24_1MBPS;
	}
	return result;
}

char * Nrf24_getDataRateString(NRF24_t * dev)
{
	return rf24_datarates[Nrf24_getDataRate(dev)];
}

uint8_t Nrf24_getCRCLength(NRF24_t * dev)
{
	rf24_crclength_e result = RF24_CRC_DISABLED;

	uint8_t config;
	Nrf24_readRegister(dev, CONFIG, &config, sizeof(config));
	//printf("CONFIG=%x\n",config);
	config = config & (_BV(CRCO) | _BV(EN_CRC));
	uint8_t AA;
	Nrf24_readRegister(dev, EN_AA, &AA, sizeof(AA));

	if (config & _BV(EN_CRC) || AA) {
		if (config & _BV(CRCO)) {
			result = RF24_CRC_16;
		} else {
			result = RF24_CRC_8;
		}
	}

	return result;
}

uint8_t Nrf24_getPALevel(NRF24_t * dev)
{
	uint8_t level;
	Nrf24_readRegister(dev, RF_SETUP, &level, sizeof(level));
	//printf("RF_SETUP=%x\n",level);
	level = (level & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1;
	return (level);
}

char * Nrf24_getPALevelString(NRF24_t * dev)
{
	return rf24_pa_dbm[Nrf24_getPALevel(dev)];
}

uint8_t Nrf24_getRetransmitDelay(NRF24_t * dev)
{
	uint8_t value;
	Nrf24_readRegister(dev, SETUP_RETR, &value, 1);
	return (value >> 4);
}

uint8_t Nrf24_getRetransmitCount(NRF24_t * dev)
{
	uint8_t value;
	Nrf24_readRegister(dev, SETUP_RETR, &value, 1);
	return (value & 0x0F);
}


uint8_t Nrf24_getChannle(NRF24_t * dev)
{
	return dev->channel;
}

uint8_t Nrf24_getPayload(NRF24_t * dev)
{
	return dev->payload;
}

bool Nrf24_isConnected(NRF24_t *dev) {
	// check if the device is connected by reading the status register
	uint8_t status = Nrf24_getStatus(dev);
	return (status != 0xFF); // If status is not 0xFF, the device is connected
}

bool Nrf24_setChannel(NRF24_t * dev, uint8_t channel) {
	// set the channel for the NRF24 device
	if (channel > 125) {
		ESP_LOGE(TAG, "Channel out of range: %d. Valid range is 0-125.", channel);
		return false;
	}
	dev->channel = channel;
	Nrf24_configRegister(dev, RF_CH, dev->channel); // Set RF channel
	ESP_LOGI(TAG, "NRF24 channel set to %d", dev->channel);
	return true;
}
bool Nrf24_transmitMode(NRF24_t * dev) {
	// set the NRF24 device to transmit mode
	Nrf24_ceLow(dev);
	Nrf24_configRegister(dev, CONFIG, mirf_CONFIG | (1 << PWR_UP) | (0 << PRIM_RX)); // Set device as TX mode
	Nrf24_ceHi(dev);
	ESP_LOGI(TAG, "NRF24 set to transmit mode");
	return true;
}

void Nrf24_clearStatus(NRF24_t * dev) {
	// clear the status register of the NRF24 device
	Nrf24_configRegister(dev, STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT)); // Clear RX_DR, TX_DS, and MAX_RT bits
	ESP_LOGI(TAG, "NRF24 status cleared");
}
bool Nrf24_receiveMode(NRF24_t * dev) {
	// set the NRF24 device to receive mode
	Nrf24_ceLow(dev);
	Nrf24_configRegister(dev, CONFIG, mirf_CONFIG | (1 << PWR_UP) | (1 << PRIM_RX)); // Set device as RX mode
	Nrf24_ceHi(dev);
	ESP_LOGI(TAG, "NRF24 set to receive mode");
	// Clear any pending interrupts
	Nrf24_clearStatus(dev);
	// Flush RX FIFO to clear any old data
	Nrf24_flushRx(dev);
	ESP_LOGI(TAG, "NRF24 ready to receive data");
	return true;
}



void educational_rf_demonstration(NRF24_t* nrf) {
    ESP_LOGW(TAG, "Starting RF Education Demo - FOR LEARNING ONLY!");
    ESP_LOGW(TAG, "Ensure you have proper authorization and are in compliance with local laws");

    // Bluetooth operates in 2.4GHz ISM band (2400-2485 MHz)
    // Channels 0-83 on NRF24 correspond to 2400-2483 MHz
    const uint8_t bluetooth_channels[] = {
        2, 26, 80  // Some Bluetooth frequency channels (2402, 2426, 2480 MHz)
    };

    // Educational noise pattern (not actual jamming)
    uint8_t noise_pattern[32];
    for (int i = 0; i < 32; i++) {
        noise_pattern[i] = 0xAA; // Alternating pattern for demonstration
    }

    ESP_LOGI(TAG, "Demonstrating RF concepts on educational channels only");

    for (int channel_idx = 0; channel_idx < 3; channel_idx++) {
        uint8_t channel = bluetooth_channels[channel_idx];

        ESP_LOGI(TAG, "Educational demo on channel %d (freq: %d MHz)",
                 channel, 2400 + channel);

        // Set channel for demonstration
        if (!Nrf24_setChannel(nrf, channel)) {
            ESP_LOGE(TAG, "Failed to set channel %d", channel);
            continue;
        }

        // Set to TX mode for demonstration
        if (!Nrf24_transmitMode(nrf)) {
            ESP_LOGE(TAG, "Failed to set transmit mode");
            continue;
        }

        // Brief educational transmission (very short duration)
        for (int i = 0; i < 10; i++) {
            // Transmit educational pattern
            if (Nrf24_send1(nrf, noise_pattern)) {
                ESP_LOGD(TAG, "Educational pattern sent on channel %d", channel);
            }

            // Very short delay - minimize any potential interference
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // Longer pause between channels for educational purposes
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Return to receive mode
    Nrf24_receiveMode(nrf);
    ESP_LOGI(TAG, "Educational RF demonstration completed");
    ESP_LOGW(TAG, "Remember: Use this knowledge responsibly and legally!");
}

/**
 * Educational function to demonstrate channel scanning
 * Shows how to detect activity across the 2.4GHz band
 */
void educational_channel_scanner(NRF24_t* nrf) {
    ESP_LOGI(TAG, "Starting Educational Channel Scanner");

    // Set to receive mode
    if (!Nrf24_receiveMode(nrf)) {
        ESP_LOGE(TAG, "Failed to set receive mode");
        return;
    }

    // Scan across channels to demonstrate spectrum usage
    for (uint8_t channel = 0; channel < 126; channel++) {
        if (!Nrf24_setChannel(nrf, channel)) {
            continue;
        }

        // Brief listen period
        vTaskDelay(pdMS_TO_TICKS(5));

        // Check if there's activity (educational purposes)
        uint8_t status = Nrf24_getStatus(nrf);
        if (status & 0x40) { // RX_DR bit
            ESP_LOGI(TAG, "Activity detected on channel %d (freq: %d MHz)",
                     channel, 2400 + channel);
        }

        // Clear status
        Nrf24_clearStatus(nrf);
    }

    ESP_LOGI(TAG, "Channel scan completed");
}

























