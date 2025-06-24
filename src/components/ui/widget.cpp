//
// Created by dhima on 24-06-2025.
//

#include "widget.h"

void show_waiting_overlay(lv_obj_t *parent) {
    if (waiting_overlay != nullptr) return; // Already active

    // Create a full-screen transparent object
    waiting_overlay = lv_obj_create(parent);
    lv_obj_remove_style_all(waiting_overlay);  // Remove default styles
    lv_obj_set_size(waiting_overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(waiting_overlay);
    lv_obj_set_style_bg_color(waiting_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(waiting_overlay, LV_OPA_50, 0); // Semi-transparent
    lv_obj_clear_flag(waiting_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(waiting_overlay, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);

    // Block inputs to objects behind
    lv_obj_add_flag(waiting_overlay, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(waiting_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Add a spinner (loading indicator)
    lv_obj_t *spinner = lv_spinner_create(waiting_overlay, 1000, 60);
    lv_obj_center(spinner);

    // Optional: add "Please wait..." label
    lv_obj_t *label = lv_label_create(waiting_overlay);
    lv_label_set_text(label, "Please wait...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);
}

void hide_waiting_overlay() {
    if (waiting_overlay != nullptr) {
        lv_obj_del(waiting_overlay);
        waiting_overlay = nullptr;
    }
}

// Timer callback
void hide_overlay_cb(lv_timer_t * timer)
{
    hide_waiting_overlay();
    lv_timer_del(timer); // Clean up
}