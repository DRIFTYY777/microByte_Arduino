#include "ui.h"
#include "lvgl.h"

// driver
#include <components/drivers/backlight/backlight.h>
#include <components/drivers/battery/battery.h>
// #include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
// #include <components/drivers/sound/sound.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/time.h>

/*
    LVGL Version: 8.3.9
*/

#define DEBOUNCE_DELAY 50 // Adjust as needed (in milliseconds)
#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE 240 * 25 // width * 2

// Create a semaphore
bool sub_menu = false;

static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;

// Group of interactive objects
lv_indev_drv_t kb_drv;
lv_indev_t *kb_indev;
lv_group_t *group_interact;

// Notification bar container objects
static lv_obj_t *notification_cont;
static lv_obj_t *battery_bar;
static lv_obj_t *battery_label;
static lv_obj_t *WIFI_label;
static lv_obj_t *BT_label;
static lv_obj_t *SD_label;
static lv_obj_t *Charging_label;

// Main menu objects
static lv_obj_t *tab_main_menu;
static lv_obj_t *tab_ext_app_manager;
static lv_obj_t *tab_bt_controller;
static lv_obj_t *tab_config;

// External app menu objects
static lv_obj_t *btn_ext_app;
static lv_obj_t *list_external_app;

// Configuration menu objects
static lv_obj_t *config_btn;
static lv_obj_t *list_config;
static lv_obj_t *list_fw_update;
static lv_obj_t *mbox_about;
static lv_obj_t *mbox_color;

//
static lv_obj_t *label;

static void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
// External app menu
void external_app_menu(lv_obj_t *parent);
static void external_app_cb(lv_event_t *e);
static void app_execute_cb(lv_obj_t *parent, lv_event_t e);
// internal app menu
void apps_menu(lv_obj_t *parent);
static void apps_cb(lv_event_t *e);
static void app_execute_cb(lv_obj_t *parent, lv_event_t e);

// settings_cb
static void settings(lv_event_t *e);
static void settings_menu(lv_event_t *e);
static void mbox_config_cb(lv_obj_t *parent, lv_event_t e);
static void fw_update_cb(lv_obj_t *parent, lv_event_t e);

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
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    lv_init();
    static EXT_RAM_ATTR lv_color_t *buf1[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t draw_buf;
    int32_t size_in_px = DISP_BUF_SIZE;
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, size_in_px); // Reduced buffer size for non-PSRAM boards
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
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10)); // Add a delay to yield CPU time
    }
    vTaskDelete(NULL);
}

static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object
    if (code == LV_EVENT_CLICKED)
    {
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
        Serial.println("Clicked  awddwadawd");
    }
}

void lv_example_list_1(lv_obj_t *parent)
{
    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(parent);
    lv_obj_set_size(Menu, 300, 180);

    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    /*Add buttons to the list*/
    lv_obj_t *btn;

    btn = lv_list_add_btn(Menu, LV_SYMBOL_FILE, "Evil Apple");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_DIRECTORY, "External App");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SAVE, "Settings");
    lv_obj_add_event_cb(btn, settings, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(group_interact, btn);
}

