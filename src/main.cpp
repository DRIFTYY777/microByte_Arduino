
/*

https://maximeborges.github.io/esp-stacktrace-decoder/
*/



#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/time.h>

#include "components/ui/ui.h"

#include <TFT_eSPI.h>

/*
    LVGL Version: 8.3.9
*/

TFT_eSPI tft = TFT_eSPI();

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;

bool boot_screen_ani = true;
static const char *TAG = "Main";

void setup()
{
    Serial.begin(115200);
    // local_time.begin();

    /**************** Basic initialization **************/
    sys_manager.system_init_config();
    sys_manager.system_info();
    led_notification.LED_init();
    led_notification.LED_mode(LED_FADE_ON);

    sys_manager.system_info();
    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    backlight.backlight_init();
    backlight.backlight_set(sys_manager.system_get_config(SYS_BRIGHT));

    ui_init();
    xTaskCreatePinnedToCore(GUI_task, "GUI_task", 1024 * 6, NULL, 1, &gui_handler, 1);
    GUI_frontend();

    // sd_card.sd_init();

    user_input.input_init();
    // batteryQueue = xQueueCreate(1, sizeof(struct BATTERY_STATUS));
    // battery.battery_init();
}

void loop()
{
}
