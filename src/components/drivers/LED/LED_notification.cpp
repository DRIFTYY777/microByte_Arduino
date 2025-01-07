#include "LED_notification.h"
#include <components/system_config/system_config.h>

#include "LED_notification.h"
#include <driver/ledc.h>

static const char *TAG = "LED notifications";

// FreeRTOS queue for LED commands
QueueHandle_t LEDQueue;

// LEDC channel configuration
ledc_channel_config_t ledc;

void LED_NOTIFICATION::LED_init()
{
    // Initialize the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // Use LOW_SPEED_MODE for Arduino
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    // Configure the LEDC channel
    ledc.channel = LEDC_CHANNEL_0;
    ledc.duty = 0;
    ledc.gpio_num = LED_PIN;
    ledc.speed_mode = LEDC_LOW_SPEED_MODE; // Use LOW_SPEED_MODE for Arduino
    ledc.hpoint = 0;
    ledc.timer_sel = LEDC_TIMER_0;

    ledc_channel_config(&ledc);

    // Install fade function
    ledc_fade_func_install(0);

    // Create FreeRTOS queue
    LEDQueue = xQueueCreate(5, sizeof(uint8_t));

    // Start the LED task
    xTaskCreate(LED_task, "LED Task", 1024, NULL, 1, NULL);
}

// Function to set LED mode
void LED_NOTIFICATION::LED_mode(uint8_t mode)
{
    if (xQueueSend(LEDQueue, &mode, (TickType_t)10) != pdPASS)
        ESP_LOGE(TAG, "Queue send failed");
}

// LED task function
void LED_NOTIFICATION::LED_task(void *arg)
{
    uint8_t mode;

    while (1)
    {
        if (xQueueReceive(LEDQueue, &mode, portMAX_DELAY))
        {
            switch (mode)
            {
            case LED_TURN_ON:
                ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                break;

            case LED_TURN_OFF:
                ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 100);
                ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                break;

            case LED_BLINK_HS:
                for (;;)
                {
                    if (xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(250 / portTICK_PERIOD_MS);
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(250 / portTICK_PERIOD_MS);
                }
                break;

            case LED_BLINK_LS:
                for (;;)
                {
                    if (xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                break;

            case LED_FADE_ON:
                ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 1000);
                ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                break;

            case LED_FADE_OFF:
                ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 1000);
                ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                break;

            case LED_LOAD_ANI:
                for (;;)
                {
                    if (xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    int random_time = random(50, 250);
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(random_time / portTICK_PERIOD_MS);
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(random_time / portTICK_PERIOD_MS);
                }
                break;

            default:
                break;
            }
        }
    }
}

LED_NOTIFICATION led_notification;