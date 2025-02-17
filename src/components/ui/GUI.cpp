#include "GUI.h"

#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE 240 * 20 // Horizontal Res * 40 vetical pixels

static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;
static SemaphoreHandle_t xGuiSemaphore;

void display_HAL_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_map->full, w * h, true);
    tft.setSwapBytes(true);
    tft.endWrite();
    lv_disp_flush_ready(drv);
}

void ui_init()
{
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    xGuiSemaphore = xSemaphoreCreateMutex();
    lv_init();
    
    static EXT_RAM_ATTR lv_color_t *buf1[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, size_in_px);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = display_HAL_flush;
    disp_drv.draw_buf = &disp_buf;
    
    lv_disp_drv_register(&disp_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000)); // LV_TICK_PERIOD_MS expressed as microseconds
}
// Async refresh
void GUI_refresh()
{
    for (int i = 0; i < 3; i++)
        lv_disp_flush_ready(&disp_drv);
}
void GUI_task(void *arg)
{
    while (1)
    {
        lv_task_handler();
    }
    printf("delete\r\n");
    // A task should NEVER return
    vTaskDelete(NULL);
}
void UI_ELEMENTS()
{
    // Create UI elements
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Hello LVGL!");
}
static void lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}
