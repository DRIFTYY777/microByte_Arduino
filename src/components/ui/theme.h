//
// Created by dhima on 15-06-2025.
//

#include "lvgl.h"

#ifndef THEME_H
#define THEME_H


typedef enum {
    THEME_DEFAULT,
    THEME_BASIC,
    THEME_MONO
} theme_type_t;


lv_res_t change_global_theme(theme_type_t theme_type, lv_color_t color_primary,
                            lv_color_t color_secondary, bool dark_mode, const lv_font_t *font);

lv_res_t set_theme_preset(const char *preset_name);


#endif //THEME_H
