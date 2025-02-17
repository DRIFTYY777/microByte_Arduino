#pragma once
#include <lvgl.h>
#include <TFT_eSPI.h>

// extern "C"
// {
// #include <components/drivers/display/st7789.h>
// #include <components/drivers/display/fontx.h>
// }

#define MSG_LOW_BATTERY_GAME 0x00
#define MSG_LOW_BATTERY 0x01

extern TFT_eSPI tft;

void ui_init();
void GUI_task(void *arg);
void GUI_async_message();

void UI_ELEMENTS();
void create_heart();
