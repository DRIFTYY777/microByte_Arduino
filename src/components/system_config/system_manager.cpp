#include "system_manager.h"

#include <multi_heap.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <soc/soc.h>
#include <esp_ota_ops.h>
// #include <esp32/spiram.h>
#include <esp_spi_flash.h>

#include <Preferences.h>

#include "esp_system.h" // For esp_chip_info()
#include <Esp.h>
#include <esp_chip_info.h>

static const char *TAG = "SystemManager";

Preferences preferences; // NVS storage handler

QueueHandle_t modeQueue;
QueueHandle_t batteryQueue;
QueueHandle_t vidQueue;
TimerHandle_t timer = NULL;

SYSTEM_MODE management;        // Only declare here
APPS app;                      // Only declare here
BATTERY_STATUS battery_status; // Only declare here
SYSTEM_WIFI system_wifi;       // Only declare here

char app_version[32];
char idf_version[32];
char cpu_version[32];

uint32_t Internal_ram;
uint32_t Used_Internal_ram;
uint32_t SPI_ram;
uint32_t Used_SPI_ram;
uint32_t Free_Flash;
uint32_t Flash_Size;

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

    // Get Internal RAM size
    Internal_ram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    Used_Internal_ram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    // Get SPI RAM size
    SPI_ram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    Used_SPI_ram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    // Get Flash size
    Flash_Size = ESP.getFlashChipSize() / (1024 * 1024); // Get Flash size in MB
    Free_Flash = ESP.getFreeSketchSpace();
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
    preferences.begin("nvs", false);

    // Initialize the wifi based on the saved state
    system_wifi.status = (uint8_t)preferences.getChar("wifi", OFF);
}

void SystemManager::system_set_state(int8_t state)
{
    preferences.putChar("prev_state", (char)state);
}

int8_t SystemManager::system_get_state()
{
    return (int8_t)preferences.getChar("prev_state", -1); // -1 is default if not found
}

void SystemManager::system_save_config(uint8_t config, int8_t value)
{
    if (config == SYS_BRIGHT && value <= 100)
    {
        preferences.putChar("scr_bright", (char)value);
    }
    else if (config == SYS_VOLUME && value <= 100)
    {
        preferences.putChar("sound_volume", (char)value);
    }
    else if (config == SYS_GUI_COLOR)
    {
        preferences.putChar("GUI_color", (char)value);
    }
    else if (config == SYS_STATE_SAV_BTN)
    {
        preferences.putChar("Save_State", (char)value);
    }
    else if (config == SYS_VIBRATION)
    {
        preferences.putChar("vibration", (char)value);
    }
    else if (config == SYS_WIFI)
    {
        preferences.putChar("wifi", (char)value);
    }
    else if (config == SYS_BLUETOOTH)
    {
        preferences.putChar("bluetooth", (char)value);
    }
}

int8_t SystemManager::system_get_config(uint8_t config)
{
    int8_t value = -1; // Default value

    if (config == SYS_BRIGHT)
    {
        value = (int8_t)preferences.getChar("scr_bright", 100); // Default 100
        if (value < 1 || value > 100)
            value = 100;
    }
    else if (config == SYS_VOLUME)
    {
        value = (int8_t)preferences.getChar("sound_volume", 80); // Default 80
        if (value < 0 || value > 100)
            value = 80;
    }
    else if (config == SYS_GUI_COLOR)
    {
        value = (int8_t)preferences.getChar("GUI_color", -1); // Default -1
    }
    else if (config == SYS_STATE_SAV_BTN)
    {
        value = (int8_t)preferences.getChar("Save_State", -1);
    }
    else if (config == SYS_VIBRATION)
    {

        value = (int8_t)preferences.getChar("vibration", -1);
    }
    else if (config == SYS_WIFI)
    {
        value = (int8_t)preferences.getChar("wifi", -1);
    }
    else if (config == SYS_BLUETOOTH)
    {
        value = (int8_t)preferences.getChar("bluetooth", -1);
    }

    return value;
}

SystemManager sys_manager;