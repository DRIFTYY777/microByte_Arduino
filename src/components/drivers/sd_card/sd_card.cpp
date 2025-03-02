#include "sd_card.h"

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include "formatSD.h"

#include <SD.h>
#include <SD_MMC.h>

static const char *TAG = "SD_CARD";

SD_CARD_INFO sd_card_info;

bool SD_CARD::sd_init()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    SD_MMC.setPins(VSPI_CLK, VSPI_MOSI, VSPI_MISO);

    if (!SD_MMC.begin(MOUNT_POINT, true))
    {
        ESP_LOGE(TAG, "Card Mount Failed");
        SD_mount = false;
        sd_card_info.card_mounted = 0;
        return false;
    }

    delay(100);
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        ESP_LOGE(TAG, "No SD card attached");
        SD_mount = false;
        sd_card_info.card_mounted = 0;
        return false;
    }

    ESP_LOGI(TAG, "SD Card Type: ");
    switch (cardType)
    {
    case CARD_MMC:
        ESP_LOGI(TAG, "MMC");
        break;
    case CARD_SD:
        ESP_LOGI(TAG, "SDSC");
        break;
    case CARD_SDHC:
        ESP_LOGI(TAG, "SDHC");
        break;
    default:
        ESP_LOGI(TAG, "Unknown");
        break;
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    ESP_LOGE(TAG, "SD Card Size: %lluMB", cardSize);

    // Update struct info
    sd_card_info.card_type = cardType;
    sd_card_info.card_size = static_cast<uint16_t>(cardSize);
    sd_card_info.card_mounted = 1;
    sd_card_info.card_status = 1; // Assume status OK
    SD_mount = true;

    system_dir();
    emulator_dir();

    return true;
}

bool SD_CARD::sd_default()
{
    // Check if the SD card is mounted
    if (!SD_mount)
    {
        ESP_LOGE(TAG, "SD Card not mounted");
        return false;
    }
    // Frmat the SD card
    format_sdcard();
    // Check if the Emulator and System folders exist and create them if they don't
    emulator_dir();
    system_dir();
    return true;
}

void SD_CARD::emulator_dir()
{
    // Check if the emulator folders exist and create them if they don't
    struct stat st;
    // create a directory named Emulator in the root directory
    if (stat("/sdcard/Emulator", &st) == -1)
    {
        ESP_LOGI(TAG, "No Emulator folder found, creating it");
        mkdir("/sdcard/Emulator", 0700);
    }
    // NES
    if (stat("/sdcard/Emulator/NES", &st) == -1)
    {
        ESP_LOGI(TAG, "No NES folder found, creating it");
        mkdir("/sdcard/Emulator/NES", 0700);
        mkdir("/sdcard/Emulator/NES/Save_Data", 0700);
    }
    // GameBoy Color
    if (stat("/sdcard/Emulator/GameBoy_Color", &st) == -1)
    {
        ESP_LOGI(TAG, "No GameBoy Color folder found, creating it");
        mkdir("/sdcard/Emulator/GameBoy_Color", 0700);
        mkdir("/sdcard/Emulator/GameBoy_Color/Save_Data", 0700);
    }
    // GameBoy
    if (stat("/sdcard/Emulator/GameBoy", &st) == -1)
    {
        ESP_LOGI(TAG, "No GameBoy folder found, creating it");
        mkdir("/sdcard/Emulator/GameBoy", 0700);
        mkdir("/sdcard/Emulator/GameBoy/Save_Data", 0700);
    }
    // SNES
    if (stat("/sdcard/Emulator/SNES", &st) == -1)
    {
        ESP_LOGI(TAG, "No SNES folder found, creating it");
        mkdir("/sdcard/Emulator/SNES", 0700);
        mkdir("/sdcard/Emulator/SNES/Save_Data", 0700);
    }
    // Game Gear
    if (stat("/sdcard/Emulator/Game_Gear", &st) == -1)
    {
        ESP_LOGI(TAG, "No Game_Gear folder found, creating it");
        mkdir("/sdcard/Emulator/Game_Gear", 0700);
        mkdir("/sdcard/Emulator/Game_Gear/Save_Data", 0700);
    }
}

void SD_CARD::system_dir()
{
    // Check if the system folders exist and create them if they don't
    struct stat st;
    // System
    if (stat("/sdcard/Master System", &st) == -1)
    {
        ESP_LOGI(TAG, "No Master_System folder found, creating it");
        mkdir("/sdcard/Master System", 0700);
        mkdir("/sdcard/Master System/Save_Data", 0700);
    }
    // Apps
    if (stat("/sdcard/Internal Apps", &st) == -1)
    {
        ESP_LOGI(TAG, "No Apps folder found, creating it");
        mkdir("/sdcard/Apps", 0700);
    }
    // External Apps
    if (stat("/sdcard/External Apps", &st) == -1)
    {
        ESP_LOGI(TAG, "No External_Apps folder found, creating it");
        mkdir("/sdcard/External_Apps", 0700);
    }
    // Firmware
    if (stat("/sdcard/Firmware", &st) == -1)
    {
        ESP_LOGI(TAG, "No Firmware folder found, creating it");
        mkdir("/sdcard/Firmware", 0700);
    }
}

SD_CARD sd_card;