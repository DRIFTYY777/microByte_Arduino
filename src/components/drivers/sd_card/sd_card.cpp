#include "sd_card.h"

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include "formatSD.h"

#include <SD.h>  // Changed from SD_MMC.h
#include <SPI.h> // Added SPI library
#include <sys/dirent.h>
#include <esp_vfs_fat.h>

/*
    The mounting point of sd card using spi and SD library is "/sd".
    And the mounting point of sd card using SD_MMC library is "/sdcard".

    this MF wasted my previous time.
*/

static const char *TAG = "SD_CARD";

#define SPI_DMA_CHAN 2

SD_CARD_INFO sd_card_info;

void listDir(File dir, int depth)
{
    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            break;
        }

        for (int i = 0; i < depth; i++)
        {
            Serial.print("  ");
        }

        if (entry.isDirectory())
        {
            Serial.print("[DIR] ");
            Serial.println(entry.name());
            listDir(entry, depth + 1); // Recursive call for subdirectory
        }
        else
        {
            Serial.print("[FILE] ");
            Serial.print(entry.name());
            Serial.print("  SIZE: ");
            Serial.println(entry.size());
        }

        entry.close();
    }
}

bool SD_CARD::sd_init()
{
    // necessary of usb msc
    if (SD_mount)
    {
        SD.end();
        SD_mount = false;
        // 20 ms delay to ensure SD Card is properly unmounted
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    SPI.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI); // Initialize SPI bus (optional, default is HSPI)
    SPI.setFrequency(4000000);

    if (!SD.begin(SD_CS, SPI)) // Initialize SD card using SPI with CS pin
    {
        ESP_LOGE(TAG, "Card Mount Failed (SPI)");
        SD_mount = false;
        sd_card_info.card_mounted = 0;
        return false;
    }

    delay(100);
    // The SD library doesn't have a direct equivalent to SD_MMC.cardType()
    // You might need to rely on the success of SD.begin()
    // Assuming the card is mounted if SD.begin() succeeds
    ESP_LOGI(TAG, "SD Card Type: SPI"); // Indicate SPI usage

    const uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    const uint64_t usedSpace = cardSize - (SD.usedBytes() / (1024 * 1024));

    // ESP_LOGE(TAG, "SD Card Size: %lluMB", cardSize);

    // Update struct info
    sd_card_info.card_type = SDIO; // Or some other indication that it's using SPI
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

bool SD_CARD::sd_default()
{
    // Check if the SD card is mounted
    if (!SD_mount)
    {
        ESP_LOGE(TAG, "SD Card not mounted");
        return false;
    }
    // Format the SD card
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

bool SD_CARD::readRAW_(uint8_t *buffer, uint32_t sector)
{
    return SD.readRAW(buffer, sector);
}

bool SD_CARD::writeRAW_(uint8_t *buffer, uint32_t sector)
{
    return SD.writeRAW(buffer, sector);
}

uint64_t SD_CARD::sd_get_size()
{
    return SD.cardSize();
}

uint8_t SD_CARD::sd_app_list(char *app_list[100], bool update)
{
    File root;
    if (update)
        root = SD.open("/Firmware");
    else
        root = SD.open("/External Apps"); //

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to open directory");
        return 0;
    }

    if (!root.isDirectory())
    {
        ESP_LOGE(TAG, "Not a directory: %s", update ? "/Firmware" : "/External Apps");
        return 0;
    }

    uint8_t i = 0;
    File entry;
    while ((entry = root.openNextFile()))
    {
        if (!entry.isDirectory())
        {
            const char *fileName = entry.name();
            ESP_LOGI(TAG, "Checking file: %s", fileName);

            if (String(fileName).endsWith(".bin"))
            {
                size_t nameLength = strlen(fileName);
                app_list[i] = static_cast<char *>(malloc(nameLength + 1));
                if (app_list[i])
                {
                    strcpy(app_list[i], fileName);
                    ESP_LOGI(TAG, "Found bin: %s", app_list[i]);
                    i++;
                }
                else
                {
                    ESP_LOGE(TAG, "Memory allocation failed for %s", fileName);
                }
            }
        }
        entry.close();
    }

    root.close();
    return i;
}

size_t SD_CARD::sd_file_size(const char *path)
{
    File file = SD.open(path);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file for size check");
        return 0;
    }
    size_t actual_size = file.size();
    ESP_LOGI(TAG, "Size: %i bytes", actual_size);
    file.close();
    return actual_size;
}

uint8_t SD_CARD::sd_game_list(char *game_list[100], uint8_t console)
{
    const char *dirPath;
    const char *extension;

    switch (console)
    {
    case NES:
        dirPath = "/Emulator/NES";
        extension = ".nes";
        break;
    case GAMEBOY:
        dirPath = "/Emulator/GameBoy";
        extension = ".gb";
        break;
    case GAMEBOY_COLOR:
        dirPath = "/Emulator/GameBoy_Color";
        extension = ".gbc";
        break;
    case SNES:
        dirPath = "/Emulator/SNES";
        extension = ".smc";
        break;
    case SMS:
        dirPath = "/Emulator/Master_System";
        extension = ".sms";
        break;
    case GG:
        dirPath = "/Emulator/Game_Gear";
        extension = ".gg";
        break;
    default:
        ESP_LOGE(TAG, "Invalid console type: 0x%02x", console);
        return 0;
    }

    File root = SD.open(dirPath);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", dirPath);
        return 0;
    }

    if (!root.isDirectory())
    {
        ESP_LOGE(TAG, "%s is not a directory", dirPath);
        return 0;
    }

    uint8_t i = 0;
    File entry;
    while ((entry = root.openNextFile()))
    {
        if (!entry.isDirectory())
        {
            String fileName = entry.name();
            ESP_LOGI(TAG, "Checking file: %s", fileName.c_str());
            if (fileName.endsWith(extension))
            {
                size_t nameLength = fileName.length();
                game_list[i] = static_cast<char *>(malloc(nameLength + 1));
                if (game_list[i])
                {
                    strcpy(game_list[i], fileName.c_str());
                    ESP_LOGI(TAG, "Found game: %s", game_list[i]);
                    i++;
                }
                else
                {
                    ESP_LOGE(TAG, "Memory allocation failed");
                }
            }
        }
        entry.close();
    }
    root.close();

    ESP_LOGI(TAG, "Total games found in %s: %d", dirPath, i);
    return i;
}

