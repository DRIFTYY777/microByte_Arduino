#include "sd_card.h"
#include <esp32-hal-log.h>
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>
#include <esp_vfs_fat.h>

#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_err.h"
#include "esp_log.h"

#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include <hal/spi_types.h>
#include <SPI.h>
#include <SD.h>

static const char *TAG = "SD_CARD";

struct sd_card_info sd_card_info;

void SD_CARD::organize_list(char *list[30], uint8_t index)
{
    uint8_t index_aux = index;
    char *list_aux = (char *)malloc(256);

    while (strcmp(list[index_aux - 1], list[index_aux]) > 0 && index_aux > 0)
    {
        memcpy(list_aux, list[index_aux - 1], 255);
        memcpy(list[index_aux - 1], list[index_aux], 255);
        memcpy(list[index_aux], list_aux, 255);
        index_aux--;
        if (!index_aux)
            break;
    }
    free(list_aux);
}

bool SD_CARD::sd_init()
{
    ESP_LOGI(TAG, "Init SD Card");

    // Initialize SPI bus for SD card communication
    SPI.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_CS0); // Pins for SPI communication

    if (!SD.begin(VSPI_CS0, SPI, SPI_SPEED))
    {
        ESP_LOGE(TAG, "SD card initialization failed.");
        return false;
    }

    ESP_LOGI(TAG, "SD Card initialized successfully");

    // Check if the folder tree exists. If not create it.
    struct stat st;
    if (stat("/sdcard/NES", &st) == -1)
    {
        ESP_LOGI(TAG, "No NES folder found, creating it");
        mkdir("/sdcard/NES", 0700);
        mkdir("/sdcard/NES/Save_Data", 0700);
    }
    if (stat("/sdcard/GameBoy_Color", &st) == -1)
    {
        ESP_LOGI(TAG, "No GameBoy Color folder found, creating it");
        mkdir("/sdcard/GameBoy_Color", 0700);
        mkdir("/sdcard/GameBoy_Color/Save_Data", 0700);
    }
    if (stat("/sdcard/GameBoy", &st) == -1)
    {
        ESP_LOGI(TAG, "No GameBoy folder found, creating it");
        mkdir("/sdcard/GameBoy", 0700);
        mkdir("/sdcard/GameBoy/Save_Data", 0700);
    }
    if (stat("/sdcard/SNES", &st) == -1)
    {
        ESP_LOGI(TAG, "No SNES folder found, creating it");
        mkdir("/sdcard/SNES", 0700);
        mkdir("/sdcard/SNES/Save_Data", 0700);
    }
    if (stat("/sdcard/Master_System", &st) == -1)
    {
        ESP_LOGI(TAG, "No Master_System folder found, creating it");
        mkdir("/sdcard/Master_System", 0700);
        mkdir("/sdcard/Master_System/Save_Data", 0700);
    }
    if (stat("/sdcard/Game_Gear", &st) == -1)
    {
        ESP_LOGI(TAG, "No Game_Gear folder found, creating it");
        mkdir("/sdcard/Game_Gear", 0700);
        mkdir("/sdcard/Game_Gear/Save_Data", 0700);
    }
    if (stat("/sdcard/apps", &st) == -1)
    {
        ESP_LOGI(TAG, "No apps folder found, creating it");
        mkdir("/sdcard/apps", 0700);
    }

    SD_mount = true;

    // Get data from the SD card
    File card = SD.open("/");
    if (!card)
    {
        ESP_LOGE(TAG, "Failed to open SD card.");
        return false;
    }

    // sd_card_info.card_size = SD.cardSize() / (1024 * 1024);  // Size in MB
    // strcpy(sd_card_info.card_name, "SDCard");
    // sd_card_info.card_speed = 25000; // Set a default speed (adjust as necessary)

    // sd_card_info.card_type = SD_CARD_TYPE;

    sd_card_info.card_size = SD.cardSize() / (1024 * 1024); // Size in MB
    strcpy(sd_card_info.card_name, "SDCard");

    sd_card_info.card_speed = 25000; // Set a default speed (adjust as necessary)
    sd_card_info.card_type = SD.cardType();

    if (SD.cardType() == CARD_NONE)
    {
        ESP_LOGE(TAG, "No SD card attached");
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "SD Card detected:\n -Name: %s\n -Capacity: %i MB\n -Speed: %i Khz\n -Type: %i",
                 sd_card_info.card_name, sd_card_info.card_size, sd_card_info.card_speed, sd_card_info.card_type);
    }

    return true;
}

