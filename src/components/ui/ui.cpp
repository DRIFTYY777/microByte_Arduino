#include "ui.h"
#include "helpers.h"

// driver
#include <components/drivers/backlight/backlight.h>
// #include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
// #include <components/drivers/sound/sound.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/time.h>

// emulators
#include <components/emulators/EmulatorUI.h>

// internal apps
#include <components/Apps/EvilApple/ui/evilAppleUI.h>

#include <components/external_app/externalAppUI.h>

#include "settings.h"

/*
    LVGL Version: 8.3.9
*/

/*
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
*/

/*
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

#define DEBOUNCE_DELAY 60 // Adjust as needed (in milliseconds)
#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE (240 * 20) // width * 2

static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;
static SemaphoreHandle_t xGuiSemaphore;

static void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

void MainMenuList(lv_obj_t *parent);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void ui_init()
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    lv_init();

    int32_t size_in_px = DISP_BUF_SIZE;
    static lv_disp_draw_buf_t draw_buf;

    static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px); // Reduced buffer size for non-PSRAM boards

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
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
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Adjust tick rate as needed
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
    kb_drv.read_cb = user_input_task;
    kb_indev = lv_indev_drv_register(&kb_drv);
    // Create a group for interactive objects
    group_interact = lv_group_create();
    lv_indev_set_group(kb_indev, group_interact);
    // Main screen
    MainMenuList(lv_scr_act());
}

void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static unsigned long lastPressTime[8] = {0}; // Debounce for each button
    static bool buttonPressed[8] = {false};
    unsigned long currentTime = millis();
    uint16_t inputs_value = user_input.input_read();
    data->state = LV_INDEV_STATE_REL; // Default state: Released

    // Up button (index 0)
    if (!((inputs_value >> 0) & 0x01))
    {
        if (!buttonPressed[0] && (currentTime - lastPressTime[0] > DEBOUNCE_DELAY))
        {
            Serial.println("Up Pressed");
            data->key = LV_KEY_UP;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[0] = currentTime;
            buttonPressed[0] = true;
            lv_group_focus_prev(group_interact);
        }
    }
    else
    {
        buttonPressed[0] = false;
    }

    // Down button (index 1)
    if (!((inputs_value >> 1) & 0x01))
    {
        if (!buttonPressed[1] && (currentTime - lastPressTime[1] > DEBOUNCE_DELAY))
        {
            Serial.println("Down Pressed");
            data->key = LV_KEY_DOWN;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[1] = currentTime;
            buttonPressed[1] = true;
            lv_group_focus_next(group_interact);
        }
    }
    else
    {
        buttonPressed[1] = false;
    }

    // Back button (index 6)
    if (!((inputs_value >> 6) & 0x01))
    {
        if (!buttonPressed[2] && (currentTime - lastPressTime[2] > DEBOUNCE_DELAY))
        {
            Serial.println("Back Pressed");
            data->key = LV_KEY_ESC;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[2] = currentTime;
            buttonPressed[2] = true;
        }
    }
    else
    {
        buttonPressed[2] = false;
    }

    // Enter button (index 7)
    if (!((inputs_value >> 7) & 0x01))
    {
        if (!buttonPressed[3] && (currentTime - lastPressTime[3] > DEBOUNCE_DELAY))
        {
            Serial.println("OK Pressed");
            data->key = LV_KEY_ENTER;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[3] = currentTime;
            buttonPressed[3] = true;
        }
    }
    else
    {
        buttonPressed[3] = false;
    }

    // Left button (index 3)
    if (!((inputs_value >> 3) & 0x01))
    {
        if (!buttonPressed[4] && (currentTime - lastPressTime[4] > DEBOUNCE_DELAY))
        {
            Serial.println("Left Pressed");
            data->key = LV_KEY_LEFT;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[4] = currentTime;
            buttonPressed[4] = true;
        }
    }
    else
    {
        buttonPressed[4] = false;
    }

    // Right button (index 2)
    if (!((inputs_value >> 2) & 0x01))
    {
        if (!buttonPressed[5] && (currentTime - lastPressTime[5] > DEBOUNCE_DELAY))
        {
            Serial.println("Right Pressed");
            data->key = LV_KEY_RIGHT;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime[5] = currentTime;
            buttonPressed[5] = true;
        }
    }
    else
    {
        buttonPressed[5] = false;
    }
    // Menu button. index 11
    if (!((inputs_value >> 5) & 0x01))
    {
        if (!buttonPressed[6] && (currentTime - lastPressTime[6] > DEBOUNCE_DELAY))
        {
            Serial.println("Menu pressed");
            // data->state = LV_INDEV_STATE_PR;
            lastPressTime[6] = currentTime;
            buttonPressed[6] = true;

            if (!isInMenu)
            {
                app.mode = MODE_NONE;
                app.status = STATUS_STOPPED;

                lv_obj_clean(lv_scr_act());
                MainMenuList(lv_scr_act());
            }
        }
    }
    else
    {
        buttonPressed[6] = false;
    }
}

static void SettingsEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(20); // preventing sudden crash
        // clear this screen and goes to the settings screen
        lv_obj_clean(lv_scr_act());
        createSettingScreen(lv_scr_act(), e);
    }
}

static void ExternalAppEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(20); // preventing sudden crash
        lv_obj_clean(lv_scr_act());
        createExternalAppScreen(lv_scr_act(), e);
    }
}

static void EvilAppleEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(20); // preventing sudden crash
        // clear this screen and goes to the Evil Apple screen
        lv_obj_clean(lv_scr_act());
        createEVIL_APPLEScreen(lv_scr_act(), e);
    }
}

static void FirmwareEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(20); // preventing sudden crash
        // print log
        Serial.println("Firmware Clicked");
    }
}
static void EmulatorEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        delay(20); // preventing sudden crash
        lv_obj_clean(lv_scr_act());
        // createEmulatorScreen(lv_scr_act(), e);
    }
}

void MainMenuList(lv_obj_t *parent)
{
    isInMenu = true;
    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(parent);
    lv_obj_set_size(Menu, 300, 180);

    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    /*Add buttons to the list*/
    lv_obj_t *btn;

    btn = lv_list_add_btn(Menu, LV_SYMBOL_FILE, "Evil Apple");
    lv_obj_add_event_cb(btn, EvilAppleEventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_PLAY, "Emulator");
    lv_obj_add_event_cb(btn, EmulatorEventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_DIRECTORY, "External App");
    lv_obj_add_event_cb(btn, ExternalAppEventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "Update Firmware");
    lv_obj_add_event_cb(btn, FirmwareEventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "Settings");
    lv_obj_add_event_cb(btn, SettingsEventHandler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    lv_obj_t *list1 = lv_obj_get_child(lv_scr_act(), 1);
    if (list1)
    {
        lv_obj_t *first_button = lv_obj_get_child(list1, 0);
        if (first_button)
        {
            lv_group_focus_obj(first_button);
        }
    }
}
