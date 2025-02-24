#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/battery/battery.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>
#include <components/drivers/vb/vibration.h>
#include <components/boot/sin_table.h>

#include <lvgl.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;
bool boot_screen_ani = true;
static const char *TAG = "Main";
#define DEBOUNCE_DELAY 50 // Adjust as needed (in milliseconds)

#define LV_TICK_PERIOD_MS 10
#define DISP_BUF_SIZE 240 * 40 // width * 2

static SemaphoreHandle_t xGuiSemaphore;
static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;
lv_obj_t *heart_label;

// Group of interactive objects
lv_indev_drv_t kb_drv;
lv_indev_t *kb_indev;
lv_group_t *group_interact;

//  icons
extern const lv_img_dsc_t ext_application_icon;

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
void settings(lv_obj_t *parent);
static void settings_cb(lv_event_t *e);
static void settings_menu(lv_obj_t *parent);
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
void UI_ELEMENTS()
{
    // Create UI elements
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label = lv_label_create(btn);
    // set color of button
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(label, "Hello LVGL!");
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
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
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

    // Time Label
    lv_obj_t *time_label = lv_label_create(notification_cont);
    lv_label_set_text(time_label, "12:00AM");
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
}

void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_key_t lastKey = 0;
    static bool keyPressed = false;
    static unsigned long lastPressTime = 0; // Debouncing variable

    data->state = LV_INDEV_STATE_REL; // Default state: Released

    unsigned long currentTime = millis();

    if (digitalRead(18) == HIGH)
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

static void external_app_cb(lv_event_t *e)
{
    Serial.println("External App Clicked");
}

static void apps_cb(lv_event_t *e)
{
    Serial.println("Apps Clicked _");
}

static void settings_cb(lv_event_t *e)
{
    Serial.println("Settings Clicked");
}
void buttons_init()
{
    pinMode(18, INPUT_PULLDOWN);
    pinMode(17, INPUT_PULLDOWN);
    pinMode(16, INPUT_PULLDOWN);
    pinMode(15, INPUT_PULLDOWN);
    pinMode(7, INPUT_PULLDOWN);
    pinMode(6, INPUT_PULLDOWN);
}
void setup()
{
    Serial.begin(115200);
    buttons_init();

    /**************** Basic initialization **************/
    sys_manager.system_init_config();

    led_notification.LED_init();
    led_notification.LED_mode(LED_FADE_ON);

    sys_manager.system_info();
    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM), sys_manager.system_memory(MEMORY_INTERNAL), sys_manager.system_memory(MEMORY_DMA));

    backlight.backlight_init();

    // vibration.vibration_init();  // fix continuous vibration


    


    ui_init();

    xTaskCreatePinnedToCore(GUI_task, "GUI_task", 4096, NULL, 1, &gui_handler, 1);
    GUI_frontend();

    backlight.backlight_set(50);

    // UI_ELEMENTS();

    // sd_card.sd_init();
    //  user_input.input_init();
    //  batteryQueue = xQueueCreate(1, sizeof(struct BATTERY_STATUS));
    //  battery.battery_init();
}

void loop()
{
}
