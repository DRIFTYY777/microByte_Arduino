#include "sound.h"
#include <Arduino.h>

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2s.h"
#include "driver/rtc_io.h"

#include <components/system_config/system_manager.h>
#include <components/system_config/system_config.h>

static const char *TAG = "SOUND_DRIVER";

bool SOUND::audio_init(uint32_t sample_rate)
{
    ESP_LOGI(TAG, "Audio configuration init.");

    ESP_LOGI(TAG, "Audio Sample Rate: %i", sample_rate);

    i2s_config_t i2s_config;
    
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    i2s_config.sample_rate = sample_rate;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    i2s_config.dma_buf_count = 6;
    i2s_config.dma_buf_len = 512;

    if (i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL) != ESP_OK)
    {
        ESP_LOGE(TAG, "I2S driver error install error");
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_DATA_O,
        .data_in_num = -1 // Not used
    };

    if (i2s_set_pin(I2S_NUM, &pin_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "I2S set pin error.");
        return false;
    }

    volume_level = (float)audio_volume_get() / 100.0f; // Get the saved volume value and transforn into float
    return true;
}

void SOUND::audio_submit(short *stereoAudioBuffer, uint32_t frameCount)
{
    uint32_t audio_length = frameCount * 2 * sizeof(int16_t);
    size_t count;

    // Normalize the size of the sample to avoid size bigger than +- 32767
    for (short i = 0; i < frameCount * 2; ++i)
    {
        int sample = stereoAudioBuffer[i] * volume_level; // Set the volumen level to the sample

        if (sample > 32767)
            sample = 32767;
        else if (sample < -32767)
            sample = -32767;

        stereoAudioBuffer[i] = (short)sample;
    }

    i2s_write(I2S_NUM, (const char *)stereoAudioBuffer, audio_length, &count, portMAX_DELAY);

    if (count != audio_length)
    {
        ESP_LOGE(TAG, "I2S Write error:\n Send count: %d\n Audio_Length: %d", count, audio_length);
        abort();
    }
}

void SOUND::audio_terminate()
{
    i2s_zero_dma_buffer(I2S_NUM); // Clean the DMA buffer
    i2s_stop(I2S_NUM);
    i2s_start(I2S_NUM);
}

uint8_t SOUND::audio_volume_get()
{
    uint8_t level = sys_manager.system_get_config(SYS_VOLUME);
    ESP_LOGI(TAG, "Volumen level: %i", level);
    return level;
}

void SOUND::audio_volume_set(float level)
{
    if (level >= 0 || level <= 100)
        volume_level = level / 100.0f;
    ESP_LOGI(TAG, "Volumen level set: %i", (uint8_t)volume_level * 100);
    sys_manager.system_save_config(SYS_VOLUME, level);
}

SOUND sound;