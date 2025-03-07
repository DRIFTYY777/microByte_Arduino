#include "EmulatorUI.h"

#include <Arduino.h>

#include <components/ui/helpers.h>

#include <components/system_config/system_manager.h>

#include <components/drivers/sd_card/sd_card.h>

static const char *TAG = "EMULATOR_UI";

static lv_obj_t *container_header_game_icon;

uint8_t emulator_selected = 0x00;

static void gameOptions(lv_event_t *e);
void executeGame(lv_event_t *e);
void game_list_event_handler(lv_event_t *e);

void createEmulatorScreen(lv_obj_t *parent, lv_event_t *e)
{
    app.mode = MODE_GAME;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        /*Create a list on parent*/
        lv_obj_t *Menu = lv_list_create(lv_scr_act());
        lv_obj_set_size(Menu, 300, 180);
        lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

        lv_obj_t *btn;

        // Add buttons to the list
        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "NES");
        lv_obj_add_event_cb(btn, game_list_event_handler, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "SNES");
        lv_obj_add_event_cb(btn, game_list_event_handler, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        // back btn
        btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
        // lv_obj_add_event_cb(btn, backButtonEventHandler, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        // make the list focusable and intraactable
        lv_group_focus_obj(Menu);
    }
}

void game_list_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    lv_obj_clean(lv_scr_act());

    if (strcmp(btn_text, "NES") == 0)
    {
        Serial.println("NES");
        emulator_selected = NES;
    }
    else if (strcmp(btn_text, "SNES") == 0)
    {
        Serial.println("SNES");
        emulator_selected = SNES;
    }

    char *game_list[100];
    uint8_t games_num = sd_card.sd_game_list(game_list, emulator_selected);
    ESP_LOGI(TAG, "Found %i games", games_num);
    Serial.println("Found games");
    Serial.println(games_num);

    // clear the screen
    lv_obj_clean(lv_scr_act());

    // Create a list on parent
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    // Create a list on parent

    lv_obj_t *Menu = lv_list_create(lv_scr_act());
    lv_obj_set_size(Menu, 300, 180);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t *btn;

    for (uint8_t i = 0; i < games_num; i++)
    {
        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, game_list[i]);
        lv_obj_add_event_cb(btn, gameOptions, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);
    }
}

static void gameOptions(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    lv_obj_clean(lv_scr_act());

    if (code == LV_EVENT_CLICKED)
    {
        /*Create a list on parent*/
        lv_obj_t *Menu = lv_list_create(lv_scr_act());
        lv_obj_set_size(Menu, 300, 180);
        lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

        lv_obj_t *btn;

        // Add buttons to the list
        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "New Game");
        lv_obj_add_event_cb(btn, executeGame, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        if (sd_card.sd_sav_exist(const_cast<char *>(lv_list_get_btn_text(list1, obj)), emulator_selected))
        {
            btn = lv_list_add_btn(Menu, LV_SYMBOL_OK, "Resume Game");
            lv_obj_add_event_cb(btn, executeGame, LV_EVENT_CLICKED, NULL);
            lv_group_add_obj(group_interact, btn);

            btn = lv_list_add_btn(Menu, LV_SYMBOL_TRASH, "Delete Save");
            lv_obj_add_event_cb(btn, executeGame, LV_EVENT_CLICKED, NULL);
            lv_group_add_obj(group_interact, btn);
        }
        // make the list focusable and intraactable
        lv_group_focus_obj(Menu);
    }
}

void executeGame(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);
}