bool SD_CARD::sd_sav_exist(char *file_name, uint8_t emulator)
{
    char file_route[256];
    const char *saveDir;

    switch (emulator)
    {
    case GAMEBOY:
        saveDir = "/Emulator/GameBoy/Save_Data/";
        break;
    case GAMEBOY_COLOR:
        saveDir = "/Emulator/GameBoy_Color/Save_Data/";
        break;
    case NES:
        saveDir = "/Emulator/NES/Save_Data/";
        break;
    case SMS:
        saveDir = "/Emulator/Master_System/Save_Data/";
        break;
    case GG:
        saveDir = "/Emulator/Game_Gear/Save_Data/";
        break;
    default:
        ESP_LOGE(TAG, "Invalid emulator type for save exist check");
        return false;
    }

    sprintf(file_route, "%s%s.sav", saveDir, file_name);

    File file = SD.open(file_route);
    bool exists = file;
    if (exists)
    {
        file.close();
    }
    return exists;
}

void SD_CARD::sd_sav_remove(char *file_name, uint8_t emulator)
{
    char file_route[256];
    const char *saveDir;

    switch (emulator)
    {
    case GAMEBOY:
        saveDir = "/Emulator/GameBoy/Save_Data/";
        break;
    case GAMEBOY_COLOR:
        saveDir = "/Emulator/GameBoy_Color/Save_Data/";
        break;
    case NES:
        saveDir = "/Emulator/NES/Save_Data/";
        break;
    case SMS:
        saveDir = "/Emulator/Master_System/Save_Data/";
        break;
    case GG:
        saveDir = "/Emulator/Game_Gear/Save_Data/";
        break;
    default:
        ESP_LOGE(TAG, "Invalid emulator type for save removal");
        return;
    }

    sprintf(file_route, "%s%s.sav", saveDir, file_name);

    if (SD.remove(file_route))
    {
        ESP_LOGI(TAG, "Save file removed: %s", file_route);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to remove save file: %s", file_route);
    }
}

void SD_CARD::emulator_dir()
{
    // Check if the emulator folders exist and create them if they don't
    if (!SD.exists("/Emulator"))
    {
        ESP_LOGI(TAG, "No Emulator folder found, creating it");
        SD.mkdir("/Emulator");
    }
    // NES
    if (!SD.exists("/Emulator/NES"))
    {
        ESP_LOGI(TAG, "No NES folder found, creating it");
        SD.mkdir("/Emulator/NES");
        SD.mkdir("/Emulator/NES/Save_Data");
    }
    // GameBoy Color
    if (!SD.exists("/Emulator/GameBoy_Color"))
    {
        ESP_LOGI(TAG, "No GameBoy Color folder found, creating it");
        SD.mkdir("/Emulator/GameBoy_Color");
        SD.mkdir("/Emulator/GameBoy_Color/Save_Data");
    }
    // GameBoy
    if (!SD.exists("/Emulator/GameBoy"))
    {
        ESP_LOGI(TAG, "No GameBoy folder found, creating it");
        SD.mkdir("/Emulator/GameBoy");
        SD.mkdir("/Emulator/GameBoy/Save_Data");
    }
    // SNES
    if (!SD.exists("/Emulator/SNES"))
    {
        ESP_LOGI(TAG, "No SNES folder found, creating it");
        SD.mkdir("/Emulator/SNES");
        SD.mkdir("/Emulator/SNES/Save_Data");
    }
    // Game Gear
    if (!SD.exists("/Emulator/Game_Gear"))
    {
        ESP_LOGI(TAG, "No Game_Gear folder found, creating it");
        SD.mkdir("/Emulator/Game_Gear");
        SD.mkdir("/Emulator/Game_Gear/Save_Data");
    }
    // Master System
    if (!SD.exists("/Emulator/Master_System"))
    {
        ESP_LOGI(TAG, "No Master_System folder found, creating it");
        SD.mkdir("/Emulator/Master_System");
        SD.mkdir("/Emulator/Master_System/Save_Data");
    }
}

void SD_CARD::system_dir()
{
    // Check if the system folders exist and create them if they don't
    if (!SD.exists("/Internal Apps"))
    {
        ESP_LOGI(TAG, "No Apps folder found, creating it");
        SD.mkdir("/Internal Apps");
    }
    // External Apps
    if (!SD.exists("/External Apps"))
    {
        ESP_LOGI(TAG, "No External_Apps folder found, creating it");
        SD.mkdir("/External Apps");
    }
    // Firmware
    if (!SD.exists("/Firmware"))
    {
        ESP_LOGI(TAG, "No Firmware folder found, creating it");
        SD.mkdir("/Firmware");
    }
}

SD_CARD sd_card;
