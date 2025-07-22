#include "ui.h"

// UI Components
#include "helpers.h"
#include "mainScreen.h"
#include "theme.h"

#include "../core/processManager.h"

// System
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

// driver
#include <components/drivers/backlight/backlight.h>
#include <components/drivers/display/displayHal.h>
// #include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
// #include <components/drivers/sound/sound.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/LocalTime.h>

// emulators
#include <esp_task_wdt.h>
#include <components/emulators/EmulatorUI.h>

/*
    LVGL Version: 8.3.9
    LV_USE_LOG 0
    FPS: 100
    CPU: 43%

    #define DISP_BUF_SIZE (240 * 10) // width * 2
    And single buffer in psram
    -SPI_RAM: 8386019 Bytes
    -INTERNAL_RAM: 276912 Bytes
    -DMA_RAM: 276912 Bytes
    PSRAM initialized successfully!
    Total PSRAM: 8386263
    Free PSRAM: 8381203
    PSRAM memory allocated successfully!
    Stored string in PSRAM: Forum

//////////////////////////////////////////

    #define DISP_BUF_SIZE (240 * 40) // width * 2
    and using 2 bugegr allocated in psram
    -SPI_RAM: 8386019 Bytes
    -INTERNAL_RAM: 276896 Bytes
    -DMA_RAM: 276896 Bytes
    PSRAM initialized successfully!
    Total PSRAM: 8386247
    Free PSRAM: 8347587
    PSRAM memory allocated successfully!
    Stored string in PSRAM: Forum
*/

#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE (240 * 30) // width * 2

static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;
static process_semaphore_t *xGuiSemaphore;

//static SemaphoreHandle_t xGuiSemaphore;


void ui_init()
{
//        xGuiSemaphore = xSemaphoreCreateMutex();

    xGuiSemaphore = semaphore_create("gui_semaphore", SEAMPHORE_TYPE_MUTEX, 1);

    lv_init();

    const int32_t size_in_px = DISP_BUF_SIZE;
    static lv_disp_draw_buf_t draw_buf;

    // buffer in psram
    //static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    //static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    
    static lv_color_t *buf1 = (lv_color_t *)memory_allocate(size_in_px * sizeof(lv_color_t));
    static lv_color_t *buf2 = (lv_color_t *)memory_allocate(size_in_px * sizeof(lv_color_t));

    // buffer in dma
    // static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_DMA);
    // static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_DMA);

    if (!buf1 || !buf2)
    {
        ESP_LOGE("LVGL", "Memory allocation failed!");
    }

    // print free memory
    ESP_LOGI("LVGL", "Total PSRAM: %d", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI("LVGL", "Free PSRAM: %d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    lv_disp_draw_buf_init(&draw_buf, buf2, buf1, size_in_px); // Reduced buffer size for non-PSRAM boards

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320; // height
    disp_drv.ver_res = 240;

    disp_drv.flush_cb = display_HAL_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Into to Theme
    change_global_theme(THEME_DEFAULT, lv_color_black(), lv_color_white(), true, nullptr);

    // Create timer for LVGL system ticks
    constexpr esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
}

[[noreturn]] void GUI_task(void *arg)
{
    while (true)
    {
        if (xGuiSemaphore != nullptr)
        {
            //if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
            if (semaphore_take(xGuiSemaphore, portMAX_DELAY))
            {
                lv_task_handler();
                semaphore_give(xGuiSemaphore);
                //                xSemaphoreGive(xGuiSemaphore);

            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5)); // Adjust tick rate as needed
    }
    vTaskDelete(nullptr);
}

static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void lv_example_img_1(void);

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    if (code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}
-
void lv_example_keyboard_1(void)
{
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());

    /*Create a text area. The keyboard will write here*/
    lv_obj_t *ta;
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta, "Hello");
    lv_obj_set_size(ta, 140, 80);

    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_size(ta, 140, 80);

    lv_keyboard_set_textarea(kb, ta);

    // make keyboard focus
    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, kb);
    lv_group_add_obj(group, ta);
    lv_indev_t *kb_indev = lv_indev_get_next(NULL);
    if (kb_indev != NULL)
    {
        lv_indev_set_group(kb_indev, group);
    }
}

void GUI_frontend()
{
    // Create a group for interactive objects
    lv_indev_drv_init(&kb_drv);
    // Telling input type to LVGL
    kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    // Telling the read function to LVGL
    kb_drv.read_cb = UserInput::user_input_task;
    // Register the input device
    kb_indev = lv_indev_drv_register(&kb_drv);
    // Create a group for interactive objects
    group_interact = lv_group_create();
    lv_indev_set_group(kb_indev, group_interact);

    // Set the group to the main screen
    // lv_example_img_1();
    backToMenu(nullptr);
}

#include "new_dd_tricks.c"
void lv_example_img_1(void)
{
    LV_IMG_DECLARE(new_dd_tricks);
    // Create a full-screen image object
    lv_obj_t *bg_img = lv_img_create(lv_scr_act());
    lv_img_set_src(bg_img, &new_dd_tricks);
    lv_obj_set_size(bg_img, LV_HOR_RES, LV_VER_RES); // Stretch to screen size if needed
    lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);
    // Send the image to the background
    // lv_obj_move_background(bg_img);
}
