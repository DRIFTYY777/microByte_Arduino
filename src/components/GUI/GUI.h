#ifndef GUI_H
#define GUI_H

#include <components/drivers/display/display_HAL/display_HAL.h>

#include <components/GUI/lv_conf.h>



#include "freertos/FreeRTOS.h"    // Include this first
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <semphr.h>

#define MSG_LOW_BATTERY_GAME 0x00
#define MSG_LOW_BATTERY 0x01
#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE 240 * 20 // Horizontal Res * 40 vetical pixels

static SemaphoreHandle_t xGuiSemaphore;


class GUI
{
private:
    static void lv_tick_task(void *arg);
    static lv_disp_drv_t disp_drv;

public:
    void GUI_init(void);
    void GUI_task(void *arg);
    void GUI_refresh();
    void GUI_async_message();
};

#endif