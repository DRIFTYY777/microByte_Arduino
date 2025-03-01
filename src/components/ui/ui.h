#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void ui_init();
void GUI_frontend();

// GUI_task
void GUI_task(void *pvParameters);

#endif // UI_H
