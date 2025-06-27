//
// Created by dhima on 24-06-2025.
//

#include "usbUI.h"

#include "helpers.h"
#include "notificationBar.h"
#include "widget.h"

#include <components/drivers/usb/usbHal.h>

#include <Arduino.h>

void usbEvent(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    if (strcmp(btn_text, "USB Mass Storage") == 0)
    {
        usbHal.modes(USB_MODE_MSC); // Set USB mode to Mass Storage
        show_waiting_overlay(lv_scr_act()); // Show overlay on current screen
        lv_timer_create(hide_overlay_cb, 1000, nullptr);
    }
}


void usbUIscreen()
{
    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(nullptr);
    lv_obj_set_size(new_screen, 300, 180);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);

    // title bar
    notificationBar(new_screen, false, "USB");
    lv_obj_clear_flag(new_screen, LV_OBJ_FLAG_SCROLLABLE);


    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(new_screen);
    lv_obj_set_size(Menu, 300, 180);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    /*Add buttons to the list*/
    lv_obj_t *btn;

    btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
    lv_obj_add_event_cb(btn, backToMenu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_USB, "USB Mass Storage");
    lv_obj_add_event_cb(btn, usbEvent, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    // make first button focused
    lv_group_focus_obj(btn);

    lv_scr_load(new_screen);
}
