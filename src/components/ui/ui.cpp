#include "ui.h"

// UI Components
#include "helpers.h"
#include "mainScreen.h"

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
static SemaphoreHandle_t xGuiSemaphore;

void set_custom_theme()
{
    static lv_theme_t *theme;

    /* Custom primary and secondary colors */
    lv_color_t primary = lv_color_make(255, 0, 0);   // Red
    lv_color_t secondary = lv_color_make(0, 255, 0); // Green

    /* Set the new theme */
    theme = lv_theme_default_init(
        lv_disp_get_default(),                   // Display
        lv_palette_main(LV_PALETTE_DEEP_PURPLE), // Primary color
        lv_palette_main(LV_PALETTE_BLUE),        // Secondary color
        LV_THEME_DEFAULT_DARK,                   // Dark or Light mode
        &lv_font_montserrat_14                   // Default font
    );

    /* Apply the theme */
    lv_disp_set_theme(lv_disp_get_default(), theme);
}

void ui_init()
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    int32_t size_in_px = DISP_BUF_SIZE;
    static lv_disp_draw_buf_t draw_buf;

    // buffer in psram
    static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

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

    lv_disp_draw_buf_init(&draw_buf, buf2, NULL, size_in_px); // Reduced buffer size for non-PSRAM boards

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320; // height
    disp_drv.ver_res = 240;

    disp_drv.flush_cb = display_HAL_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Create timer for LVGL system ticks
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
}

static lv_obj_t *fps_label;
static lv_obj_t *cpu_label;
static uint32_t frame_count = 0;
static void update_fps_and_cpu(lv_timer_t *timer)
{
    // Calculate FPS
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", frame_count);
    // ESP_LOGI("UI", "%s", fps_text); // Print FPS to serial
    Serial.println(fps_text); // Print FPS to serial
    frame_count = 0;          // Reset frame count

    // Calculate CPU usage (example using FreeRTOS heap info)
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    int cpu_usage = 100 - ((free_heap * 100) / total_heap);

    char cpu_text[32];
    snprintf(cpu_text, sizeof(cpu_text), "CPU: %d%%", cpu_usage);
    // ESP_LOGI("UI", "%s", cpu_text); // Print CPU usage to serial
    Serial.println(cpu_text); // Print CPU usage to serial
}

void GUI_task(void *arg)
{
    while (1)
    {
        if (xGuiSemaphore != NULL)
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
            {
                lv_task_handler();
                xSemaphoreGive(xGuiSemaphore);

                frame_count++; // Increment frame count
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust tick rate as needed
    }
    vTaskDelete(NULL);
}

static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void GUI_frontend()
{
    // Create a group for interactive objects
    lv_indev_drv_init(&kb_drv);
    kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    kb_drv.read_cb = user_input.user_input_task;
    kb_indev = lv_indev_drv_register(&kb_drv);
    // Create a group for interactive objects
    group_interact = lv_group_create();
    lv_indev_set_group(kb_indev, group_interact);
    // Create a group for non-interactive objects

    // lv_timer_create(update_fps_and_cpu, 1000, NULL);
    backToMenu(NULL);
}
