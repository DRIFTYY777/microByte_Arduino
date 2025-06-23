//
// Created by dhima on 18-06-2025.
//

#include "usbStorage.h"

#include <esp_log.h>
#include <components/drivers/sd_card/sd_card.h>


static const char *TAG = "USBSTORAGE";

bool USBSTORAGE::onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
   ESP_LOGI(TAG, "Power condition: %d, start: %d, load_eject: %d", power_condition, start, load_eject);
   return true; // Always return true for simplicity
}

int32_t USBSTORAGE::onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    uint8_t *buf = (uint8_t *)buffer;
    for (uint32_t i = 0; i < bufsize; i += 512)
    {
        // sd_card.readRAW_ is bassically SD.readRAW just passing the buffer and sector

        if (!sd_card.readRAW_(buf + i, lba + (i / 512)))
        {
            ESP_LOGE(TAG, "Read error at LBA %lu", lba + (i / 512));
            return -1;
        }
    }
    return bufsize;
}

int32_t USBSTORAGE::onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    for (uint32_t i = 0; i < bufsize; i += 512)
    {
        if (!sd_card.writeRAW_(buffer + i, lba + (i / 512)))
        {
            // Serial.printf("Write error at LBA %lu\n", lba + (i / 512));
            ESP_LOGE(TAG, "Write error at LBA %lu", lba + (i / 512));
            return -1;
        }
    }
    return bufsize;
}

bool USBSTORAGE::init()
{
    sd_card.sd_init();

    // Initialize USB if not already done
    //USB.begin();

    // Configure MSC
    MSC.vendorID("ESP32");
    MSC.productID("S3 SD");
    MSC.productRevision("1.0");
    MSC.onStartStop(onStartStop);
    MSC.onRead(onRead);
    MSC.onWrite(onWrite);
    MSC.mediaPresent(true);


    uint32_t sectorCount = sd_card.sd_get_size() / 512; // Get total size in sectors (512 bytes each)

    // get sd card size
    //uint32_t sectorCount = sd_card.sd_card_info.card_size * 1024 * 1024 / 512; // Convert MB to sectors (512 bytes each)


    // Start MSC with sector information
    if (MSC.begin(sectorCount, 512))
    {
        usbModeActive = true;
        ESP_LOGI(TAG, "USB Mass Storage started");
        return true;
    }
    ESP_LOGI(TAG, "Failed to start USB Mass Storage");
    return false;
}

bool USBSTORAGE::deinit()
{
    if (usbModeActive)
    {
        ESP_LOGI(TAG, "Stopping USB Mass Storage...");
        MSC.end();
        //vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for USB to stop

        usbModeActive = false;
    }
    sd_card.sd_init();
}

USBSTORAGE usbStorage;
