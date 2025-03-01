#include "evilAppleUI.h"

#include <Arduino.h>

#include <components/ui/helpers.h>
#include <components/system_config/system_manager.h>
#include <components/Apps/EvilApple/evilApple.h>

const char *TAG = "EVIL APPLE";

void start_event(lv_event_t *e)
{
    evilApple.begin();
    app.status = STATUS_RUNNING;
    Serial.println("Start Event");

    if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
    {
        Serial.println("Successfully sent modeQueue");
    }
    else
    {
        Serial.println("Failed to send modeQueue");
    }
}

void stop_event(lv_event_t *e)
{
    app.status = STATUS_STOPPED;
    evilApple.stopAdvertising();

    if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
    {
        ESP_LOGI(TAG, "Successfully sent modeQueue");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to send modeQueue");
    }
}

void createEVIL_APPLEScreen(lv_obj_t *parent, lv_event_t *e)
{
    isInMenu = false;
    app.mode = MODE_APPLEJUICE;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        // Create a window
        lv_obj_t *window = lv_obj_create(lv_scr_act());
        lv_obj_set_size(window, 300, 180);
        lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
        lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

        // Create a label
        lv_obj_t *label = lv_label_create(window);
        lv_label_set_text(label, "EVIL APPLE");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20); // Align label to the top center

        // Create a button
        lv_obj_t *start_btn = lv_btn_create(window);
        lv_obj_set_size(start_btn, 80, 30);
        lv_obj_align(start_btn, LV_ALIGN_CENTER, -50, 0);
        lv_obj_t *start_label = lv_label_create(start_btn);
        lv_label_set_text(start_label, "Start");
        lv_obj_center(start_label);
        lv_obj_add_event_cb(start_btn, start_event, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, start_btn);

        // Create a button
        lv_obj_t *stop_btn = lv_btn_create(window);
        lv_obj_set_size(stop_btn, 80, 30);
        lv_obj_align(stop_btn, LV_ALIGN_CENTER, 50, 0);
        lv_obj_t *stop_label = lv_label_create(stop_btn);
        lv_label_set_text(stop_label, "Stop");
        lv_obj_center(stop_label);
        lv_obj_add_event_cb(stop_btn, stop_event, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, stop_btn);
    }
}