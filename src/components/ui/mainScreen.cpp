#include "mainScreen.h"
#include <Arduino.h>

// internal apps
#include <components/Apps/EvilApple/ui/evilAppleUI.h>
#include <components/external_app/externalAppUI.h>

#include <components/Apps/pwmGenerator/ui/pwmUI.h>

#include <components/ota/otaUI.h>
#include <components/emulators/EmulatorUI.h>

// ui components
#include "helpers.h"
#include "notificationBar.h"
#include "settings.h"

//

static void EventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        const char *text = lv_list_get_btn_text(list1, obj);
        if (strcmp(text, "Evil Apple") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createEVIL_APPLEScreen();
        }
        else if (strcmp(text, "External App") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createExternalAppScreen();
        }
        else if (strcmp(text, "Emulator") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createEmulatorScreen();
        }
        else if (strcmp(text, "Update Firmware") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createOTAScreen();
        }
        else if (strcmp(text, "USB") == 0)
        {
            Serial.println("USB");
        }
        else if (strcmp(text, "Settings") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createSettingScreen();
        }
        else if (strcmp(text, "PWM Generator") == 0)
        {
            isInMenu = false;
            clear_group_focus();
            createPWMscreen();
        }
    }
}

// void mainScreen(lv_obj_t *parent)
void mainScreen()
{

    // clear the whole screen
    isInMenu = true;
    isInSubMenu = false;

    lv_obj_t *parent = lv_scr_act(); // Active screen

    notificationBar(parent, true, NULL);

    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(parent);
    // lv_scr_load(Menu); // Load the new screen

    lv_obj_set_size(Menu, 300, 180);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    /* Buttons to the menu */
    btn = lv_list_add_btn(Menu, LV_SYMBOL_BLUETOOTH, "Evil Apple");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_BARS, "PWM Generator");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_PLAY, "Emulator");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_DIRECTORY, "External App");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_DOWNLOAD, "Update Firmware");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "Settings");
    lv_obj_add_event_cb(btn, EventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    lv_obj_t *list1 = lv_obj_get_child(lv_scr_act(), 1);
    if (list1)
    {
        lv_obj_t *first_button = lv_obj_get_child(list1, 0);
        if (first_button)
        {
            lv_group_focus_obj(first_button);
        }
    }
}
