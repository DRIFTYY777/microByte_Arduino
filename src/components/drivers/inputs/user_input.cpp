#include "user_input.h"
#include <esp32-hal-log.h>
#include <components/system_config/system_manager.h>

#include "TCA9555.h"

static const char *TAG = "user_input";

void UserInput::input_init(void)
{
    ESP_LOGI(TAG, "Initalization of GPIO mux driver");
    tca9555.TCA955_init();
}

uint16_t UserInput::input_read(void)
{
    // Get the mux values
    uint16_t inputs_value = tca9555.TCA9555_readInputs();

    // Check if the menu button it was pushed
    if (!((inputs_value >> 11) & 0x01))
    { // Temporary workaround !((inputs_value >>11) & 0x01) is the real button

        struct SYSTEM_MODE management;

        // Get the actual time
        uint32_t actual_time = xTaskGetTickCount() / portTICK_PERIOD_MS;

        // Check if any of the special buttons was pushed
        if (!((inputs_value >> 0) & 0x01))
        {
            // Down arrow, volume down
            /*  int volume_aux = audio_volume_get();
              volume_aux -= 10;
              if(volume_aux < 0)volume_aux = 0;

              management.mode = MODE_CHANGE_VOLUME;
              management.volume_level = volume_aux;

              if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((inputs_value >> 2) & 0x01))
        {
            // UP arrow, volume UP
            /* int volume_aux = audio_volume_get();
             volume_aux += 10;
             if(volume_aux > 100)volume_aux = 100;

             management.mode = MODE_CHANGE_VOLUME;
             management.volume_level = volume_aux;

             if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((inputs_value >> 1) & 0x01))
        {
            /*  // Right arrow, brightness up
              int brightness_aux = 0;//st7789_backlight_get();
              brightness_aux += 10;
              if(brightness_aux > 100)brightness_aux = 100;

              management.mode = MODE_CHANGE_BRIGHT;
              management.volume_level = brightness_aux;

              if( xQueueSend( modeQueue,&management, ( TickType_t ) 10) != pdPASS ) ESP_LOGE(TAG, "Queue send failed");*/
        }
        else if (!((inputs_value >> 3) & 0x01))
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
        return inputs_value;
    }
}

UserInput user_input;
