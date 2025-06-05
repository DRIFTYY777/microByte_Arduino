/*

https://maximeborges.github.io/esp-stacktrace-decoder/
*/

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>
#include <WiFi.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/display/displayHal.h>
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
#include <components/emulators/NES/NesManager.h>
#include <components/emulators/GBC/GboyManager.h>

extern "C"
{
#include <components/boot/boot_screen.h>
#include <components/drivers/nrf24/nrf24.h>
}

/*

Amazing Spider-Man.gb
[ 22059][E][vfs_api.cpp:105]
 open(): /sd/Emulator/GameBoy/Save_Data/Amazing Spider-Man.gb.sav does not exist, no permits for creation


 gbu loader.c
*/

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


nrf24_config_t nrf24_config = {
    .pin_ce = NRF_CE,
    .pin_csn = NRF_CSN,
    .pin_mosi = HSPI_MOSI,
    .pin_miso = HSPI_MISO,
    .pin_sck = HSPI_CLK,
    .spi_host = SPI3_HOST,
    .spi_speed = NRF_CLK_SPEED,
};

void setup()
{
    Serial.begin(115200);
    Serial.print("\n");

    /* System Init for hardware state */
    sys_manager.system_init_config();
    sys_manager.system_info();
    app.mode = MODE_NONE;

    /* Internal RTC  Init. */
    local_time.init();
    local_time.setDate("2024-06-05");
    local_time.setTime("12:00:00");
    //local_time.setDateTime("2024-06-05 12:00:00");

    ESP_LOGE(TAG, "RTC: %s", local_time.getTime(), local_time.getDate());

    /* SD Card */
    sd_card.sd_init();

    /* Responsible for User Input */
    user_input.input_init();

    /* Display Drivers Init. */
    display_hall_init();

    vTaskDelay(1000 / portTICK_RATE_MS);

    /* Radio */
    nrf24_init(&nrf24_config);

    /* 1 Sec Delay */
    vTaskDelay(1000 / portTICK_RATE_MS);

    /* Testing NRF */
    Serial.println(nrf24_isConnected(&nrf24_config) ? "NRF24L01 connected" : "NRF24L01 not connected");

    /* Init LED for Notification */
    led_notification.LED_init();

    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM),
             sys_manager.system_memory(MEMORY_INTERNAL),
             sys_manager.system_memory(MEMORY_DMA));

    /* Init of Display Backlight */
    BACKLIGHT::backlight_init();

    /* Lvgl driver init */
    ui_init();
    xTaskCreatePinnedToCore(GUI_task, "Graphical User Interface", 1024 * 10, nullptr, 1, &gui_handler, 0);

    /* Init of GUI */
    GUI_frontend();

    /* Queue for creating or ... */
    modeQueue = xQueueCreate(1, sizeof(app));
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
            }
        }
        else if (app.mode == MODE_GAME)
        {
            if (app.console == NES)
            {
                if (app.status == STATUS_RUNNING)
                {
                    vTaskSuspend(gui_handler);
                    display_set_NES();
                    NES_start(app.aap_name);
                    if (app.load_save_game)
                    {
                        vTaskDelay(1500 / portTICK_RATE_MS);
                        NES_load_game();
                    }
                    game_executed = true;
                    game_running = true;
                }
            }
            else if (app.console == GAMEBOY_COLOR || app.console == GAMEBOY)
            {
                if (app.status == STATUS_RUNNING)
                {
                    vTaskSuspend(gui_handler);
                    // display_set_GB();
                    gnuboy_execute_game(app.aap_name, app.console, app.load_save_game);
                    gnuboy_start();
                    game_executed = true;
                    game_running = true;
                }
            }
        }
    }
}