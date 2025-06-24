//
// Created by dhima on 24-06-2025.
//

#ifndef WIDGET_H
#define WIDGET_H


#include <lvgl.h>


static lv_obj_t *waiting_overlay = nullptr;
void show_waiting_overlay(lv_obj_t *parent);

void hide_waiting_overlay();

void hide_overlay_cb(lv_timer_t * timer);


#endif //WIDGET_H
