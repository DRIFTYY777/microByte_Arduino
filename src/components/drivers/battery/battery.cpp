#include "battery.h"
#include <Arduino.h>
#include <esp_adc_cal.h>

static const char *TAG = "Battery";

bool game_mode_active = false;
bool battery_alert;

struct BATTERY_STATUS battery_status;
struct SYSTEM_MODE management;

esp_adc_cal_characteristics_t *adc_chars;
adc_channel_t channel = ADC_CHANNEL_0;
adc_atten_t atten = ADC_ATTEN_DB_11;
adc_unit_t unit = ADC_UNIT_1;

BATTERY battery;

void BATTERY::battery_init()
{
    ESP_LOGI(TAG, "Battery initialization");
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    xTaskCreatePinnedToCore(batteryTask, "Battery Task", 2044, NULL, 5, NULL, 0);
}

void BATTERY::battery_game_mode(bool game_mode)
{
    game_mode_active = game_mode;
}

uint8_t BATTERY::battery_get_percentage()
{
    return battery_status.percentage;
}

void BATTERY::batteryTask(void *arg)
{
    while (1)
    {
        uint32_t adc_reading = 0;

        // Perform a multisampling reading of the ADC1 Channel 0
        for (int i = 0; i < 128; i++)
        {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }

        adc_reading /= 128;
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        voltage += 1310; // Add 1000 mV to offset the value obtained from the voltage divider
        battery_status.voltage = voltage;

        // To avoid lost of precision, first we calculate the percentage in float.
        float percentage_aux = (voltage - 3570.6) / 5.2128;
        // Explanation of this weird thing:
        //  The curve from this point (3576 mV) is not lineal so, to avoid split the curve and
        //  add a new polinomic function I've follow the ugly/lazy path, it's 1% all the time.
        if (battery_status.voltage < 3576)
            percentage_aux = 1.00;
        battery_status.percentage = (uint8_t)percentage_aux;

        // When you're charging the battery, the percentage can higher than 100%
        if (battery_status.percentage > 100)
            battery_status.percentage = 100;

        // If we're playing, we don't need the battery info. We only need the an alert if we're on very low percentage
        // The alert was done, it's not necessay to do it again
        if (game_mode_active == true)
        {
            ESP_LOGI(TAG, "Game mode active");
            if (battery_status.percentage < 10 && battery_alert == false)
            {
                battery_alert = true;
                management.mode = MODE_BATTERY_ALERT;
                xQueueSend(modeQueue, &management, 0);
            }
            else if (battery_status.percentage > 10 && battery_alert == true)
                battery_alert = false;
        }

        else if (battery_status.percentage > 10 && battery_alert == true)
            battery_alert = false;

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
