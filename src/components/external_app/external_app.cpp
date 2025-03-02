#include "external_app.h"

#include "stdio.h"
#include "stdlib.h"

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"

static const char *TAG = "EXTERNAL_APP";

void EXTERNAL_APP::external_app_init(const char *app_name)
{
    ESP_LOGI(TAG, "Starting external App installation");
    esp_err_t err;

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL)
    {
        ESP_LOGE(TAG, "Failed to get OTA partition");
        return;
    }

    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return;
    }

    FILE *fd = NULL;
    char name_aux[256];
    sprintf(name_aux, "/sdcard/External Apps/%s", app_name);

    fd = fopen(name_aux, "rb");
    if (fd == NULL)
    {
        ESP_LOGE(TAG, "Opening error with: %s", name_aux);
        return;
    }

    fd = fopen(name_aux, "rb");
    if (fd == NULL)
    {
        ESP_LOGE(TAG, "Opening error with: %s", name_aux);
        return;
    }

    static char write_buffer[512 + 1] = {0};
    int binary_file_length = 0;
    while (1)
    {
        size_t data_read = fread(write_buffer, 1, 512, fd);
        err = esp_ota_write(update_handle, (const void *)write_buffer, data_read);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "OTA write error");
        }
        binary_file_length += data_read;
        ESP_LOGI(TAG, "Written image length %d", binary_file_length);
        if (data_read < 512)
            break;
    }

    ESP_LOGI(TAG, "OTA Write correct");

    err = esp_ota_end(update_handle);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED)
        {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
    }
}

EXTERNAL_APP external_app;
