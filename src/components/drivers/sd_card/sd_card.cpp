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

bool SD_CARD::sd_init()
{
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
    return true;
}

SD_CARD sd_card;