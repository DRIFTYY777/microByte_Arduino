#include "radioHall.h"

#include <esp_log.h>
#include <components/drivers/nrf24/nrf24.h>
#include <components/system_config/system_config.h>

static const char* TAG = "RADIOHALL";

NRF24_t nrf24 = {
    .cePin = NRF_CE,
    .csnPin = NRF_CSN,
    .PTX = 1, // Start in TX mode
    .channel = 76, // Default channel (0-127, avoid WiFi overlap)
    .payload = 16, // Default payload size

    .spi_host = SPI3_HOST,
    .spi_speed = 1000000, // 1 MHz (explicit value)
    .spi = nullptr, // Will be initialized during setup
    .status = 0 // Initial status
};


void RADIOHALL::init() {
    if (Nrf24_init(&nrf24)) {
        ESP_LOGI(TAG, "NRF24 radio initialized successfully");
    }
    else {
        ESP_LOGE(TAG, "Failed to initialize NRF24 radio");
    }
}

void RADIOHALL::denit() {
    Nrf24_deinit(&nrf24);
}

void RADIOHALL::configureRadio(const uint8_t channel, const uint8_t payload) {
    Nrf24_config(&nrf24, channel, payload);
}

void RADIOHALL::isConnected() {
    if (Nrf24_isConnected(&nrf24)) {
        ESP_LOGI(TAG, "NRF24 radio is connected");
    } else {
        ESP_LOGE(TAG, "NRF24 radio is not connected");
    }
}




extern RADIOHALL radioHall;

