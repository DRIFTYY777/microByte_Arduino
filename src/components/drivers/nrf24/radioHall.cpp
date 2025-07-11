#include "radioHall.h"

#include <esp_log.h>
#include <HardwareSerial.h>
#include <components/drivers/nrf24/nrf24.h>
#include <components/system_config/system_config.h>

static const char *TAG = "RADIOHALL";

NRF24_t nrf24 = {
    .cePin = NRF_CE,
    .csnPin = NRF_CSN,
    .PTX = 1,      // Start in TX mode
    .channel = 76, // Default channel (0-127, avoid WiFi overlap)
    .payload = 16, // Default payload size

    .spi_host = SPI3_HOST,
    .spi_speed = 1000000, // 1 MHz (explicit value)
    .spi = nullptr,       // Will be initialized during setup
    .status = 0           // Initial status
};

void RADIOHALL::init()
{
    if (Nrf24_init(&nrf24))
    {
        ESP_LOGI(TAG, "NRF24 radio initialized successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize NRF24 radio");
    }
}

void RADIOHALL::denit()
{
    Nrf24_deinit(&nrf24);
}

void RADIOHALL::configureRadio(const uint8_t channel, const uint8_t payload)
{
    Nrf24_config(&nrf24, channel, payload);
}

void RADIOHALL::isConnected()
{
    if (Nrf24_isConnected(&nrf24))
    {
        ESP_LOGI(TAG, "NRF24 radio is connected");
    }
    else
    {
        ESP_LOGE(TAG, "NRF24 radio is not connected");
    }
}

void RADIOHALL::PrintAllNetworks() {
    ESP_LOGI(TAG, "Starting NRF24 network scan...");
    Serial.println("=== NRF24 Network Scanner ===");

    // Check if radio is connected first
    if (!Nrf24_isConnected(&nrf24)) {
        ESP_LOGE(TAG, "NRF24 radio is not connected");
        Serial.println("Error: NRF24 radio not connected");
        return;
    }

    // Set to receive mode for scanning
    if (!Nrf24_receiveMode(&nrf24)) {
        ESP_LOGE(TAG, "Failed to set receive mode");
        Serial.println("Error: Failed to set receive mode");
        return;
    }

    uint8_t networkCount = 0;
    Serial.println("Scanning channels 0-125 for activity...");
    Serial.println("Channel | Frequency | RSSI | Activity");
    Serial.println("--------|-----------|------|----------");

    // Scan all available channels
    for (uint8_t channel = 0; channel < 126; channel++) {
        // Set scanning channel
        if (!Nrf24_setChannel(&nrf24, channel)) {
            continue;
        }

        // Clear previous status
        Nrf24_clearStatus(&nrf24);

        // Listen for a short period
        vTaskDelay(pdMS_TO_TICKS(10));

        // Check for activity using multiple methods
        uint8_t status = Nrf24_getStatus(&nrf24);
        bool activity = false;

        // Method 1: Check RX_DR flag
        if (status & (1 << 6)) {
            activity = true;
        }

        // Method 2: Check if RX FIFO is not empty
        if (!Nrf24_rxFifoEmpty(&nrf24)) {
            activity = true;
        }

        // Method 3: Simple carrier detection (reading register multiple times)
        uint8_t carrier_count = 0;
        for (int i = 0; i < 10; i++) {
            uint8_t reg_val = Nrf24_getStatus(&nrf24);
            if (reg_val != 0x0E) { // Default empty state
                carrier_count++;
            }
            vTaskDelay(1);
        }

        if (carrier_count > 3 || activity) {
            uint16_t frequency = 2400 + channel;
            Serial.printf("  %3d   |  %4d MHz |  N/A | DETECTED\n",
                         channel, frequency);
            ESP_LOGI(TAG, "Activity detected on channel %d (%d MHz)",
                     channel, frequency);
            networkCount++;
        }

        // Show progress every 25 channels
        if (channel % 25 == 0) {
            Serial.printf("Progress: %d/126 channels scanned...\n", channel + 1);
        }

        // Clear any received data
        if (!Nrf24_rxFifoEmpty(&nrf24)) {
            Nrf24_flushRx(&nrf24);
        }
    }

    Serial.println("========================================");
    Serial.printf("Scan complete. Found activity on %d channels\n", networkCount);
    ESP_LOGI(TAG, "Network scan completed. Found activity on %d channels", networkCount);

    if (networkCount == 0) {
        Serial.println("No activity detected. Possible causes:");
        Serial.println("- No other 2.4GHz devices active");
        Serial.println("- Radio sensitivity needs adjustment");
        Serial.println("- Antenna issues");
        Serial.println("- Interference from strong signals");
    }

    // Reset to default channel and mode
    Nrf24_setChannel(&nrf24, nrf24.channel);
    Nrf24_receiveMode(&nrf24);
}

