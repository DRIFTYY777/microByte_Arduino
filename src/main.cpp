
/*

https://maximeborges.github.io/esp-stacktrace-decoder/
*/

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>

#include <components/drivers/backlight/backlight.h>
// #include <components/drivers/battery/battery.h>
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


    old
    name=TFT_eSPI
    version=2.5.43


*/

TFT_eSPI tft = TFT_eSPI();

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;

bool boot_screen_ani = true;
static const char *TAG = "Main";

void printRam()
{
    // Check if PSRAM is enabled
    if (psramInit())
    {
        Serial.println("PSRAM initialized successfully!");
    }
    else
    {
        Serial.println("PSRAM initialization failed...");
        while (1)
            ; // Stop if PSRAM isn't available
    }

    // Check total available PSRAM
    size_t psramSize = ESP.getPsramSize();
    Serial.print("Total PSRAM: ");
    Serial.println(psramSize);

    // Check available PSRAM
    size_t freePsram = ESP.getFreePsram();
    Serial.print("Free PSRAM: ");
    Serial.println(freePsram);

    // Allocate memory in PSRAM to store the string "Forum"
    const char *originalString = "Forum";
    int stringLength = strlen(originalString) + 1; // +1 for null terminator

    // Allocate memory in PSRAM for the string
    char *psramString = (char *)heap_caps_malloc(stringLength, MALLOC_CAP_SPIRAM);

    if (psramString == NULL)
    {
        Serial.println("Failed to allocate memory in PSRAM");
    }
    else
    {
        Serial.println("PSRAM memory allocated successfully!");

        // Copy the string "Forum" to PSRAM
        strcpy(psramString, originalString);

        // Verify by printing the string from PSRAM
        Serial.print("Stored string in PSRAM: ");
        Serial.println(psramString);

        // Free the allocated memory after use (optional, depending on the application)
        heap_caps_free(psramString);
    }
}

void setup()
{
    Serial.begin(115200);

    // local_time.begin();

    /**************** Basic initialization **************/
    sys_manager.system_init_config();
    sys_manager.system_info();
    // led_notification.LED_init();
    // led_notification.LED_mode(LED_FADE_ON);

    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    backlight.backlight_init();
    backlight.backlight_set(100);
    ui_init();
    GUI_frontend();

    xTaskCreatePinnedToCore(GUI_task, "GUI_task", 1024 * 6, NULL, 1, &gui_handler, 1);

    user_input.input_init();

    sd_card.sd_init();
}

/*

nvs,      data, nvs,     0x9000,  0x4000,
otadata,  data, ota,     0xd000,  0x2000,
phy_init, data, phy,     0xf000,  0x1000,
ota_0,    app,  ota_0,   0x10000, 1M,
ota_1,    app,  ota_1,   0x110000, 1M,
factory,  app,  factory, 0x210000,  5M,

*/

void loop()
{
}
