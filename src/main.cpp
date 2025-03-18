
/*

https://maximeborges.github.io/esp-stacktrace-decoder/
*/

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>
#include <WiFi.h>

#include <components/drivers/backlight/backlight.h>
// #include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/LocalTime.h>
#include <components/drivers/sd_card/formatSD.h>

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include "components/ui/ui.h"

#include <components/Apps/EvilApple/evilApple.h>

#include <components/external_app/external_app.h>
#include <components/ota/update_firmware.h>

// #include <components/emulators/NES/NesManager.h>
// #include <components/emulators/GBC/GboyManager.h>

#include <SD.h>

/*
    LVGL Version: 8.3.9
    old TFT_eSPI: 2.5.43 // not stable
*/

/// add blue ducky
/// MAX9814 mic
/// arduboy

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
// TimerHandle_t timer;

static const char *TAG = "Main";

uint8_t console_running;
bool boot_screen_ani = true;
bool game_running = false;
bool game_executed = false;

static void timer_isr(void)
{
    printf("save\r\n");
    app.mode = MODE_SAVE_GAME;
    if (xQueueSend(modeQueue, &app, (TickType_t)10) != pdPASS)
    {
        ESP_LOGE(TAG, "modeQueue send error");
    }
}

void setup()
{
    Serial.begin(115200);

    /* System Init for hardware state */
    sys_manager.system_init_config();
    sys_manager.system_info();

    /* Init of Time */
    // if (WiFi.status())
    //{
    //     local_time.init();
    // }

    delay(200);

    /* SD Crad */
    sd_card.sd_init();

    /* Init LED for Notification */
    led_notification.LED_init();

    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    /* Init of Display Backlight */
    backlight.backlight_init();

    /* Display and Lvgl driver init */
    ui_init();
    xTaskCreatePinnedToCore(GUI_task, "Graphical User Interface", 1024 * 12, NULL, 1, &gui_handler, 0);

    /* Init of GUI */
    GUI_frontend();

    /* Responsibile for User Input */
    user_input.input_init();

    /* Queue for creating or ... */
    modeQueue = xQueueCreate(1, sizeof(app));

    // WiFI.begin(WIFI_PASS, WIFI_SSID);
    // delay(1000);
    // if (WiFi.status() != WL_CONNECTED)
    // {
    //     ESP_LOGE(TAG, "WiFi not connected");
    // }

    // local_time.setDateTime("2021-09-01 12:00:00"); // set date time

    // print local time
    // ESP_LOGI(TAG, "Local Time: %s", local_time.getFormattedTime().c_str());
}

void loop()
{
    if (xQueueReceive(modeQueue, &app, portMAX_DELAY) == pdTRUE)
    {
        if (app.mode == MODE_APPLEJUICE)
        {
            if (app.status == STATUS_RUNNING)
            {
                while (app.status == STATUS_RUNNING)
                    evilApple.startAdvertising();
            }
            evilApple.stopAdvertising();
        }
        else if (app.mode == MODE_EXT_APP)
        {
            if (app.status == STATUS_RUNNING)
            {
                ESP_LOGI(TAG, "Loading external App");
                vTaskSuspend(gui_handler);
                vTaskDelay(1000 / portTICK_RATE_MS);
                external_app.external_app_init(app.aap_name);
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
            }
        }
        else if (app.mode == MODE_UPDATE)
        {
            if (app.status == STATUS_RUNNING)
            {
                ESP_LOGE(TAG, "Loading OTA");
                vTaskSuspend(gui_handler);
                vTaskDelay(1000 / portTICK_RATE_MS);
                update_firmware.update_init(app.aap_name);
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
            }
        }
    }
}