uint8_t SD_CARD::sd_game_list(char *game_list[100], uint8_t console)
{
    struct dirent *entry;
    // Open the folder of the specific console
    DIR *dir = NULL;
    if (console == NES)
        dir = opendir("/sdcard/NES");
    else if (console == GAMEBOY)
        dir = opendir("/sdcard/GameBoy");
    else if (console == GAMEBOY_COLOR)
        dir = opendir("/sdcard/GameBoy_Color");
    else if (console == SNES)
        dir = opendir("/sdcard/SNES");
    else if (console == SMS)
        dir = opendir("/sdcard/Master_System");
    else if (console == GG)
        dir = opendir("/sdcard/Game_Gear");

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
            game_list[i] = (char *)malloc(256); // Explicit cast
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 3), ".gb") == 0) && console == GAMEBOY)
        {
            game_list[i] = (char *)malloc(256); // Explicit cast
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 4), ".gbc") == 0) && console == GAMEBOY_COLOR)
        {
            game_list[i] = (char *)malloc(256); // Explicit cast
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 4), ".sms") == 0) && console == SMS)
        {
            game_list[i] = (char *)malloc(256); // Explicit cast
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
        else if ((strcmp(entry->d_name + (nameLength - 3), ".gg") == 0) && console == GG)
        {
            game_list[i] = (char *)malloc(256); // Explicit cast
            sprintf(game_list[i], "%s", entry->d_name);
            ESP_LOGI(TAG, "Found %s ", (char *)game_list[i]);
            // if(i>0) organize_list(game_list, i);
            i++;
        }
    }

    // Return the number of files
    return i;
}

uint8_t SD_CARD::sd_app_list(char *app_list[100], bool update)
{
    struct dirent *entry;

    DIR *dir = NULL;
    if (update)
        dir = opendir("/sdcard");
    else
        dir = opendir("/sdcard/apps");

    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to stat dir ");
        return 0;
    }

    // Only find .bin file on the apps folder
    uint8_t i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        size_t nameLength = strlen(entry->d_name);
        // app_list[i] = malloc(nameLength + 1);

        app_list[i] = (char *)malloc(256); // Explicit cast

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

void SD_CARD::sd_get_file(const char *path, void *data)
{
    const size_t BLOCK_SIZE = 512; // We're going to read file in chunk of 512 Bytes
    size_t r = 0;

    FILE *fd = fopen(path, "rb"); // Open the file in binary read mode

    if (fd == NULL)
    {
        ESP_LOGE(TAG, "Error opening: %s ", path);
    }
    while (true)
    {
        //__asm__("memw"); // Protect the write into the RAM memory
        size_t count = fread((uint8_t *)data + r, 1, BLOCK_SIZE, fd);
        // __asm__("memw");

        r += count;
        if (count < BLOCK_SIZE)
            break;
    }

    fclose(fd);
}

char *IRAM_ATTR SD_CARD::sd_get_file_flash(const char *path)
{
    const size_t BLOCK_SIZE = 4096 * 2;
    const void *map_ptr; // Correct type for the function

    spi_flash_mmap_handle_t map_handle;

    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "data_0");
    if (partition == NULL)
    {
        ESP_LOGE(TAG, "Partition NULL");
    }
    ESP_LOGI(TAG, "Partition label %s, offset 0x%x with size 0x%x\r\n", partition->label, partition->address, partition->size);

    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));

    size_t r = 0;

    FILE *fd = fopen(path, "rb"); // Open the file in binary read mode

    if (fd == NULL)
    {
        ESP_LOGE(TAG, "Error opening: %s ", path);
    }

    char *temp_buffer;
    temp_buffer = (char *)heap_caps_malloc(BLOCK_SIZE, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    esp_fill_random(temp_buffer, BLOCK_SIZE);

    while (true)
    {
        __asm__("memw"); // Protect the write into the RAM memory
        size_t count = fread(temp_buffer, 1, BLOCK_SIZE, fd);
        esp_partition_write(partition, r, temp_buffer, BLOCK_SIZE);
        __asm__("memw");

        r += count;
        if (count < BLOCK_SIZE)
            break;
    }

    fclose(fd);

    free(temp_buffer);
    // Return a pointer to the position of the saved file on the internal flash.

    ESP_ERROR_CHECK(esp_partition_mmap(partition, 0, 0x300000, SPI_FLASH_MMAP_DATA, (const void **)&map_ptr, &map_handle));

    return (char *)map_ptr; // Cast to char* before returning
}

bool SD_CARD::sd_mounted()
{
    return SD_mount;
}

bool SD_CARD::sd_sav_exist(char *file_name, uint8_t emulator)
{
    char *file_route = (char *)malloc(256);

    if (emulator == GAMEBOY)
        sprintf(file_route, "/sdcard/GameBoy/Save_Data/%s.sav", file_name);
    else if (emulator == GAMEBOY_COLOR)
        sprintf(file_route, "/sdcard/GameBoy_Color/Save_Data/%s.sav", file_name);
    else if (emulator == NES)
        sprintf(file_route, "/sdcard/NES/Save_Data/%s.sav", file_name);
    else if (emulator == SMS)
        sprintf(file_route, "/sdcard/Master_System/Save_Data/%s.sav", file_name);
    else if (emulator == GG)
        sprintf(file_route, "/sdcard/Game_Gear/Save_Data/%s.sav", file_name);

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
        sprintf(file_route, "/sdcard/GameBoy/Save_Data/%s.sav", file_name);
    else if (emulator == GAMEBOY_COLOR)
        sprintf(file_route, "/sdcard/GameBoy_Color/Save_Data/%s.sav", file_name);
    else if (emulator == NES)
        sprintf(file_route, "/sdcard/NES/Save_Data/%s.sav", file_name);
    else if (emulator == SMS)
        sprintf(file_route, "/sdcard/Master_System/Save_Data/%s.sav", file_name);
    else if (emulator == GG)
        sprintf(file_route, "/sdcard/Game_Gear/Save_Data/%s.sav", file_name);

    remove(file_route);
}

SD_CARD sd_card;