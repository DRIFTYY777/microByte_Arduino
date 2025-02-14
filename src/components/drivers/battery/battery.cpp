#include "battery.h"
#include <Arduino.h>
#include <esp_adc_cal.h>

#include <components/drivers/battery/MAX17048.h>

static const char *TAG = "Battery";

bool game_mode_active = false;
bool battery_alert;

struct BATTERY_STATUS battery_status;
struct SYSTEM_MODE management;

BATTERY battery;

void BATTERY::battery_init()
{

    if (!max17048.MAX17048_init())
    {
        ESP_LOGE(TAG, "MAX17048 initialization failed");
        return;
    }
    ESP_LOGE(TAG, "MAX17048 initialization success");

    xTaskCreatePinnedToCore(batteryTask, "Battery Task", 2048, NULL, 5, NULL, 0);
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
        uint32_t voltage = max17048.MAX17048_readVoltage();
        battery_status.voltage = voltage;
        float percentage_aux = (voltage - 3570.6) / 5.2128;
        if (battery_status.voltage < 3576)
            percentage_aux = 1.00;
        battery_status.percentage = (uint8_t)percentage_aux;
        if (battery_status.percentage > 100)
            battery_status.percentage = 100;

        if (!game_mode_active)
        {
            // If we're playing, we don't need the battery info. We only need the an alert if we're on very low percentage
            if (xQueueSend(batteryQueue, &battery_status, (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, "Battery queue send fail");
            }
        }
        if (battery_status.percentage <= 10 && battery_alert != true)
        {
            // Send battery alert if the level is below 10%
            struct SYSTEM_MODE management;
            management.mode = MODE_BATTERY_ALERT;

            if (xQueueSend(modeQueue, &management, (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, "Mode queue send fail, battery alert!");
            }

            battery_alert = true; // The alert was done, it's not necessay to do it again
        }
        else if (battery_status.percentage > 10 && battery_alert == true)
            battery_alert = false;

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
