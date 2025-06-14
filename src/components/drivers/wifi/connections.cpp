#include "connections.h"
#include "components/system_config/system_manager.h"
#include "components/system_config/system_config.h"

#include <WiFi.h>
#include <task.h>

static const char *TAG = "CONNECTIONS";



/*
 * SSID could be wrong
 *
 * SSID isn't a valid Wi-Fi network
 *
 * SSID is a valid Wi-Fi network, but the password is wrong
 *
 */

QueueHandle_t WIFI_queue = nullptr;
WiFiState WIFI_CONNECTIONS::currentState = NO_CREDENTIALS;


bool WIFI_CONNECTIONS::begin_wifi() {
// check the saved Wi-Fi credentials if exist return true else return false
    if ( SystemManager::getCredentials("wifi_name") == nullptr ||
         SystemManager::getCredentials("wifi_password") == nullptr){
         currentState = NO_CREDENTIALS;
         SystemManager::saveCredentials("wifi_name", "WIFISSID");
         SystemManager::saveCredentials("wifi_password", "WIFIPASSWORD");
        return false;
    }
    return true;
}

[[noreturn]] void WIFI_CONNECTIONS::wifi_task(void *pvParameters) {
    while (true) {
        uint8_t start_wifi;
        if (xQueueReceive(WIFI_queue, &start_wifi, portMAX_DELAY) == pdTRUE && start_wifi == 1) {
            const auto ssid = SystemManager::getCredentials("wifi_name");
            const auto password =  SystemManager::getCredentials("wifi_password");

            ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
            int attempts = 0;
            if (WiFiClass::status() != WL_CONNECTED) {
                WiFi.begin(ssid, password);
                currentState = CONNECTING;
            }
            if (!ssid || !password) {
                currentState = NO_CREDENTIALS;
                continue;
            }
            while (WiFiClass::status() != WL_CONNECTED && attempts < 10) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
                attempts++;
            }
            if (WiFiClass::status() == WL_CONNECTED) {
                ESP_LOGE(TAG, "Connected to Wi-Fi!");
                ESP_LOGI(TAG, "IP address: ");
                ESP_LOGI(TAG, WiFi.localIP());

                currentState = CONNECTED;
            } else {
                const wl_status_t status = WiFiClass::status();
                if (status == WL_CONNECT_FAILED) {
                    currentState = WRONG_PASSWORD;
                    ESP_LOGI(TAG, "Connection failed.");
                } else if (status == WL_NO_SSID_AVAIL) {
                    currentState = AVAILABLE;
                    ESP_LOGI(TAG, "Connection failed.");
                } else {
                    currentState = CONNECTION_FAILED;
                    ESP_LOGI(TAG, "Failed to connect.");
                }
                WiFi.disconnect();
            }
        }
    }
}


void WIFI_CONNECTIONS::wifi_init()
{
    // Create a queue first
    WIFI_queue = xQueueCreate(5, sizeof(uint8_t));

    // Create a task
    TaskHandle_t wifi_task_handle = nullptr;
    xTaskCreate(wifi_task, "WIFI_TASK", 4096, nullptr, 1, &wifi_task_handle);  // Increased from 2096

    // Initialize Wi-Fi after queue and task are ready
    if (begin_wifi()) {
        constexpr uint8_t start_wifi = 1;
        xQueueSend(WIFI_queue, &start_wifi, portMAX_DELAY);
    }

    system_wifi.status = ONN;
    sys_manager.system_save_config(SYS_WIFI, ONN);
}

void WIFI_CONNECTIONS::wifi_deinit()
{
    WiFi.disconnect();
    system_wifi.status = OFF;
    sys_manager.system_save_config(SYS_WIFI, OFF);
}

WiFiState WIFI_CONNECTIONS::getState() {
    return currentState;
}

bool WIFI_CONNECTIONS::changePassword(char *ssid, char *password) {
    if (!ssid || !password) return false;
    sys_manager.saveCredentials("wifi_name", ssid);
    sys_manager.saveCredentials("wifi_password", password);
    return true;
}



WIFI_CONNECTIONS wifi;