#ifndef HELPERS_H
#define HELPERS_H

#include <lvgl.h>

// Group of interactive objects
extern lv_indev_drv_t kb_drv;
extern lv_indev_t *kb_indev;
extern lv_group_t *group_interact;

// variables that are used multiple times
extern lv_obj_t *btn;

// save the current state of the menu
extern bool isInMenu;
extern bool isInSubMenu;

extern void clear_group_focus();

extern void backToMenu(lv_event_t *e);

// events
// void mainScreen(lv_obj_t *parent);
void mainScreen();

#endif