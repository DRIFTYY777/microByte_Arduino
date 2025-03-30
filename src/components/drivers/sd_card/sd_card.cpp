#include "sd_card.h"

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include "formatSD.h"

#include <SD.h>
#include <SD_MMC.h>
#include <sys/dirent.h>
#include <esp_vfs_fat.h>

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
    uint64_t usedSpace = cardSize - (SD_MMC.usedBytes() / (1024 * 1024));

    ESP_LOGE(TAG, "SD Card Size: %lluMB", cardSize);

    // Update struct info
    sd_card_info.card_type = cardType;
    sd_card_info.card_size = static_cast<uint16_t>(cardSize);
    sd_card_info.card_mounted = 1;
    sd_card_info.card_status = 1; // Assume status OK
    sd_card_info.used_space = static_cast<uint8_t>(usedSpace);
    sd_card_info.free_space = static_cast<uint8_t>(cardSize - usedSpace);

    SD_mount = true;

    system_dir();
    emulator_dir();

    return true;
}

#define SPI_DMA_CHAN 1
#define SPI_DMA_CHAN2 2

// begin spi

bool SD_CARD::sd_init2()
{
    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = VSPI_MOSI,
        .miso_io_num = VSPI_MISO,
        .sclk_io_num = VSPI_CLK,
        .quadwp_io_num = -1,    // Not used
        .quadhd_io_num = -1,    // Not used
        .max_transfer_sz = 4000 // Maximum transfer size
    };

    ESP_LOGI(TAG, "Initializing SPI bus on VSPI_HOST...");
    esp_err_t ret = spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "SPI bus initialized successfully.");
    }
    else
    {
        ESP_LOGE(TAG, "SPI bus initialization failed: %s", esp_err_to_name(ret));
        return false;
    }

    // Configure SD card interface
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = VSPI_HOST; // Use VSPI hardware SPI

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(SD_CS); // Chip select pin
    slot_config.host_id = VSPI_HOST;

    // Mount the filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024 // 16 KB
    };

    sdmmc_card_t *card;
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount filesystem: %s", esp_err_to_name(ret));
        spi_bus_free(VSPI_HOST); // Free the SPI bus if mount fails
        return false;
    }

    ESP_LOGI(TAG, "SD card initialized successfully.");
    SD_mount = true;
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

bool SD_CARD::is_card_mounted()
{
    return SD_mount;
}

uint8_t SD_CARD::sd_app_list(char *app_list[100], bool update)
{
    struct dirent *entry;
    DIR *dir = NULL;
    if (update)
        dir = opendir("/sdcard/Firmware");
    else
        dir = opendir("/sdcard/External_Apps");

    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory");
        return 0;
    }
    // Only find .bin file on the apps folder
    uint8_t i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        size_t nameLength = strlen(entry->d_name);
        app_list[i] = (char *)malloc(nameLength + 1);
        if (strcmp(entry->d_name + (nameLength - 4), ".bin") == 0)
        {
            sprintf(app_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", app_list[i]);
            i++;
        }
    }
    return i;
}

size_t SD_CARD::sd_file_size(const char *path)
{
    // TODO: There is a bug which with some specific letters combination, fail opening the file and crash the program
    FILE *fd = fopen(path, "rb");

    fseek(fd, 0, SEEK_END);
    size_t actual_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    ESP_LOGI(TAG, "Size: %i bytes", actual_size);
    fclose(fd);

    return actual_size;
}

uint8_t SD_CARD::sd_game_list(char *game_list[100], uint8_t console)
{
    struct dirent *entry;

    // Open the folder of the specific console
    DIR *dir = NULL;
    if (console == NES)
        dir = opendir("/sdcard/Emulator/NES");
    else if (console == GAMEBOY)
        dir = opendir("/sdcard/Emulator/GameBoy");
    else if (console == GAMEBOY_COLOR)
        dir = opendir("/sdcard/Emulator/GameBoy_Color");
    else if (console == SNES)
        dir = opendir("/sdcard/Emulator/SNES");
    else if (console == SMS)
        dir = opendir("/sdcard/Emulator/Master_System");
    else if (console == GG)
        dir = opendir("/sdcard/Emulator/Game_Gear");

    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to stat dir : 0x%02x", console);
        return 0;
    }
    // Loop to find the game of each console base on the file extension
    uint8_t i = 0;
    while ((entry = readdir(dir)) != NULL)
    {

        size_t nameLength = strlen(entry->d_name);

        // TODO: Rework game list maker, set by alphabetical order
        if ((strcmp(entry->d_name + (nameLength - 4), ".nes") == 0) && console == NES)
        {
            game_list[i] = (char *)malloc(256);
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 3), ".gb") == 0) && console == GAMEBOY)
        {
            game_list[i] = (char *)malloc(256);
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 4), ".gbc") == 0) && console == GAMEBOY_COLOR)
        {
            game_list[i] = (char *)malloc(256);
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 4), ".sms") == 0) && console == SMS)
        {
            game_list[i] = (char *)malloc(256);
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 3), ".gg") == 0) && console == GG)
        {
            game_list[i] = (char *)malloc(256);
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
    }

    // Return the number of files
    return i;
}

bool SD_CARD::sd_sav_exist(char *file_name, uint8_t emulator)
{

    char *file_route = (char *)malloc(256);
    if (emulator == GAMEBOY)
        sprintf(file_route, "/sdcard/Emulator/GameBoy/Save_Data/%s.sav", file_name);
    else if (emulator == GAMEBOY_COLOR)
        sprintf(file_route, "/sdcard/Emulator/GameBoy_Color/Save_Data/%s.sav", file_name);
    else if (emulator == NES)
        sprintf(file_route, "/sdcard/Emulator/NES/Save_Data/%s.sav", file_name);
    else if (emulator == SMS)
        sprintf(file_route, "/sdcard/Emulator/Master_System/Save_Data/%s.sav", file_name);
    else if (emulator == GG)
        sprintf(file_route, "/sdcard/Emulator/Game_Gear/Save_Data/%s.sav", file_name);

    struct stat st;
    if (stat(file_route, &st) == -1)
    {
        free(file_route);
        return false;
    }
    free(file_route);
    return true;
}

void SD_CARD::sd_sav_remove(char *file_name, uint8_t emulator)
{
    char *file_route = (char *)malloc(256);
    if (emulator == GAMEBOY)
        sprintf(file_route, "/sdcard/Emulator/GameBoy/Save_Data/%s.sav", file_name);
    else if (emulator == GAMEBOY_COLOR)
        sprintf(file_route, "/sdcard/Emulator/GameBoy_Color/Save_Data/%s.sav", file_name);
    else if (emulator == NES)
        sprintf(file_route, "/sdcard/Emulator/NES/Save_Data/%s.sav", file_name);
    else if (emulator == SMS)
        sprintf(file_route, "/sdcard/Emulator/Master_System/Save_Data/%s.sav", file_name);
    else if (emulator == GG)
        sprintf(file_route, "/sdcard/Emulator/Game_Gear/Save_Data/%s.sav", file_name);

    remove(file_route);
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
    // Master System
    if (stat("/sdcard/Emulator/Master_System", &st) == -1)
    {
        ESP_LOGI(TAG, "No Master_System folder found, creating it");
        mkdir("/sdcard/Emulator/Master System", 0700);
        mkdir("/sdcard/Emulator/Master System/Save_Data", 0700);
    }
}

void SD_CARD::system_dir()
{
    // Check if the system folders exist and create them if they don't
    struct stat st;

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