#include "settings.h"

#include <Arduino.h>
#include <WiFi.h>

/* UI */
#include "notificationBar.h"
#include "helpers.h"

/* Drivers */
#include <components/system_config/system_manager.h>
#include <components/drivers/backlight/backlight.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sd_card/formatSD.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/time/LocalTime.h>

/*
                  W 300
        |-----------------------|
   H183 |________Center_________|
        |          0            |
    x   |         150           |
        |                       |
        |                       |
        |_______________________|
                    Y
*/

static lv_obj_t *brightness_label = nullptr;
static lv_obj_t *brightness_slider = nullptr;


void backButtonEventHandler(lv_event_t *e)
{
    clear_group_focus();
    delay(50);
    // lv_obj_clean(lv_scr_act());
    createSettingScreen();
}

void backButton(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_obj_t *parent)
{
    // Create a button
    lv_obj_t *back_btn = lv_btn_create(parent);
    lv_obj_set_size(back_btn, w, h);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, x, y);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, backButtonEventHandler, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, back_btn);
    lv_group_focus_obj(back_btn);
}

void aboutThisDevice()
{
    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(lv_scr_act());
    lv_obj_set_size(Menu, 320, 240);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_ver(Menu, 5, 0); // Add vertical padding to the list.

    sys_manager.system_info();
    // Buffer for formatted strings
    char buffer[200];

    // App Version
    sprintf(buffer, "App Version: %s", app_version);
    lv_obj_t *App_version = lv_label_create(Menu);
    lv_label_set_text(App_version, buffer);

    // IDF Version
    sprintf(buffer, "IDF Version: %s", idf_version);
    lv_obj_t *Idf_version = lv_label_create(Menu);
    lv_label_set_text(Idf_version, buffer);

    // CPU Version
    sprintf(buffer, "CPU Version: %s", cpu_version);
    lv_obj_t *Cpu_version = lv_label_create(Menu);
    lv_label_set_text(Cpu_version, buffer);

    // Internal RAM
    sprintf(buffer, "Internal RAM: %.2f / %.2f MB", (float)Used_Internal_ram / (1024 * 1024), (float)Internal_ram / (1024 * 1024));
    lv_obj_t *InternalRamLabel = lv_label_create(Menu);
    lv_label_set_text(InternalRamLabel, buffer);

    // SPI RAM
    sprintf(buffer, "SPI RAM: %.2f / %.2f MB", (float)Used_SPI_ram / (1024 * 1024), (float)SPI_ram / (1024 * 1024));
    lv_obj_t *SPIRamLabel = lv_label_create(Menu);
    lv_label_set_text(SPIRamLabel, buffer);

    // Flash
    sprintf(buffer, "Flash: %.2f / %.2f MB", (float)Free_Flash / (1024 * 1024), (float)Flash_Size / (1024 * 1024));
    lv_obj_t *FlashLabel = lv_label_create(Menu);
    lv_label_set_text(FlashLabel, buffer);

    // Details of sd card card_mounted
    sprintf(buffer, "SD Card: %s", sd_card_info.card_mounted ? "Mounted" : "Not Mounted");
    lv_obj_t *SdCardLabel = lv_label_create(Menu);
    lv_label_set_text(SdCardLabel, buffer);

    // Details of card_size
    sprintf(buffer, "SD Card Size: %d MB", sd_card_info.card_size);
    lv_obj_t *SdCardSizeLabel = lv_label_create(Menu);
    lv_label_set_text(SdCardSizeLabel, buffer);

    // Details of card_type
    sprintf(buffer, "SD Card Type: %d", sd_card_info.card_type);
    lv_obj_t *SdCardTypeLabel = lv_label_create(Menu);
    lv_label_set_text(SdCardTypeLabel, buffer);

    // free_space
    sprintf(buffer, "Free Space: %d MB", sd_card_info.free_space);
    lv_obj_t *FreeSpaceLabel = lv_label_create(Menu);
    lv_label_set_text(FreeSpaceLabel, buffer);

    // used_space
    sprintf(buffer, "Used Space: %d MB", sd_card_info.used_space);
    lv_obj_t *UsedSpaceLabel = lv_label_create(Menu);
    lv_label_set_text(UsedSpaceLabel, buffer);

    // Refresh the list to apply the padding.
    lv_obj_refresh_ext_draw_size(Menu);

    // Add list scroll bar
    lv_obj_add_flag(Menu, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_snap_y(Menu, LV_SCROLL_SNAP_CENTER); // snap to the center of the elements.

    // Add a back button
    backButton(0, -10, 80, 30, lv_scr_act());

    // CLEAR THE char buffer[200]; FREE TEH RAM
    memset(buffer, 0, sizeof(buffer));
}

void brightness_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int brightness = lv_slider_get_value(slider);
    // Prevent brightness from going below 10
    if (brightness < 10)
    {
        brightness = 10;
        lv_slider_set_value(slider, brightness, LV_ANIM_OFF); // Force update
    }
    // Convert brightness value to string
    char brightnessStr[4];
    sprintf(brightnessStr, "%d", brightness);
    // Update the label
    if (brightness_label)
        lv_label_set_text(brightness_label, brightnessStr);
    // Set backlight brightness
    BACKLIGHT::backlight_set(static_cast<uint8_t>(brightness));
}
void brightnessSettings()
{
    // Create a window
    lv_obj_t *window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(window, 300, 180);
    lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar
    notificationBar(lv_scr_act(), false, "Brightness");

    // Get stored brightness level
    int8_t brightness = sys_manager.system_get_config(SYS_BRIGHT);
    if (brightness < 5 || brightness > 100)
        brightness = 50; // Default

    // Create slider
    brightness_slider = lv_slider_create(window);
    lv_obj_set_width(brightness_slider, 200);
    lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(brightness_slider, 0, 100);
    lv_slider_set_value(brightness_slider, brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // Create label
    brightness_label = lv_label_create(window);
    lv_label_set_text_fmt(brightness_label, "%d", brightness);
    lv_obj_align_to(brightness_label, brightness_slider, LV_ALIGN_OUT_TOP_MID, 0, -15);

    // Add to input group
    lv_group_add_obj(group_interact, brightness_slider);
    lv_group_focus_obj(brightness_slider);

    backButton(0, -10, 80, 30, window);
}

/// @brief Vibration Button Event Handler
/// @param e
/// @short Event handler for the vibration button
static void VBEH(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED) == false)
        {
            vibration.vibration_off();
        }
        else
        {
            vibration.vibration_on();
        }
    }
}