void notificationBar(lv_obj_t *parent)
{

    /* Device State Bar */
    // This bar shows the battery status, if the SD card is attached
    // or if any wireless communication is active.
    notification_cont = lv_obj_create(lv_scr_act()); // Create on active screen
    lv_obj_set_width(notification_cont, 300);
    lv_obj_set_height(notification_cont, 35);
    lv_obj_set_x(notification_cont, 3);
    lv_obj_set_y(notification_cont, -100);
    lv_obj_set_align(notification_cont, LV_ALIGN_CENTER);
    lv_obj_clear_flag(notification_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Create the battery bar
    battery_bar = lv_bar_create(notification_cont);
    lv_bar_set_value(battery_bar, 25, LV_ANIM_OFF);      // Set initial value for battery bar
    lv_bar_set_start_value(battery_bar, 0, LV_ANIM_OFF); // Set starting value

    lv_obj_set_width(battery_bar, 50);                  // Set width of battery bar
    lv_obj_set_height(battery_bar, 15);                 // Set height of battery bar
    lv_obj_align(battery_bar, LV_ALIGN_LEFT_MID, 0, 0); // Align battery bar to the left of the container

    //// Define and apply style
    lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_hex(0x0CC62D));     // Set background color
    lv_style_set_border_color(&style, LV_COLOR_MAKE(0, 0, 0)); // Set border color
    lv_obj_add_style(battery_bar, &style, LV_PART_MAIN);       // Corrected call to add style to the bar

    // SD Card Status
    SD_label = lv_label_create(notification_cont);
    lv_label_set_text(SD_label, LV_SYMBOL_SD_CARD);
    lv_obj_align_to(SD_label, battery_bar, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 15px offset from battery bar

    // Wi-Fi Status
    WIFI_label = lv_label_create(notification_cont);
    lv_label_set_text(WIFI_label, LV_SYMBOL_WIFI);
    lv_obj_align_to(WIFI_label, SD_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 15px offset from SD card label

    // Bluetooth Status
    BT_label = lv_label_create(notification_cont);
    lv_label_set_text(BT_label, LV_SYMBOL_BLUETOOTH);
    lv_obj_align_to(BT_label, WIFI_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 15px offset from Wi-Fi label

    // Charging Status
    Charging_label = lv_label_create(notification_cont);
    lv_label_set_text(Charging_label, LV_SYMBOL_CHARGE);
    lv_obj_align_to(Charging_label, BT_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 15px offset from Bluetooth label

    // Time Label,   get time from time.h

    lv_obj_t *time_label = lv_label_create(notification_cont);
    lv_label_set_text(time_label, "ww");
    lv_obj_align_to(time_label, Charging_label, LV_ALIGN_OUT_RIGHT_MID, 50, 0); // 30px offset from Charging label

    // Optional: Adjust the height of the labels if necessary
    lv_obj_set_height(SD_label, 20);
    lv_obj_set_height(WIFI_label, 20);
    lv_obj_set_height(BT_label, 20);
    lv_obj_set_height(Charging_label, 20);
    lv_obj_set_height(time_label, 20);
}

void GUI_frontend()
{
    // Create a group for interactive objects
    lv_indev_drv_init(&kb_drv);
    kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    kb_drv.read_cb = user_input_task;
    kb_indev = lv_indev_drv_register(&kb_drv);
    // Create a notification bar
    notificationBar(lv_scr_act());

    // Create a group for interactive objects
    group_interact = lv_group_create();
    lv_indev_set_group(kb_indev, group_interact);

    lv_example_list_1(lv_scr_act());
    lv_obj_t *list1 = lv_obj_get_child(lv_scr_act(), 1);
    if (list1)
    {
        lv_obj_t *first_button = lv_obj_get_child(list1, 0);
        if (first_button)
        {
            lv_group_focus_obj(first_button);
        }
    }
    // refresh display
    // lv_task_handler();
}

void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_key_t lastKey = 0;
    static bool keyPressed = false;
    static unsigned long lastPressTime = 0; // Debouncing variable

    data->state = LV_INDEV_STATE_REL; // Default state: Released

    unsigned long currentTime = millis();

    if (digitalRead(5) == HIGH)
    { // Up button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("Up Pressed");
            data->key = LV_KEY_UP;
            data->state = LV_INDEV_STATE_PR;
            lv_group_focus_prev(group_interact); // Move focus to previous object
            lastPressTime = currentTime;
        }
    }
    else if (digitalRead(17) == HIGH)
    { // Down button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("Down Pressed");
            data->key = LV_KEY_DOWN;
            data->state = LV_INDEV_STATE_PR;
            lv_group_focus_next(group_interact); // Move focus to next object
            lastPressTime = currentTime;
        }
    }
    else if (digitalRead(16) == HIGH)
    { // Back button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("Back Pressed");
            data->key = LV_KEY_ESC;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime = currentTime;
        }
    }
    else if (digitalRead(15) == HIGH)
    { // OK button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("OK Pressed");
            data->key = LV_KEY_ENTER;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime = currentTime;
        }
    }
    else if (digitalRead(7) == HIGH)
    { // Left button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("Left Pressed");
            data->key = LV_KEY_LEFT;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime = currentTime;
        }
    }
    else if (digitalRead(6) == HIGH)
    { // Right button
        if (currentTime - lastPressTime > DEBOUNCE_DELAY)
        {
            Serial.println("Right Pressed");
            data->key = LV_KEY_RIGHT;
            data->state = LV_INDEV_STATE_PR;
            lastPressTime = currentTime;
        }
    }
}

