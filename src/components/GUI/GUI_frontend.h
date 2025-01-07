#ifndef GUI_FRONTEND_H
#define GUI_FRONTEND_H

// #include "LVGL/lvgl.h"
#include "lvgl.h"

// #include "LVGL/src/lv_themes/lv_theme_material.h"

#include <components/system_config/system_manager.h>

#include <components/GUI/GUI.h>



#define bouncing_time 7 // MS to wait to avoid bouncing on button selection

class GUI_FRONTEND
{

public:
    // TODO: Get this configuration from the ROM

    /**********************
     *  LVGL groups and objects
     **********************/






    static lv_group_t *group_interact; // Group of interactive objects

    static lv_indev_t *kb_indev;

    // Notification bar container objects
    static lv_obj_t *notification_cont;
    static lv_obj_t *battery_bar;
    static lv_obj_t *battery_label;
    static lv_obj_t *WIFI_label;
    static lv_obj_t *BT_label;
    static lv_obj_t *SD_label;
    static lv_obj_t *Charging_label;

    // Main menu objects

    static lv_obj_t *tab_main_menu;
    static lv_obj_t *tab_emulators;
    static lv_obj_t *tab_ext_app_manager;
    static lv_obj_t *tab_bt_controller;
    static lv_obj_t *tab_config;

    // Emulators menu objects

    static lv_obj_t *list_emulators_main;
    static lv_obj_t *btn_emulator_lib;
    static lv_obj_t *container_header_game_icon;
    static lv_obj_t *list_game_emulator;
    static lv_obj_t *list_game_options;
    static lv_obj_t *mbox_game_options;
    // Buttons of the game initialization list
    static lv_obj_t *btn_game_new;
    static lv_obj_t *btn_game_resume;
    static lv_obj_t *btn_game_delete;

    // On-game menu objects

    static lv_obj_t *list_on_game;
    static lv_obj_t *mbox_volume;
    static lv_obj_t *mbox_brightness;

    // External app menu objects

    static lv_obj_t *btn_ext_app;
    static lv_obj_t *list_external_app;

    // Configuration menu objects

    static lv_obj_t *config_btn;
    static lv_obj_t *list_config;
    static lv_obj_t *list_fw_update;
    static lv_obj_t *mbox_about;
    static lv_obj_t *mbox_color;

    void GUI_frontend(void);
    void async_battery_alert();

    static bool user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
    static void battery_status_task(lv_task_t *task);

    // Emulators menu
    static void emulators_menu(lv_obj_t *parent);
    static void emulators_list_cb(lv_obj_t *parent, lv_event_t e);
    static void emulator_list_event(lv_obj_t *parent, lv_event_t e);
    static void game_execute_cb(lv_obj_t *parent, lv_event_t e);
    static void game_menu_cb(lv_obj_t *parent, lv_event_t e);
    static void msgbox_no_game_cb(lv_obj_t *msgbox, lv_event_t e);
    static void game_list_cb(lv_obj_t *parent, lv_event_t e);

    // On game menu
    static void on_game_menu();
    static void slider_volume_cb(lv_obj_t *slider, lv_event_t e);
    static void slider_brightness_cb(lv_obj_t *slider, lv_event_t e);
    static void list_game_menu_cb(lv_obj_t *parent, lv_event_t e);

    // External app menu
    static void external_app_menu(lv_obj_t *parent);
    static void external_app_cb(lv_obj_t *parent, lv_event_t e);
    static void app_execute_cb(lv_obj_t *parent, lv_event_t e);

    // Configuration menu
    void config_menu(lv_obj_t *parent);
    static void configuration_cb(lv_obj_t *parent, lv_event_t e);
    static void config_option_cb(lv_obj_t *parent, lv_event_t e);
    static void mbox_config_cb(lv_obj_t *parent, lv_event_t e);
    static void fw_update_cb(lv_obj_t *parent, lv_event_t e);

    // Extra functions
    static void GUI_theme_color(uint8_t color_selected);
};

extern GUI_FRONTEND gui_frontend;

#endif