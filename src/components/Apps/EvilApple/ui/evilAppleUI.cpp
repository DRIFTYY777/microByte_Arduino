#include "evilAppleUI.h"

#include <Arduino.h>

#include <components/ui/helpers.h>
#include <components/ui/notificationBar.h>

#include <components/system_config/system_manager.h>
#include <components/Apps/EvilApple/evilApple.h>

const char *TAG = "EVIL APPLE";

void start_event(lv_event_t *e)
{
    app.status = STATUS_RUNNING;
    Serial.println("Start Event");
    delay(150);
    evilApple.begin();

    if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
    {
        ESP_LOGI(TAG, "Successfully sent modeQueue");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to send modeQueue");
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

void createEVIL_APPLEScreen()
{
    app.mode = MODE_APPLEJUICE;

    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(NULL);
    lv_obj_set_size(new_screen, 300, 180);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);

    // title bar
    notificationBar(new_screen, false, "EVIL APPLE");
    lv_obj_clear_flag(new_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Start BTN */
    lv_obj_t *start_btn = lv_btn_create(new_screen);
    lv_obj_set_size(start_btn, 80, 30);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, -50, 0);
    lv_obj_t *start_label = lv_label_create(start_btn);
    lv_label_set_text(start_label, "Start");
    lv_obj_center(start_label);
    lv_obj_add_event_cb(start_btn, start_event, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, start_btn);

    /* Stop BTN */
    lv_obj_t *stop_btn = lv_btn_create(new_screen);
    lv_obj_set_size(stop_btn, 80, 30);
    lv_obj_align(stop_btn, LV_ALIGN_CENTER, 50, 0);
    lv_obj_t *stop_label = lv_label_create(stop_btn);
    lv_label_set_text(stop_label, "Stop");
    lv_obj_center(stop_label);
    lv_obj_add_event_cb(stop_btn, stop_event, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, stop_btn);

    /* Back BTN */
    lv_obj_t *back_btn = lv_btn_create(new_screen);
    lv_obj_set_size(back_btn, 80, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, backToMenu, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, back_btn);

    // Safely switch to the new screen
    lv_scr_load(new_screen);
}
