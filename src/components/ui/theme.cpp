//
// Created by dhima on 15-06-2025.
//

#include "theme.h"

lv_res_t change_global_theme(theme_type_t theme_type, lv_color_t color_primary, lv_color_t color_secondary,
    bool dark_mode, const lv_font_t *font) {

    lv_theme_t *theme = NULL;
    lv_disp_t *disp = lv_disp_get_default();

    if(disp == NULL) {
        LV_LOG_ERROR("No display found");
        return LV_RES_INV;
    }

    // Get display properties
    lv_coord_t disp_width = lv_disp_get_hor_res(disp);
    lv_coord_t disp_height = lv_disp_get_ver_res(disp);

    // Use default font if none specified
    if(font == NULL) {
        font = LV_FONT_DEFAULT;
    }

    // Initialize the selected theme
    switch(theme_type) {
        case THEME_DEFAULT:
            theme = lv_theme_default_init(disp, color_primary, color_secondary,
                                        dark_mode, font);
            break;

        case THEME_BASIC:
            theme = lv_theme_basic_init(disp);
            break;

        case THEME_MONO:
            theme = lv_theme_mono_init(disp, dark_mode, font);
            break;

        default:
            LV_LOG_ERROR("Unknown theme type");
            return LV_RES_INV;
    }

    if(theme == NULL) {
        LV_LOG_ERROR("Failed to initialize theme");
        return LV_RES_INV;
    }

    // Apply the theme to the display
    lv_disp_set_theme(disp, theme);

    // Refresh all objects on all screens to apply the new theme
    lv_obj_t *screen = lv_disp_get_scr_act(disp);
    if(screen != NULL) {
        lv_obj_refresh_style(screen, LV_PART_ANY, LV_STYLE_PROP_ANY);
    }

    // Also refresh inactive screens
    lv_obj_t *scr_list = lv_disp_get_scr_prev(disp);
    while(scr_list != NULL) {
        lv_obj_refresh_style(scr_list, LV_PART_ANY, LV_STYLE_PROP_ANY);
        scr_list = lv_disp_get_scr_prev(disp);
    }

    LV_LOG_INFO("Theme changed successfully");
    return LV_RES_OK;
}

lv_res_t set_theme_preset(const char *preset_name) {
    if(strcmp(preset_name, "dark") == 0) {
        return change_global_theme(THEME_DEFAULT, lv_palette_main(LV_PALETTE_BLUE),
                                 lv_palette_main(LV_PALETTE_GREY), true, NULL);
    }
    else if(strcmp(preset_name, "light") == 0) {
        return change_global_theme(THEME_DEFAULT, lv_palette_main(LV_PALETTE_BLUE),
                                 lv_palette_main(LV_PALETTE_GREY), false, NULL);
    }
    else if(strcmp(preset_name, "blue") == 0) {
        return change_global_theme(THEME_DEFAULT, lv_palette_main(LV_PALETTE_BLUE),
                                 lv_palette_main(LV_PALETTE_BLUE_GREY), false, NULL);
    }
    else if(strcmp(preset_name, "red") == 0) {
        return change_global_theme(THEME_DEFAULT, lv_palette_main(LV_PALETTE_RED),
                                 lv_palette_main(LV_PALETTE_GREY), false, NULL);
    }
    else if(strcmp(preset_name, "green") == 0) {
        return change_global_theme(THEME_DEFAULT, lv_palette_main(LV_PALETTE_GREEN),
                                 lv_palette_main(LV_PALETTE_GREY), false, NULL);
    }
    else if(strcmp(preset_name, "basic") == 0) {
        return change_global_theme(THEME_BASIC, lv_color_white(), lv_color_black(), false, NULL);
    }
    else if(strcmp(preset_name, "mono") == 0) {
        return change_global_theme(THEME_MONO, lv_color_white(), lv_color_black(), false, NULL);
    }
    else {
        LV_LOG_ERROR("Unknown preset: %s", preset_name);
        return LV_RES_INV;
    }
}
