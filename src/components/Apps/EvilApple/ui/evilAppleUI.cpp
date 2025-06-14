#include "evilAppleUI.h"

#include <Arduino.h>

#include <components/ui/helpers.h>
#include <components/ui/notificationBar.h>

#include <components/system_config/system_manager.h>
#include <components/Apps/EvilApple/evilApple.h>

const char *TAG = "EVIL APPLE UI";

void start_event(lv_event_t *e)
{
    app.status = STATUS_RUNNING;
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

static void event_cb(lv_event_t * e)
{
    // if button is clicked, start the evil apple
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "Start button clicked");
        // change button text to stop
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0); // Get the label inside the button
        lv_label_set_text(label, "Stop");
        start_event(e);
    }
    else
    {
        ESP_LOGW(TAG, "Unexpected event code: %d", code);
    }

}


void createEVIL_APPLEScreen()
{
    app.mode = MODE_APPLEJUICE;

    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(nullptr);
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


    // lv_obj_t * btn = lv_btn_create(new_screen);
    // lv_obj_set_size(btn, 100, 50);
    // lv_obj_center(btn);
    // lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, nullptr);
    //
    // lv_obj_t * label = lv_label_create(btn);
    // lv_label_set_text(label, "Start");
    // lv_obj_center(label);







    // Safely switch to the new screen
    lv_scr_load(new_screen);
}
