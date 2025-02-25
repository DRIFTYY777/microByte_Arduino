#include "user_input.h"
#include <esp32-hal-log.h>
#include <components/system_config/system_manager.h>
#include <components/system_config/system_config.h>
#include "TCA9555.h"

static const char *TAG = "user_input";

TCA9555 TCA(TCA_dev_address);

void UserInput::input_init(void)
{
    Wire.begin(I2C_SDA, I2C_SCL, TCA_CLK_SPEED);
    TCA.begin();
    Wire.setClock(TCA_CLK_SPEED);

    // GND___________---__________IOEXPANDER
}
uint16_t UserInput::input_read(void)
{
    uint16_t input_value = TCA.read16();

    // Check if the menu button it was pushed
    if (!((input_value >> 11) & 0x01))
    { // Temporary workaround !((inputs_value >>11) & 0x01) is the real button

        struct SYSTEM_MODE management;

        // Get the actual time
        uint32_t actual_time = xTaskGetTickCount() / portTICK_PERIOD_MS;

        // Check if any of the special buttons was pushed
        if (!((input_value >> 0) & 0x01))
        {
            // Down arrow, volume down
            /*  int volume_aux = audio_volume_get();
              volume_aux -= 10;
              if(volume_aux < 0)volume_aux = 0;

              management.mode = MODE_CHANGE_VOLUME;
              management.volume_level = volume_aux;

              if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((input_value >> 2) & 0x01))
        {
            // UP arrow, volume UP
            /* int volume_aux = audio_volume_get();
             volume_aux += 10;
             if(volume_aux > 100)volume_aux = 100;

             management.mode = MODE_CHANGE_VOLUME;
             management.volume_level = volume_aux;

             if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((input_value >> 1) & 0x01))
        {
            /*  // Right arrow, brightness up
              int brightness_aux = 0;//st7789_backlight_get();
              brightness_aux += 10;
              if(brightness_aux > 100)brightness_aux = 100;

              management.mode = MODE_CHANGE_BRIGHT;
              management.volume_level = brightness_aux;

              if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((input_value >> 3) & 0x01))
        {
            /*   // Left arrow, brightness down
               int brightness_aux = 0;//st7789_backlight_get();
               brightness_aux -= 10;
               if(brightness_aux < 0 )brightness_aux = 0;

               management.mode = MODE_CHANGE_BRIGHT;
               management.volume_level = brightness_aux;

               if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else
        {
            if ((actual_time - menu_btn_time) > 25)
            {
                printf("Menu\r\n");
                management.mode = MODE_GAME;
                management.status = 0;

                if (xQueueSend(modeQueue, &management, (TickType_t)10) != pdPASS)
                    ESP_LOGE(TAG, "Queue send failed");
                menu_btn_time = actual_time;
            }
        }
        return 0xFFFF;
    }
    else
    {
        return input_value;
    }
}

UserInput user_input;
