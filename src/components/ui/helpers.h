#ifndef HELPERS_H
#define HELPERS_H

#include <lvgl.h>

// Group of interactive objects
extern lv_indev_drv_t kb_drv;
extern lv_indev_t *kb_indev;
extern lv_group_t *group_interact;

extern bool isInMenu;
extern bool isInSubMenu;

#endif