#include "battery.h"
#include <Arduino.h>

static const char *TAG = "Battery";

struct BATTERY_STATUS battery_status;

void BATTERY::battery_init(void)
{
    // Configure ADC width (12 bits) and channel attenuation
    analogReadResolution(12); // ESP32 ADC width 12-bit (values between 0-4095)
    // No need to manually configure attenuation since the Arduino framework handles this for you
    // For specific channel, you can set attenuation using: analogSetAttenuation(ADC_0, ADC_1, ADC_2, ADC_3, etc.)

    // Initialize the ADC channel (assuming you're using a standard GPIO)
    xTaskCreate(batteryTaskWrapper, "Battery management", 2048, this, 5, NULL); // Pass `this` pointer
}
void BATTERY::battery_game_mode(bool game_mode)
{
    game_mode_active = game_mode;
}

uint8_t BATTERY::battery_get_percentage()
{
    return battery_status.percentage;
}

void BATTERY::batteryTaskWrapper(void *arg)
{
    BATTERY *battery = (BATTERY *)arg; // Cast the argument back to an object
    battery->batteryTask(arg);         // Call the instance method
}

// Battery management task
void BATTERY::batteryTask(void *arg)
{
    while (1)
    {
        uint32_t adc_reading = 0;

        // Perform a multisampling reading of the ADC1 Channel 0
        for (int i = 0; i < 128; i++)
        {
            adc_reading += analogRead(channel); // Use Arduino analogRead instead of adc1_get_raw
        }

        adc_reading /= 128;                                    // Average reading
        uint32_t voltage = map(adc_reading, 0, 4095, 0, 3300); // Map ADC value to voltage (0-3.3V)

        voltage += 1310; // Add offset from voltage divider (e.g., 1000 mV)
        battery_status.voltage = voltage;

        // Calculate the battery percentage based on voltage (non-linear approximation)
        float percentage_aux = (voltage - 3570.6) / 5.2128;

        // If voltage is below 3576mV, assume 1% battery
        if (battery_status.voltage < 3576)
            percentage_aux = 1.00;

        battery_status.percentage = (uint8_t)percentage_aux;

        // Ensure percentage doesn't exceed 100%
        if (battery_status.percentage > 100)
            battery_status.percentage = 100;

        // If we're not in game mode, send battery status to the queue
        if (!game_mode_active)
        {
            if (xQueueSend(batteryQueue, &battery_status, (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, "Battery queue send fail");
            }
        }

        // Check if battery percentage is below 10% and trigger an alert
        if (battery_status.percentage <= 10 && battery_alert != true)
        {
            struct SYSTEM_MODE management;
            management.mode = MODE_BATTERY_ALERT;

            if (xQueueSend(modeQueue, &management, (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, "Mode queue send fail, battery alert!");
            }

            battery_alert = true; // Set the alert flag to true
        }
        else if (battery_status.percentage > 10 && battery_alert == true)
        {
            battery_alert = false; // Reset the alert flag
        }

        vTaskDelay(1000 / portTICK_RATE_MS); // Delay task for 1 second
    }
}


BATTERY battery;