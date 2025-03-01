#include "formatSD.h"
#include <esp32-hal-log.h>
#include <ff.h>
#include <vfs_fat_internal.h>

static const char *TAG = "formatSD";

esp_err_t format_sdcard()
{
    char drv[3] = {'0', ':', 0};
    const size_t workbuf_size = 4096;
    void *workbuf = NULL;
    esp_err_t err = ESP_OK;
    ESP_LOGW(TAG, "Formatting the SD card");

    size_t allocation_unit_size = 16 * 1024;
    int sector_size_default = 512;

    workbuf = ff_memalloc(workbuf_size);
    if (workbuf == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        return ESP_ERR_NO_MEM;
    }

    size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(
        sector_size_default,
        allocation_unit_size);

    if (alloc_unit_size == 0)
    {
        ESP_LOGE(TAG, "Invalid allocation unit size");
        free(workbuf);
        return ESP_FAIL;
    }

#if (ESP_IDF_VERSION_MAJOR < 5)
    FRESULT res = f_mkfs(drv, FM_ANY, alloc_unit_size, workbuf, workbuf_size);
#else
    const MKFS_PARM opt = {(BYTE)FM_ANY, 0, 0, 0, alloc_unit_size};
    FRESULT res = f_mkfs(drv, &opt, workbuf, workbuf_size);
#endif /* ESP_IDF_VERSION_MAJOR */

    free(workbuf);

    if (res != FR_OK)
    {
        ESP_LOGE(TAG, "f_mkfs failed (%d)", res);
        return ESP_FAIL; // Ensure error is returned
    }

    ESP_LOGI(TAG, "Successfully formatted the SD card");
    return ESP_OK;
}
