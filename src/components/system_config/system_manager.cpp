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

nvs_handle_t config_handle;

QueueHandle_t batteryQueue = nullptr; // Definition
QueueHandle_t modeQueue = nullptr;   // Definition


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
    RAM_size = esp_spiram_get_size();
    if (RAM_size > 0)
    {
        RAM_size /= (1024 * 1024); // Convert to MB
    }
    else
    {
        RAM_size = 0; // No SPI RAM available
    }

    // Get Flash size
    size_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    FLASH_size = flash_size / (1024 * 1024); // Convert to MB
}

int SystemManager::system_memory(uint8_t memory)
{
    multi_heap_info_t info;

    if (memory == MEMORY_DMA)
        heap_caps_get_info(&info, MALLOC_CAP_DMA);
    else if (memory == MEMORY_INTERNAL)
        heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
    else if (memory == MEMORY_SPIRAM)
        heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    else if (memory == MEMORY_ALL)
        heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    else
    {
        return -1;
    }

    return info.total_free_bytes;
}

void SystemManager::system_init_config()
{
    nvs_flash_init();
}

void SystemManager::system_set_state(int8_t state)
{
    // TODO: Check if it's a valid code
    nvs_open("nvs", NVS_READWRITE, &config_handle);
    nvs_set_i8(config_handle, "prev_state", state);
    nvs_close(config_handle);
}

int8_t SystemManager::system_get_state()
{
    int8_t prev_state;
    nvs_open("nvs", NVS_READWRITE, &config_handle);
    nvs_get_i8(config_handle, "prev_state", &prev_state);
    nvs_close(config_handle);
    return prev_state;
}

void SystemManager::system_save_config(uint8_t config, int8_t value)
{
    nvs_handle_t config_handle;
    esp_err_t err = nvs_open("nvs", NVS_READWRITE, &config_handle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS: %s\n", esp_err_to_name(err));
        return;
    }

    if (config == SYS_BRIGHT && value <= 100)
    {
        err = nvs_set_i8(config_handle, "scr_bright", value);
    }
    else if (config == SYS_VOLUME && value <= 100)
    {
        err = nvs_set_i8(config_handle, "sound_volume", value);
    }
    else if (config == SYS_GUI_COLOR)
    {
        err = nvs_set_i8(config_handle, "GUI_color", value);
    }
    else if (config == SYS_STATE_SAV_BTN)
    {
        err = nvs_set_i8(config_handle, "Save_State", value);
        printf("Set save_State value %i\r\n", value);
    }

    if (err != ESP_OK)
    {
        printf("Error saving config: %s\n", esp_err_to_name(err));
    }

    nvs_close(config_handle);
}

int8_t SystemManager::system_get_config(uint8_t config)
{
    nvs_open("nvs", NVS_READWRITE, &config_handle);
    int8_t value = -1;
    if (config == SYS_BRIGHT && value <= 100)
    {
        nvs_get_i8(config_handle, "scr_bright", &value);
        if (value < 1 || value > 100)
            value = 100;
    }
    else if (config == SYS_VOLUME && value <= 100)
    {
        nvs_get_i8(config_handle, "sound_volume", &value);
        if (value < 0 || value > 100)
            value = 80; // Default value of 80%
    }
    else if (config == SYS_GUI_COLOR)
    {
        nvs_get_i8(config_handle, "GUI_color", &value);
        // if(value != 0 || value != 1) value = 0;
    }
    else if (config == SYS_STATE_SAV_BTN)
    {
        nvs_get_i8(config_handle, "Save_State", &value);
        printf("Value get %i\r\n", value);
    }
    nvs_close(config_handle);

    return value;
}

SystemManager sys_manager;
