#ifndef _NOTIFICATIONBAR_H
#define _NOTIFICATIONBAR_H

#include <lvgl.h>

void notificationBar(lv_obj_t *parent, bool isHidden, const char *text);
void notificationBar(lv_obj_t *parent, const char *text, 
                     void (*event_handler)(lv_event_t *)

);

#endif // _NOTIFICATIONBAR_H