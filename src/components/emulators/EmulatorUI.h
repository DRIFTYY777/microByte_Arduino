#ifndef EMULATORS_EMULATORUI_H
#define EMULATORS_EMULATORUI_H

#include <lvgl.h>

static void on_game_menu(lv_event_t e);

static void executeGame(lv_event_t *e);

void createEmulatorScreen(lv_obj_t *parent, lv_event_t *e);

#endif // EMULATORS_EMULATORUI_H