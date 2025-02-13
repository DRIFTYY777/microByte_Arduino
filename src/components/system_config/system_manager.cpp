#include "system_manager.h"

#include "nvs_flash.h"
#include "nvs.h"

#include <multi_heap.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <soc/soc.h>
#include <esp_ota_ops.h>
#include <esp32/spiram.h>

#include "esp_system.h" // For esp_chip_info()
#include <Esp.h>
#include <SPIFFS.h>

static const char *TAG = "SystemManager";

// nvs_handle_t config_handle;

QueueHandle_t batteryQueue; // Definition
QueueHandle_t modeQueue;    // Definition

char app_version[32];
char idf_version[32];
char cpu_version[32];
uint32_t RAM_size;
uint32_t FLASH_size;

void SystemManager::system_info()
{
    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    if (app_desc != nullptr)
    {
        // Get App version
        strncpy(app_version, app_desc->version, sizeof(app_version) - 1);
        app_version[sizeof(app_version) - 1] = '\0'; // Ensure null-termination
    }
    else
    {
        strcpy(app_version, "Unknown");
    }

    // Get Chip information using esp_chip_info()
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    // Based on the chip type, assign the CPU version
    if (chip_info.model == CHIP_ESP32S3)
    {
        strncpy(cpu_version, "ESP32-S3", sizeof(cpu_version) - 1);
    }
    else
    {
        strncpy(cpu_version, "UNKNOWN", sizeof(cpu_version) - 1);
    }

    cpu_version[sizeof(cpu_version) - 1] = '\0'; // Ensure null-termination

    // Get RAM size
    RAM_size = ESP.getFreeHeap() / (1024 * 1024); // Get RAM size in MB
    if (RAM_size == 0)
    {
        RAM_size = 0; // No SPI RAM available
    }

    // Get Flash size
    FLASH_size = ESP.getFlashChipSize() / (1024 * 1024); // Get Flash size in MB
}

int SystemManager::system_memory(uint8_t memory)
{
    // For Arduino, heap management isn't as flexible as in ESP-IDF, so this is simplified.
    if (memory == MEMORY_DMA)
        return ESP.getFreeHeap();
    else if (memory == MEMORY_INTERNAL)
        return ESP.getFreeHeap();
    else if (memory == MEMORY_SPIRAM)
        return ESP.getFreePsram();
    else if (memory == MEMORY_ALL)
        return ESP.getFreeHeap() + ESP.getFreePsram();
    else
    {
        return -1;
    }
}

void SystemManager::system_init_config()
{
    // Initialize SPIFFS filesystem
    if (!SPIFFS.begin(true))
    {
        ESP_LOGE(TAG, "Failed to mount file system");
        return;
    }
    ESP_LOGE(TAG, "SPIFFS Initialized");
}

void SystemManager::system_set_state(int8_t state)
{
    File configFile = SPIFFS.open("/config.txt", "w");
    if (configFile)
    {
        configFile.write(state);
        configFile.close();
    }
    else
    {
        ESP_LOGE(TAG, "Failed to open config file for writing");
    }
}

int8_t SystemManager::system_get_state()
{
    File configFile = SPIFFS.open("/config.txt", "r");
    if (configFile)
    {
        int8_t prev_state = configFile.read();
        configFile.close();
        return prev_state;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to open config file for reading");
        return -1;
    }
}

void SystemManager::system_save_config(uint8_t config, int8_t value)
{
    File configFile = SPIFFS.open("/config.txt", "w");
    if (configFile)
    {
        if (config == SYS_BRIGHT && value <= 100)
        {
            configFile.print("scr_bright=");
            configFile.println(value);
        }
        else if (config == SYS_VOLUME && value <= 100)
        {
            configFile.print("sound_volume=");
            configFile.println(value);
        }
        else if (config == SYS_GUI_COLOR)
        {
            configFile.print("GUI_color=");
            configFile.println(value);
        }
        else if (config == SYS_STATE_SAV_BTN)
        {
            configFile.print("Save_State=");
            configFile.println(value);
        }
        configFile.close();
    }
    else
    {
        ESP_LOGE(TAG, "Error saving config to file");
    }
}

int8_t SystemManager::system_get_config(uint8_t config)
{
    File configFile = SPIFFS.open("/config.txt", "r");
    int8_t value = -1;

    if (configFile)
    {
        while (configFile.available())
        {
            String line = configFile.readStringUntil('\n');
            if (line.startsWith("scr_bright=") && config == SYS_BRIGHT)
            {
                value = line.substring(12).toInt();
            }
            else if (line.startsWith("sound_volume=") && config == SYS_VOLUME)
            {
                value = line.substring(14).toInt();
            }
            else if (line.startsWith("GUI_color=") && config == SYS_GUI_COLOR)
            {
                value = line.substring(10).toInt();
            }
            else if (line.startsWith("Save_State=") && config == SYS_STATE_SAV_BTN)
            {
                value = line.substring(11).toInt();
            }
        }
        configFile.close();
    }
    else
    {
        ESP_LOGE(TAG, "Error reading config file");
    }
    return value;
}

SystemManager sys_manager;