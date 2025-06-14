#include "connections.h"
#include "components/system_config/system_manager.h"
#include "components/system_config/system_config.h"
#include "components/core/processManager.h"

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
uint32_t WIFI_CONNECTIONS::wifi_process_id = 0;

bool WIFI_CONNECTIONS::begin_wifi() {
    // check the saved Wi-Fi credentials if exist return true else return false
    if (SystemManager::getCredentials("wifi_name") == nullptr ||
        SystemManager::getCredentials("wifi_password") == nullptr) {
        currentState = NO_CREDENTIALS;
        SystemManager::saveCredentials("wifi_name", "WIFISSID");
        SystemManager::saveCredentials("wifi_password", "WIFIPASSWORD");
        return false;
    }
    return true;
}

[[noreturn]] void WIFI_CONNECTIONS::wifi_task(void *pvParameters) {
    ESP_LOGI(TAG, "WiFi task started and managed by ProcessManager");

    while (true) {
        // Check if process is still alive and should continue running
        if (!process_manager.is_process_alive(wifi_process_id)) {
            ESP_LOGW(TAG, "WiFi process is no longer alive, terminating task");
            break;
        }

        uint8_t start_wifi;
        if (xQueueReceive(WIFI_queue, &start_wifi, pdMS_TO_TICKS(5000)) == pdTRUE && start_wifi == 1) {
            const auto ssid = SystemManager::getCredentials("wifi_name");
            const auto password = SystemManager::getCredentials("wifi_password");

            if (!ssid || !password) {
                currentState = NO_CREDENTIALS;
                ESP_LOGW(TAG, "No WiFi credentials available");
                continue;
            }

            ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
            int attempts = 0;
            
            if (WiFiClass::status() != WL_CONNECTED) {
                WiFi.begin(ssid, password);
                currentState = CONNECTING;
            }

            while (WiFiClass::status() != WL_CONNECTED && attempts < 10) {
                // Check if process should continue during connection attempts
                if (!process_manager.is_process_alive(wifi_process_id)) {
                    ESP_LOGW(TAG, "WiFi process terminated during connection attempt");
                    WiFi.disconnect();
                    vTaskDelete(NULL);
                    return;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP_LOGI(TAG, "Attempting to connect to SSID: %s (attempt %d/10)", ssid, attempts + 1);
                attempts++;
            }

            if (WiFiClass::status() == WL_CONNECTED) {
                ESP_LOGI(TAG, "Connected to Wi-Fi!");
                ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
                currentState = CONNECTED;
            } else {
                const wl_status_t status = WiFiClass::status();
                if (status == WL_CONNECT_FAILED) {
                    currentState = WRONG_PASSWORD;
                    ESP_LOGW(TAG, "Connection failed - wrong password");
                } else if (status == WL_NO_SSID_AVAIL) {
                    currentState = AVAILABLE;
                    ESP_LOGW(TAG, "Connection failed - SSID not available");
                } else {
                    currentState = CONNECTION_FAILED;
                    ESP_LOGW(TAG, "Failed to connect - unknown error");
                }
                WiFi.disconnect();
            }
        } else {
            // Timeout occurred, check connection status
            if (WiFiClass::status() == WL_CONNECTED) {
                if (currentState != CONNECTED) {
                    currentState = CONNECTED;
                    ESP_LOGI(TAG, "WiFi connection maintained");
                }
            } else if (currentState == CONNECTED) {
                currentState = CONNECTION_FAILED;
                ESP_LOGW(TAG, "WiFi connection lost");
            }
        }
    }

    ESP_LOGI(TAG, "WiFi task terminating");
    vTaskDelete(NULL);
}

void WIFI_CONNECTIONS::wifi_init()
{
    // Create a queue first
    WIFI_queue = xQueueCreate(5, sizeof(uint8_t));
    if (!WIFI_queue) {
        ESP_LOGE(TAG, "Failed to create WiFi queue");
        return;
    }

    // Create WiFi process using ProcessManager instead of xTaskCreate
    wifi_process_id = process_manager.create_process(
        "WiFi_Task",               // Process name
        wifi_task,                 // Task function
        4096,                      // Stack size (increased from 2096)
        nullptr,                   // Parameters
        PROCESS_PRIORITY_NORMAL,   // Priority
        tskNO_AFFINITY            // Core affinity
    );

    if (wifi_process_id == 0) {
        ESP_LOGE(TAG, "Failed to create WiFi process");
        // Clean up queue if process creation failed
        if (WIFI_queue) {
            vQueueDelete(WIFI_queue);
            WIFI_queue = nullptr;
        }
        return;
    }

    ESP_LOGI(TAG, "WiFi process created with ID: %lu", wifi_process_id);

    // Initialize Wi-Fi after queue and task are ready
    if (begin_wifi()) {
        constexpr uint8_t start_wifi = 1;
        if (xQueueSend(WIFI_queue, &start_wifi, pdMS_TO_TICKS(1000)) != pdPASS) {
            ESP_LOGW(TAG, "Failed to send initial WiFi start command");
        }
    }

    system_wifi.status = ONN;
    sys_manager.system_save_config(SYS_WIFI, ONN);
}

void WIFI_CONNECTIONS::wifi_deinit()
{
    ESP_LOGI(TAG, "Deinitializing WiFi");
    
    // Stop WiFi process
    stop_wifi_process();
    
    // Disconnect WiFi
    WiFi.disconnect();
    
    // Clean up queue
    if (WIFI_queue) {
        vQueueDelete(WIFI_queue);
        WIFI_queue = nullptr;
    }
    
    // Update state and save configuration
    currentState = NO_CREDENTIALS;
    system_wifi.status = OFF;
    sys_manager.system_save_config(SYS_WIFI, OFF);
    
    ESP_LOGI(TAG, "WiFi deinitialized");
}

WiFiState WIFI_CONNECTIONS::getState() {
    return currentState;
}

bool WIFI_CONNECTIONS::changePassword(char *ssid, char *password) {
    if (!ssid || !password) {
        ESP_LOGE(TAG, "Invalid SSID or password provided");
        return false;
    }
    
    ESP_LOGI(TAG, "Changing WiFi credentials to SSID: %s", ssid);
    
    // Save new credentials
    sys_manager.saveCredentials("wifi_name", ssid);
    sys_manager.saveCredentials("wifi_password", password);
    
    // Trigger reconnection with new credentials
    return restart_wifi_connection();
}

// Process management functions
void WIFI_CONNECTIONS::stop_wifi_process() {
    if (wifi_process_id != 0) {
        ESP_LOGI(TAG, "Stopping WiFi process ID: %lu", wifi_process_id);
        process_manager.delete_process(wifi_process_id);
        wifi_process_id = 0;
    }
}

void WIFI_CONNECTIONS::suspend_wifi_process() {
    if (wifi_process_id != 0) {
        ESP_LOGI(TAG, "Suspending WiFi process ID: %lu", wifi_process_id);
        process_manager.suspend_process(wifi_process_id);
    }
}

void WIFI_CONNECTIONS::resume_wifi_process() {
    if (wifi_process_id != 0) {
        ESP_LOGI(TAG, "Resuming WiFi process ID: %lu", wifi_process_id);
        process_manager.resume_process(wifi_process_id);
    }
}

uint32_t WIFI_CONNECTIONS::get_wifi_process_id() {
    return wifi_process_id;
}

bool WIFI_CONNECTIONS::restart_wifi_connection() {
    if (wifi_process_id == 0) {
        ESP_LOGW(TAG, "No WiFi process running to restart");
        return false;
    }
    
    // Disconnect current connection
    WiFi.disconnect();
    currentState = CONNECTING;
    
    // Send restart command to WiFi task
    constexpr uint8_t start_wifi = 1;
    if (xQueueSend(WIFI_queue, &start_wifi, pdMS_TO_TICKS(1000)) == pdPASS) {
        ESP_LOGI(TAG, "WiFi restart command sent");
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to send WiFi restart command");
        return false;
    }
}

WIFI_CONNECTIONS wifi;
