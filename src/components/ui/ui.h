#ifndef UI_H
#define UI_H

#include <lvgl.h>
// #include <TFT_eSPI.h>

// extern TFT_eSPI tft;

/* Initilization of Display Drivers and LVGL drivers */
void ui_init();

/* Initilization of screens (GUI) */
void GUI_frontend();

/* GUI Tasks with UserInput tasks */
void GUI_task(void *pvParameters);

#endif // UI_H
