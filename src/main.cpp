#include <Arduino.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

// extern "C"
// {
// #include <components/drivers/display/st7789.h>
// #include <components/drivers/display/fontx.h>
// }

#include <components/ui/GUI.h>

#include <esp32-hal-log.h>

TFT_eSPI tft = TFT_eSPI(); // Define global TFT instance

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;

bool boot_screen_ani = true;
static const char *TAG = "microByte_main";

void setup()
{
    Serial.begin(115200);

    /**************** Basic initialization **************/
    sys_manager.system_init_config();

    led_notification.LED_init();
    led_notification.LED_mode(LED_FADE_ON);

    sys_manager.system_info();
    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    backlight.backlight_init();
    backlight.backlight_set(100);

    ui_init();

    xTaskCreatePinnedToCore(GUI_task, "Graphical User Interface", 8156, NULL, 1, &gui_handler, 1);
    UI_ELEMENTS();
    // sd_card.sd_init();

    // batteryQueue = xQueueCreate(1, sizeof(struct BATTERY_STATUS));
    // battery.battery_init();
}

void loop()
{
}