void settings(lv_event_t *e)
{
    sub_menu = true;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *list1 = lv_obj_get_parent(obj); // Get the list object

    if (code == LV_EVENT_CLICKED)
    {
        /*Create a list on parent*/
        lv_obj_t *Menu = lv_list_create(lv_scr_act());
        lv_obj_set_size(Menu, 300, 180);
        lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

        /*Add buttons to the list*/
        lv_obj_t *btn;

        btn = lv_list_add_btn(Menu, LV_SYMBOL_BARS, "About this device");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        /*System config options */
        btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "Brightness");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_VOLUME_MAX, "Volume");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "Vibration");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_BATTERY_FULL, "Battery");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_WIFI, "Wi-Fi");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_BLUETOOTH, "Bluetooth");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        btn = lv_list_add_btn(Menu, LV_SYMBOL_SD_CARD, "SD Card");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);

        /* Add back button*/
        btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
        lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group_interact, btn);
    }
}

static void mbox_config_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CANCEL || code == LV_EVENT_CLICKED)
    {
        char *aux_text = (char *)lv_obj_get_user_data(obj); // Retrieve aux_text
        if (aux_text != NULL)
        {
            free(aux_text); // Free aux_text
        }
        lv_obj_del(obj);
    }
}

static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);

    /*Refresh the text*/
    lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}

void settings_menu(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    // Get the button text
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    if (strcmp(btn_text, "About this device") == 0)
    {
        Serial.println("About this device");
    }
    else if (strcmp(btn_text, "Brightness") == 0)
    {
        // Clear the screen to remove the settings menu
        lv_obj_clean(lv_scr_act());

        // Create a window object to place brightness slider on it
        lv_obj_t *window = lv_obj_create(lv_scr_act());
        lv_obj_set_size(window, 300, 180);
        lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
        lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

        // Create a slider
        lv_obj_t *slider = lv_slider_create(window);
        lv_obj_set_width(slider, 200);
        lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

        // Create a label
        label = lv_label_create(window);
        lv_label_set_text(label, "0");
        lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);

        // Set the slider's value
        lv_slider_set_value(slider, 50, LV_ANIM_OFF);

        // Make the slider focusable and user input enabled
        lv_obj_set_user_data(slider, (void *)"Brightness");
        lv_group_add_obj(group_interact, slider);
        lv_group_focus_obj(slider);

        // Add a back button
        lv_obj_t *back_btn = lv_btn_create(window);
        lv_obj_set_size(back_btn, 80, 30);
        lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_t *back_label = lv_label_create(back_btn);
        lv_label_set_text(back_label, "Back");
        lv_obj_center(back_label);

        // Add back button event
        lv_obj_add_event_cb(back_btn, [](lv_event_t *e)
                            {
            lv_obj_clean(lv_scr_act());
            lv_example_list_1(lv_scr_act());
            lv_obj_t *list1 = lv_obj_get_child(lv_scr_act(), 1);
            if (list1)
            {
                lv_obj_t *first_button = lv_obj_get_child(list1, 0);
                if (first_button)
                {
                    lv_group_focus_obj(first_button);
                }
            } }, LV_EVENT_CLICKED, NULL);

        lv_group_add_obj(group_interact, back_btn);
    }
}
