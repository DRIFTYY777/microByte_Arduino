#include "helpers.h"

// Group of interactive objects
lv_indev_drv_t kb_drv;
lv_indev_t *kb_indev;
lv_group_t *group_interact;

bool isInMenu = true;
bool isInSubMenu = false;
