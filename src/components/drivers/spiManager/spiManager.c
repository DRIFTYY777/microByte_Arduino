// spiManager.c
#include "spiManager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "SPI_MANAGER";

// Structure to track SPI bus state
typedef struct {
    bool initialized;
    int device_count;
    SemaphoreHandle_t mutex;
} spi_bus_state_t;

static spi_bus_state_t spi_buses[SOC_SPI_PERIPH_NUM] = {0};

bool spi_bus_manager_init(const spi_host_device_t host, const int mosi, const int miso, const int sclk, size_t max_transfer_sz)
{
    // Validate host parameter
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        ESP_LOGE(TAG, "Invalid SPI host: %d", host);
        return false;
    }
    // Check if already initialized
    if (spi_buses[host].initialized) {
        ESP_LOGI(TAG, "SPI%d already initialized", host);
        return true;
    }
    // Create mutex for this bus if not already created
    if (spi_buses[host].mutex == NULL) {
        spi_buses[host].mutex = xSemaphoreCreateMutex();
        if (spi_buses[host].mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create mutex for SPI%d", host);
            return false;
        }
    }
    // Take mutex
    if (xSemaphoreTake(spi_buses[host].mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take mutex for SPI%d", host);
        return false;
    }
    // Set the default max transfer size if not specified
    if (max_transfer_sz == 0) {
        max_transfer_sz = 4092; // Default safe size
    }
    // Configure SPI bus
    const spi_bus_config_t buscfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = max_transfer_sz,
        .flags = SPICOMMON_BUSFLAG_MASTER
        // .flags = 0
    };

    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI%d: %s", host, esp_err_to_name(ret));
        xSemaphoreGive(spi_buses[host].mutex);
        return false;
    }

    // Mark as initialized
    spi_buses[host].initialized = true;
    spi_buses[host].device_count = 0;

    xSemaphoreGive(spi_buses[host].mutex);

    ESP_LOGI(TAG, "SPI%d initialized successfully", host);
    ESP_LOGI(TAG, "MOSI: %d, MISO: %d, CLK: %d, Max Transfer: %zu bytes",
             mosi, miso, sclk, max_transfer_sz);

    return true;
}

