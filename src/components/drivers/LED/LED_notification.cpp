#include "LED_notification.h"
#include <driver/ledc.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>
#include <components/core/processManager.h>

static const char *TAG = "LED notifications";

// FreeRTOS queue for LED commands
QueueHandle_t LEDQueue;

// LEDC channel configuration
ledc_channel_config_t ledc;

// Process ID for LED task
uint32_t led_process_id = 0;

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

    // Create LED process using process manager instead of xTaskCreate
    led_process_id = process_manager.create_process(
        "LED_Task",              // Process name
        LED_task,                // Task function
        1024,                    // Stack size
        NULL,                    // Parameters
        PROCESS_PRIORITY_LOW,    // Priority
        tskNO_AFFINITY          // Core affinity
    );

    if (led_process_id == 0) {
        ESP_LOGE(TAG, "Failed to create LED process");
        return;
    }

    ESP_LOGI(TAG, "LED process created with ID: %lu", led_process_id);

    // Check if the brightness is set to 0, if so set it to 50
    if (sys_manager.system_get_config(SYS_LED) == 0)
    {
        sys_manager.system_save_config(SYS_LED, LED_FADE_ON);
    }
    else
    {
        // Set the last saved value
        LED_mode(sys_manager.system_get_config(SYS_LED));
    }
}

// Function to set LED mode
void LED_NOTIFICATION::LED_mode(uint8_t mode)
{
    // Save the LED mode to NVS
    sys_manager.system_save_config(SYS_LED, mode);
    // Send the LED mode to the queue
    if (xQueueSend(LEDQueue, &mode, (TickType_t)10) != pdPASS)
        ESP_LOGE(TAG, "Queue send failed");
}

void LED_NOTIFICATION::LED_set(uint8_t state)
{
    // Set the LED state on or off
    if (state)
        ledc_set_duty(ledc.speed_mode, ledc.channel, 8191);
    else
        ledc_set_duty(ledc.speed_mode, ledc.channel, 0);
    ledc_update_duty(ledc.speed_mode, ledc.channel);
}

// LED task function - now managed by ProcessManager
void LED_NOTIFICATION::LED_task(void *arg)
{
    uint8_t mode;

    ESP_LOGI(TAG, "LED task started and managed by ProcessManager");

    while (1)
    {
        // Check if process is still alive and should continue running
        if (!process_manager.is_process_alive(led_process_id)) {
            ESP_LOGW(TAG, "LED process is no longer alive, terminating task");
            break;
        }

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
                    // Check if process should continue and no new mode in queue
                    if (!process_manager.is_process_alive(led_process_id) || 
                        xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(250 / portTICK_PERIOD_MS);
                    
                    if (!process_manager.is_process_alive(led_process_id))
                        break;
                        
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 0, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(250 / portTICK_PERIOD_MS);
                }
                break;

            case LED_BLINK_LS:
                for (;;)
                {
                    // Check if process should continue and no new mode in queue
                    if (!process_manager.is_process_alive(led_process_id) || 
                        xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    
                    if (!process_manager.is_process_alive(led_process_id))
                        break;
                        
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
                    // Check if process should continue and no new mode in queue
                    if (!process_manager.is_process_alive(led_process_id) || 
                        xQueueReceive(LEDQueue, &mode, 0))
                        break;
                    
                    int random_time = random(50, 250);
                    ledc_set_fade_with_time(ledc.speed_mode, ledc.channel, 5000, 100);
                    ledc_fade_start(ledc.speed_mode, ledc.channel, LEDC_FADE_NO_WAIT);
                    vTaskDelay(random_time / portTICK_PERIOD_MS);
                    
                    if (!process_manager.is_process_alive(led_process_id))
                        break;
                        
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
    
    ESP_LOGI(TAG, "LED task terminating");
    vTaskDelete(NULL);
}

// Function to stop LED process
void LED_NOTIFICATION::LED_stop()
{
    if (led_process_id != 0) {
        ESP_LOGI(TAG, "Stopping LED process ID: %lu", led_process_id);
        process_manager.delete_process(led_process_id);
        led_process_id = 0;
    }
}

// Function to suspend LED process
void LED_NOTIFICATION::LED_suspend()
{
    if (led_process_id != 0) {
        ESP_LOGI(TAG, "Suspending LED process ID: %lu", led_process_id);
        process_manager.suspend_process(led_process_id);
    }
}

// Function to resume LED process
void LED_NOTIFICATION::LED_resume()
{
    if (led_process_id != 0) {
        ESP_LOGI(TAG, "Resuming LED process ID: %lu", led_process_id);
        process_manager.resume_process(led_process_id);
    }
}

// Function to get LED process info
uint32_t LED_NOTIFICATION::LED_get_process_id()
{
    return led_process_id;
}

LED_NOTIFICATION led_notification;