#include "externalAppUI.h"

#include <Arduino.h>

#include <components/drivers/sd_card/sd_card.h>
#include <components/ui/helpers.h>
#include <components/system_config/system_manager.h>

static const char *TAG = "EXTERNAL_APP_UI";

void backButtonEvent(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);
    app.mode = MODE_NONE;
}

void ExternalAppEvientHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(10); // preventing sudden crash
        ESP_LOGI(TAG, "Loading: %s", (char *)lv_list_get_btn_text(obj));
        app.status = STATUS_RUNNING;
        strcpy(app.aap_name, lv_list_get_btn_text(lv_obj_get_parent(obj), obj));

        Serial.println(app.aap_name);

        if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
        {
            Serial.println("Successfully sent modeQueue");
        }
        // clear all
        lv_obj_clean(lv_scr_act());
    }
}

void createExternalAppScreen(lv_obj_t *parent, lv_event_t *e)
{
    isInMenu = false;
    app.mode = MODE_EXT_APP;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        /*Create a list on parent*/
        lv_obj_t *Menu = lv_list_create(lv_scr_act());
        lv_obj_set_size(Menu, 300, 180);
        lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

        char *app_list[100];
        uint8_t app_num = sd_card.sd_app_list(app_list, false);

        ESP_LOGI(TAG, "Found %i applications", app_num);

        lv_obj_t *btn;
        if (app_num > 0)
        {
            for (uint8_t i = 0; i < app_num; i++)
            {
                btn = lv_list_add_btn(Menu, LV_SYMBOL_FILE, app_list[i]);
                lv_obj_add_event_cb(btn, ExternalAppEvientHandler, LV_EVENT_CLICKED, NULL);
                lv_group_add_obj(group_interact, btn); // Add button to group for interaction
            }
            // add back button to list
            btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
            lv_obj_add_event_cb(btn, backButtonEvent, LV_EVENT_CLICKED, NULL);
            lv_group_add_obj(group_interact, btn); // Add back button to group
        }
        else
        {
            // add back button to list
            btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
            lv_obj_add_event_cb(btn, backButtonEvent, LV_EVENT_CLICKED, NULL);
            lv_group_add_obj(group_interact, btn); // Add back button to group
        }
        lv_group_focus_obj(lv_group_get_focused(group_interact)); // ensure focus starts on a valid object.
    }
}