void vibrationSettings()
{
    // Create a window
    lv_obj_t *window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(window, 300, 180);
    lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar
    notificationBar(lv_scr_act(), false, "Vibration");

    // Creating a label of the switch vibration on/off
    lv_obj_t *label = lv_label_create(window);
    sys_manager.system_get_config(SYS_VIBRATION) == 1 ? lv_label_set_text(label, "Vibration: On") : lv_label_set_text(label, "Vibration: Off");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20); // Align label to the top center

    // Create a switch
    lv_obj_t *sw = lv_switch_create(window);
    lv_obj_align(sw, LV_ALIGN_CENTER, 0, 0); // Align switch to the center
    lv_group_add_obj(group_interact, sw);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, VBEH, LV_EVENT_VALUE_CHANGED, nullptr);

    // Create a button
    backButton(0, -10, 80, 30, window);
}

/// @brief Wifi Button Event Handler
/// @param e
/// @short Wifi Button Event Handler
void WBEH(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED) == true)
        {
            // wifi.wifi_init();
            WiFi.begin();
        }
        else
        {
            WiFi.disconnect();
            // wifi.wifi_deinit();
        }
    }
}
void wifiSettings()
{
    // title for wifi and on off button
    lv_obj_t *title = lv_obj_create(lv_scr_act());
    lv_obj_set_width(title, 300);
    lv_obj_set_height(title, 35);
    lv_obj_set_x(title, 3);
    lv_obj_set_y(title, -100);
    lv_obj_set_align(title, LV_ALIGN_CENTER);
    lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE);

    // label
    lv_obj_t *label = lv_label_create(title);
    lv_label_set_text(label, "Wi-Fi");
    lv_obj_align(label, LV_ALIGN_CENTER, -120, 0);

    // Create a switch
    lv_obj_t *sw = lv_switch_create(title);
    lv_obj_align(sw, LV_ALIGN_CENTER, 100, 0);
    lv_group_add_obj(group_interact, sw);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, WBEH, LV_EVENT_VALUE_CHANGED, nullptr);

    // create a lsit view
    lv_obj_t *list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, 300, 190);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 40);
}

