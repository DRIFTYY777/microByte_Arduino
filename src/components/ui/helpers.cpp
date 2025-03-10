#include "helpers.h"

#include <Arduino.h>

#include "mainScreen.h"

// Group of interactive objects
lv_indev_drv_t kb_drv;
lv_indev_t *kb_indev;
lv_group_t *group_interact;

// variables that are used multiple times
lv_obj_t *btn;

// save the current state of the menu
bool isInMenu = true;
bool isInSubMenu = false;

void backToMenu(lv_event_t *e)
{
    isInMenu = true;
    isInSubMenu = false;
    clear_group_focus();
    delay(10);
    mainScreen();
}

void clear_group_focus()
{
    lv_obj_t *obj;
    while ((obj = lv_group_get_focused(group_interact)) != NULL)
    {
        lv_group_remove_obj(obj);
    }
}
