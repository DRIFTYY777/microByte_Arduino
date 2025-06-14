#include "radioHall.h"
#include <esp_log.h>

static const char *TAG = "RadioHall";

// Static member definitions
nrf24_config_t RadioHall::nrf24_config = {
    .pin_ce = NRF_CE,
    .pin_csn = NRF_CSN,
    .pin_irq = -1,  // Now works with int8_t
    .channel = 76,
    .status = 0,
    .payload = 8,
    .address_width = 5,
    .auto_ack = 1,
    .retransmit_delay = 15,
    .retransmit_count = 15,
    .spi_host = SPI3_HOST,
    .spi_speed = NRF_CLK_SPEED,
    .spi = NULL
};

bool RadioHall::initialized = false;

bool RadioHall::init() {
    if (initialized) {
        ESP_LOGW(TAG, "RadioHall already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing RadioHall with NRF24L01+");
    
    if (nrf24_init(&nrf24_config)) {
        initialized = true;
        ESP_LOGI(TAG, "RadioHall initialized successfully");
        
        // Basic configuration
        config(76, 8);  // Default channel and payload
        
        return true;
    } else {
        ESP_LOGE(TAG, "RadioHall initialization failed");
        return false;
    }
}

void RadioHall::config(uint8_t channel, uint8_t payload_size) {
    if (!initialized) {
        ESP_LOGE(TAG, "RadioHall not initialized");
        return;
    }

    ESP_LOGI(TAG, "Configuring RadioHall: Channel=%d, Payload=%d", channel, payload_size);
    nrf2_config(&nrf24_config, channel, payload_size);
}

bool RadioHall::connected() {
    if (!initialized) {
        return false;
    }
    
    return nrf24_isConnected(&nrf24_config);
}

void RadioHall::turn_on() {
    if (!initialized) {
        ESP_LOGE(TAG, "RadioHall not initialized");
        return;
    }
    
    ESP_LOGI(TAG, "Turning on RadioHall");
    // nrf24_onn(&nrf24_config);
}

void RadioHall::turn_off() {
    if (!initialized) {
        ESP_LOGE(TAG, "RadioHall not initialized");
        return;
    }
    
    ESP_LOGI(TAG, "Turning off RadioHall");
    // nrf24_off(&nrf24_config);
}

const nrf24_config_t* RadioHall::get_config() {
    return &nrf24_config;
}

bool RadioHall::is_initialized() {
    return initialized;
}

// Global instance
RadioHall radio_hall;