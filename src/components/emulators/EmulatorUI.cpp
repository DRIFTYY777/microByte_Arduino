#include "EmulatorUI.h"

#include <Arduino.h>

#include <components/ui/helpers.h>
#include <components/ui/notificationBar.h>

#include <components/system_config/system_manager.h>

#include <components/drivers/sd_card/sd_card.h>

static const char *TAG = "EMULATOR_UI";

static lv_obj_t *container_header_game_icon;

uint8_t emulator_selected = 0x00;

static void gameOptions(lv_event_t *e);
void executeGame(lv_event_t *e);
void game_list_event_handler(lv_event_t *e);

void createEmulatorScreen()
{
    app.mode = MODE_GAME;

    // Create a new screen for the emulators
    lv_obj_t *new_screen = lv_obj_create(NULL);
    lv_obj_set_size(new_screen, 320, 240);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);

    // title bar
    notificationBar(new_screen, false, "Emulators");

    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(new_screen);
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

    btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "GameBoy");
    lv_obj_add_event_cb(btn, game_list_event_handler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "GameBoy Color");
    lv_obj_add_event_cb(btn, game_list_event_handler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    // back btn
    btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
    lv_obj_add_event_cb(btn, backToMenu, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    // make the list focusable and intraactable
    lv_group_focus_obj(Menu);

    // Safely switch to the new screen
    lv_scr_load(new_screen);
}

void back_To_EMUSCREEN(lv_event_t *e)
{
    /* Clearing all data like gameName, Emu.. */
    app.aap_name[0] = '\0';
    app.status = STATUS_STOPPED;
    emulator_selected = MODE_NONE;
    app.console = MODE_NONE;

    clear_group_focus();
    delay(50);
    createEmulatorScreen();
}

void game_list_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    /* List of Emulators */
    if (strcmp(btn_text, "NES") == 0)
    {
        Serial.println("NES");
        emulator_selected = NES;
        app.console = NES;
    }
    else if (strcmp(btn_text, "SNES") == 0)
    {
        Serial.println("SNES");
        emulator_selected = SNES;
        app.console = SNES;
    }
    else if (strcmp(btn_text, "GameBoy") == 0)
    {
        Serial.println("GameBoy");
        emulator_selected = GAMEBOY;
        app.console = GAMEBOY;
    }
    else if (strcmp(btn_text, "GameBoy Color") == 0)
    {
        Serial.println("GameBoy Color");
        emulator_selected = GAMEBOY_COLOR;
        app.console = GAMEBOY_COLOR;
    }

    /* List of games based on selected emulaotr */
    char *game_list[1000];
    uint8_t games_num = sd_card.sd_game_list(game_list, emulator_selected);
    ESP_LOGI(TAG, "Found %i games", games_num);

    // clear the screen
    clear_group_focus();
    // lv_obj_clean(lv_scr_act());

    // notification bar
    notificationBar(lv_scr_act(), false, "Games");

    // Create a list on parent
    lv_obj_t *Menu = lv_list_create(lv_scr_act());
    lv_obj_set_size(Menu, 300, 180);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t *btn;
    btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
    lv_obj_add_event_cb(btn, back_To_EMUSCREEN, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    for (uint8_t i = 0; i < games_num; i++)
    {
        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, game_list[i]);
        lv_obj_add_event_cb(btn, gameOptions, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);
    }

    // make first btn focusable
    lv_group_focus_obj(Menu);
}

/* UI Function */
static void gameOptions(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    ESP_LOGI(TAG, "Loading: %s", (char *)lv_list_get_btn_text(obj));
    strcpy(app.aap_name, lv_list_get_btn_text(lv_obj_get_parent(obj), obj));
    Serial.println(app.aap_name);

    // clear the screen
    clear_group_focus();
    // lv_obj_clean(lv_scr_act());

    // title bar
    notificationBar(lv_scr_act(), false, "Game Options");

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

        // back btn
        btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
        lv_obj_add_event_cb(btn, back_To_EMUSCREEN, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);
        // make the list focusable and intraactable
        lv_group_focus_obj(Menu);
    }
}

/* NON UIFunction */
void executeGame(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    if (strcmp(btn_text, "New Game") == 0)
    {
        Serial.println("New Game");

        app.mode = MODE_GAME;
        app.status = STATUS_RUNNING;
        app.load_save_game = 0;
        app.console = emulator_selected;
        Serial.println(app.aap_name);

        if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
        {
            ESP_LOGI(TAG, "Successfully sent modeQueue");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send modeQueue");
        }
    }
    else if (strcmp(btn_text, "Resume Game") == 0)
    {
        Serial.println("Resume Game");

        app.mode = MODE_GAME;
        app.status = STATUS_RUNNING;
        app.load_save_game = 1;
        app.console = emulator_selected;
        Serial.println(app.aap_name);

        if (xQueueSend(modeQueue, &app, (TickType_t)10) == pdTRUE)
        {
            ESP_LOGI(TAG, "Successfully sent modeQueue");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send modeQueue");
        }
    }
    else if (strcmp(btn_text, "Delete Save") == 0)
    {
        sd_card.sd_sav_remove(const_cast<char *>(app.aap_name), emulator_selected);
        // Update the list of options

        // clear the screen
        clear_group_focus();
        lv_obj_clean(lv_obj_get_parent(lv_obj_get_parent(obj)));
        gameOptions(e);
    }
}
