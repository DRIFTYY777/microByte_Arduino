#include <Arduino.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include <TFT_eSPI.h>

#include <esp32-hal-log.h>

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;

bool boot_screen_ani = true;

static const char *TAG = "microByte_main";

TFT_eSPI tft = TFT_eSPI();

void setup()
{
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("Hello World!");
    



    /**************** Basic initialization **************/
    sys_manager.system_init_config();

    led_notification.LED_init();
    led_notification.LED_mode(LED_FADE_ON);

    sys_manager.system_info();
    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    //backlight.backlight_init();
    //backlight.backlight_set(10);

    // sd_card.sd_init();

    // batteryQueue = xQueueCreate(1, sizeof(struct BATTERY_STATUS));
    // battery.battery_init();
}

void loop()
{
}