/// @brief SD Card Event Handler
/// @param e
/// @short SD Card  Event Handler
void SDEH(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);
    if (strcmp(btn_text, "Reset") == 0)
    {
        if (sd_card.sd_default())
        {
            notificationBar(lv_scr_act(), false, "SD Card Reset Done");
        }
        else
        {
            notificationBar(lv_scr_act(), false, "SD Card Reset Failed");
        }
    }
    else if (strcmp(btn_text, "Format") == 0)
    {
        format_sdcard();
    }
}

void sdCardSettings()
{
    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(lv_scr_act());
    lv_obj_set_size(Menu, 320, 240);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_ver(Menu, 5, 0); // Add vertical padding to the list.

    notificationBar(Menu, false, "SD Card");

    // create a list for sd card
    // sd_default
    lv_obj_t *sd_default = lv_list_add_btn(Menu, LV_SYMBOL_OK, "Reset");
    lv_obj_add_event_cb(sd_default, SDEH, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, sd_default);

    // sd_format
    lv_obj_t *sd_format = lv_list_add_btn(Menu, LV_SYMBOL_REFRESH, "Format");
    lv_obj_add_event_cb(sd_format, SDEH, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, sd_format);

    // back
    lv_obj_t *back = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
    lv_obj_add_event_cb(back, backButtonEventHandler, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, back);

    // make first button focused
    lv_group_focus_obj(sd_default);
}

/// @brief Notificaiton Led Event Handler
/// @param e
static void NLEH(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        switch (lv_dropdown_get_selected(obj))
        {
        case 0:
            led_notification.LED_mode(LED_BLINK_HS);
            break;
        case 1:
            led_notification.LED_mode(LED_BLINK_LS);
            break;
        case 2:
            led_notification.LED_mode(LED_TURN_ON);
            break;
        case 3:
            led_notification.LED_mode(LED_TURN_OFF);
            break;
        case 4:
            led_notification.LED_mode(LED_FADE_ON);
            break;
        case 5:
            led_notification.LED_mode(LED_FADE_OFF);
            break;
        default:
                break;
        }
    }
}

