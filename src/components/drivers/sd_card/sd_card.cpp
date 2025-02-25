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
#include <SD_MMC.h>

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
    delay(1000);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, 1);

    SD_MMC.setPins(VSPI_CLK, VSPI_MOSI, VSPI_MISO);
    if (!SD_MMC.begin("/sdcard", true))
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    delay(500);
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