void RADIOHALL::EducationalWiFiInterferenceDemo() {
    ESP_LOGW(TAG, "=== EDUCATIONAL RF INTERFERENCE DEMONSTRATION ===");
    ESP_LOGW(TAG, "WARNING: This is for educational purposes only!");
    ESP_LOGW(TAG, "Ensure you have proper authorization and comply with local laws");
    ESP_LOGW(TAG, "Use only in controlled environments with your own equipment");

    Serial.println("=== Educational WiFi Interference Demo ===");
    Serial.println("WARNING: Educational demonstration only!");
    Serial.println("Do not use without proper authorization!");

    // Check if radio is connected
    if (!Nrf24_isConnected(&nrf24)) {
        ESP_LOGE(TAG, "NRF24 radio is not connected");
        Serial.println("Error: NRF24 radio not connected");
        return;
    }

    // WiFi channels in 2.4GHz band (overlapping with NRF24 channels)
    const uint8_t wifi_channels[][2] = {
        {12, 1},   // NRF24 channel 12 = WiFi channel 1 (2412 MHz)
        {37, 6},   // NRF24 channel 37 = WiFi channel 6 (2437 MHz)
        {62, 11}   // NRF24 channel 62 = WiFi channel 11 (2462 MHz)
    };

    // Educational interference pattern
    uint8_t interference_pattern[32];
    for (int i = 0; i < 32; i++) {
        interference_pattern[i] = 0x55; // Alternating bit pattern for demo
    }

    Serial.println("Demonstrating interference on common WiFi channels:");
    Serial.println("NRF Ch | WiFi Ch | Frequency | Duration");
    Serial.println("-------|---------|-----------|----------");

    // Fixed: Loop through only 3 channels (array size)
    for (int i = 0; i < 3; i++) {
        uint8_t nrf_channel = wifi_channels[i][0];
        uint8_t wifi_channel = wifi_channels[i][1];
        uint16_t frequency = 2400 + nrf_channel;

        Serial.printf("  %2d   |   %2d    | %4d MHz | 5 sec\n",
                     nrf_channel, wifi_channel, frequency);

        ESP_LOGI(TAG, "Educational demo on WiFi channel %d (NRF24 ch %d, freq %d MHz)",
                 wifi_channel, nrf_channel, frequency);

        // Set channel and transmit mode
        if (!Nrf24_setChannel(&nrf24, nrf_channel)) {
            ESP_LOGE(TAG, "Failed to set channel %d", nrf_channel);
            continue;
        }

        if (!Nrf24_transmitMode(&nrf24)) {
            ESP_LOGE(TAG, "Failed to set transmit mode");
            continue;
        }

        // Brief educational transmission (5 seconds per channel)
        TickType_t start_time = xTaskGetTickCount();
        TickType_t demo_duration = pdMS_TO_TICKS(5000); // 5 seconds

        while ((xTaskGetTickCount() - start_time) < demo_duration) {
            // Transmit educational pattern
            if (Nrf24_send1(&nrf24, interference_pattern)) {
                ESP_LOGD(TAG, "Educational pattern transmitted");
            }

            // Short interval between transmissions
            vTaskDelay(pdMS_TO_TICKS(50)); // 50ms intervals
        }

        Serial.printf("Completed demo on WiFi channel %d\n", wifi_channel);
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2 second pause
    }

    // Return to receive mode
    Nrf24_receiveMode(&nrf24);
    Nrf24_setChannel(&nrf24, nrf24.channel);

    Serial.println("Educational demonstration completed");
    ESP_LOGI(TAG, "Educational WiFi interference demonstration completed");
}

extern RADIOHALL radioHall;