void LED()
{
    // Create a window
    lv_obj_t *window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(window, 300, 180);
    lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar
    notificationBar(lv_scr_act(), false, "LED");

    /*Create a normal drop-down list*/
    lv_obj_t *dd = lv_dropdown_create(window);
    lv_dropdown_set_options(dd, "Blink HS\n"
                                "Blink LS\n"
                                "Led On\n"
                                "Led Off\n"
                                "Fade On\n"
                                "Fade Off");

    lv_obj_align(dd, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_add_event_cb(dd, NLEH, LV_EVENT_ALL, nullptr);

    // Add a back button
    backButton(0, -10, 80, 30, window);

    // make it focus
    lv_group_add_obj(group_interact, dd);
}


void dateAndTimeSettings() {
    // Create a window
    lv_obj_t *window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(window, 300, 180);
    lv_obj_align(window, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar
    notificationBar(lv_scr_act(), false, "DATE AND TIME");


    // print date, time, year, day, region etc
    LocalTime local_time;
    char buffer[100];
    sprintf(buffer, "Date: %s\nTime: %s\nYear: %d\nDay: %d\nRegion: %s",
            local_time.getDate(), local_time.getTime(),
            local_time.getYear(), local_time.getDay(),
            local_time.getRegion());
    // Create a label to display the date and time
    lv_obj_t *date_time_label = lv_label_create(window);
    lv_label_set_text(date_time_label, buffer);
    lv_obj_align(date_time_label, LV_ALIGN_LEFT_MID, 0, 20);

    // Create a label to display the year and day
    lv_obj_t *year_day_label = lv_label_create(window);
    sprintf(buffer, "Year: %d\nDay: %d", local_time.getYear(), local_time.getDay());
    lv_label_set_text(year_day_label, buffer);
    lv_obj_align(year_day_label, LV_ALIGN_LEFT_MID, 0, 60);
    // Create a label to display the region
    lv_obj_t *region_label = lv_label_create(window);
    sprintf(buffer, "Region: %s", local_time.getRegion());
    lv_label_set_text(region_label, buffer);
    lv_obj_align(region_label, LV_ALIGN_LEFT_MID, 0, 100);
    // Create a label to display the online time
    lv_obj_t *online_time_label = lv_label_create(window);
    sprintf(buffer, "Online Time: %s", local_time.get_time_online());
    lv_label_set_text(online_time_label, buffer);
    lv_obj_align(online_time_label, LV_ALIGN_LEFT_MID, 0, 140);





    // Add a back button
    backButton(0, -10, 80, 30, window);

    // make it focus
    lv_group_add_obj(group_interact, date_time_label);
}


void settings_menu(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *btn_text = lv_list_get_btn_text(lv_obj_get_parent(obj), obj);

    if (strcmp(btn_text, "About this device") == 0)
    {
        clear_group_focus();
        delay(50);
        aboutThisDevice();
    }
    else if (strcmp(btn_text, "Brightness") == 0)
    {
        clear_group_focus();
        delay(50);
        brightnessSettings();
    }
    else if (strcmp(btn_text, "Vibration") == 0)
    {
        clear_group_focus();
        delay(50);
        vibrationSettings();
    }
    else if (strcmp(btn_text, "LED") == 0) // possible bug
    {
        clear_group_focus();
        delay(50);
        LED();
    }
    else if (strcmp(btn_text, "SD Card") == 0) // possible bug
    {
        clear_group_focus();
        delay(50);
        sdCardSettings();
    }else if (strcmp(btn_text, "Date and Time") == 0)
    {
        clear_group_focus();
        delay(50);
        dateAndTimeSettings();
    }
}

void createSettingScreen()
{
    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(nullptr);
    lv_obj_set_size(new_screen, 320, 240);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);

    // title bar
    notificationBar(new_screen, false, "Settings");

    /*Create a list on parent*/
    lv_obj_t *Menu = lv_list_create(new_screen);
    lv_obj_set_size(Menu, 300, 180);
    lv_obj_align(Menu, LV_ALIGN_CENTER, 0, 20);

    /*Add buttons to the list*/
    lv_obj_t *btn;

    btn = lv_list_add_btn(Menu, LV_SYMBOL_LEFT, "Back");
    lv_obj_add_event_cb(btn, backToMenu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_BARS, "About this device");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    /*System config options */
    btn = lv_list_add_btn(Menu, LV_SYMBOL_VIDEO, "Brightness");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "LED");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_VOLUME_MAX, "Volume");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SETTINGS, "Vibration");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_WIFI, "Wi-Fi");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    btn = lv_list_add_btn(Menu, LV_SYMBOL_SD_CARD, "SD Card");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    // date and time settings
    btn =  lv_list_add_btn(Menu, LV_SYMBOL_EDIT, "Date and Time");
    lv_obj_add_event_cb(btn, settings_menu, LV_EVENT_CLICKED, nullptr);
    lv_group_add_obj(group_interact, btn);

    // Safely switch to the new screen
    lv_scr_load(new_screen);
}