esp_err_t spi_bus_manager_add_device2(const spi_host_device_t host, const int cs_pin, spi_device_interface_config_t *devcfg, spi_device_handle_t *handle)
{
    // Validate parameters
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        ESP_LOGE(TAG, "Invalid SPI host: %d", host);
        return ESP_ERR_INVALID_ARG;
    }

    if (devcfg == NULL || handle == NULL) {
        ESP_LOGE(TAG, "Invalid device config or handle pointer");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if SPI bus is initialized
    if (!spi_buses[host].initialized) {
        ESP_LOGE(TAG, "SPI%d not initialized. Call spi_bus_manager_init first.", host);
        return ESP_ERR_INVALID_STATE;
    }

    // Take mutex
    if (xSemaphoreTake(spi_buses[host].mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take mutex for SPI%d", host);
        return ESP_ERR_TIMEOUT;
    }

    // Set CS pin if provided and not already set in devcfg
    if (cs_pin >= 0 && devcfg->spics_io_num == -1) {
        devcfg->spics_io_num = cs_pin;
    }

    // Set default queue size if not specified
    if (devcfg->queue_size == 0) {
        devcfg->queue_size = 7;
    }

    // Add a device to SPI bus
    esp_err_t ret = spi_bus_add_device(host, devcfg, handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device to SPI%d: %s", host, esp_err_to_name(ret));
        xSemaphoreGive(spi_buses[host].mutex);
        return ret;
    }

    // Increment device count
    spi_buses[host].device_count++;

    xSemaphoreGive(spi_buses[host].mutex);

    ESP_LOGI(TAG, "Device added to SPI%d successfully (CS: %d, Speed: %lu Hz)", 
             host, devcfg->spics_io_num, devcfg->clock_speed_hz);

    return ESP_OK;
}

esp_err_t spi_bus_manager_add_device(spi_host_device_t host, spi_device_interface_config_t *devcfg,
    spi_device_handle_t*handle) {
    return spi_bus_add_device(host, devcfg, handle);
}

esp_err_t spi_bus_manager_remove_device(spi_device_handle_t handle)
{
    if (handle == NULL) {
        ESP_LOGE(TAG, "Invalid device handle");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = spi_bus_remove_device(handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Device removed from SPI bus successfully");
        // Note: We can't easily determine which host this was, so we don't decrement device_count here
        // This is a limitation of the ESP-IDF SPI API
    } else {
        ESP_LOGE(TAG, "Failed to remove device from SPI bus: %s", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t spi_bus_manager_deinit(const spi_host_device_t host)
{
    // Validate host parameter
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        ESP_LOGE(TAG, "Invalid SPI host: %d", host);
        return ESP_ERR_INVALID_ARG;
    }

    // Check if bus is initialized
    if (!spi_buses[host].initialized) {
        ESP_LOGW(TAG, "SPI%d not initialized", host);
        return ESP_OK;
    }

    // Take mutex
    if (xSemaphoreTake(spi_buses[host].mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take mutex for SPI%d", host);
        return ESP_ERR_TIMEOUT;
    }

    // Check if there are still devices attached
    if (spi_buses[host].device_count > 0) {
        ESP_LOGW(TAG, "SPI%d still has %d devices attached", host, spi_buses[host].device_count);
    }

    // Free the SPI bus
    esp_err_t ret = spi_bus_free(host);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to free SPI%d: %s", host, esp_err_to_name(ret));
        xSemaphoreGive(spi_buses[host].mutex);
        return ret;
    }

    // Mark as uninitialized
    spi_buses[host].initialized = false;
    spi_buses[host].device_count = 0;

    xSemaphoreGive(spi_buses[host].mutex);

    // Clean up mutex
    vSemaphoreDelete(spi_buses[host].mutex);
    spi_buses[host].mutex = NULL;

    ESP_LOGI(TAG, "SPI%d deinitialized successfully", host);

    return ESP_OK;
}

bool spi_bus_manager_is_initialized(const spi_host_device_t host)
{
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        return false;
    }
    return spi_buses[host].initialized;
}

int spi_bus_manager_get_device_count(spi_host_device_t host)
{
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        return -1;
    }
    return spi_buses[host].device_count;
}

void set_max_transfer_size(spi_host_device_t host, size_t max_transfer_sz) {
    // set the maximum transfer size for the specified SPI host
    if (host >= SOC_SPI_PERIPH_NUM || host < 0) {
        ESP_LOGE(TAG, "Invalid SPI host: %d", host);
        return;
    }
    if (!spi_buses[host].initialized) {
        ESP_LOGE(TAG, "SPI%d not initialized. Call spi_bus_manager_init first.", host);
        return;
    }
    if (max_transfer_sz == 0) {
        max_transfer_sz = 4092; // Default safe size
    }
    if (xSemaphoreTake(spi_buses[host].mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take mutex for SPI%d", host);
        return;
    }
    spi_bus_config_t buscfg = {
        .max_transfer_sz = max_transfer_sz,
    };
}

esp_err_t spi_manager_transmit(spi_device_handle_t handle, const uint8_t *tx_data, size_t length)
{
    if (handle == NULL || tx_data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    spi_transaction_t trans = {
        .length = length * 8,  // Length in bits
        .tx_buffer = tx_data,
        .rx_buffer = NULL
    };

    return spi_device_transmit(handle, &trans);
}

esp_err_t spi_manager_receive(const spi_device_handle_t handle, uint8_t *rx_data, const size_t length)
{
    if (handle == NULL || rx_data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    spi_transaction_t trans = {
        .length = length * 8,  // Length in bits
        .tx_buffer = NULL,
        .rx_buffer = rx_data
    };

    return spi_device_transmit(handle, &trans);
}

esp_err_t spi_manager_transmit_receive(const spi_device_handle_t handle, const uint8_t *tx_data, uint8_t *rx_data, const size_t length)
{
    if (handle == NULL || tx_data == NULL || rx_data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    spi_transaction_t trans = {
        .length = length * 8,  // Length in bits
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };

    return spi_device_transmit(handle, &trans);
}