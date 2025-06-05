#include "notificationBar.h"

/* Drivers */
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/time/LocalTime.h>

#include "helpers.h"

// Notification bar container objects
static lv_obj_t *notification_cont;
static lv_obj_t *battery_bar;
static lv_obj_t *battery_label;
static lv_obj_t *WIFI_label;
static lv_obj_t *BT_label;
static lv_obj_t *SD_label;
static lv_obj_t *Charging_label;
static lv_obj_t *time_label;

static lv_timer_t *time_updater = nullptr; // Timer for updating the time label

void updateTimeLabel(lv_timer_t *timer)
{
    if (time_label)
    {
        lv_label_set_text(time_label, local_time.getTime());
    }
}

/// @brief Notification bar for the top of the screen or as a title bar for the app
/// @param parent the parent object to attach the notification bar to
/// @param isHidden if true, show the battery, Wi-Fi, bluetooth, sd card, charging, and time status
/// if false, show the text passed in the text parameter
/// @param text for showing the text in the notification bar like the app name etc.
void notificationBar(lv_obj_t *parent, const bool isHidden, const char *text)
{
    // notification bar
    lv_obj_t *notification_cont = lv_obj_create(parent); // Create on an active screen
    lv_obj_set_width(notification_cont, 300);
    lv_obj_set_height(notification_cont, 35);
    lv_obj_set_x(notification_cont, 3);
    lv_obj_set_y(notification_cont, -100);
    lv_obj_set_align(notification_cont, LV_ALIGN_CENTER);
    lv_obj_clear_flag(notification_cont, LV_OBJ_FLAG_SCROLLABLE);

    if (isHidden)
    {
        // Create the battery bar
        battery_bar = lv_bar_create(notification_cont);
        lv_bar_set_value(battery_bar, 25, LV_ANIM_OFF);      // Set initial value for battery bar
        lv_bar_set_start_value(battery_bar, 0, LV_ANIM_OFF); // Set a starting value

        lv_obj_set_width(battery_bar, 50);                  // Set width of battery bar
        lv_obj_set_height(battery_bar, 15);                 // Set height of battery bar
        lv_obj_align(battery_bar, LV_ALIGN_LEFT_MID, 0, 0); // Align the battery bar to the left of the container

        // Define and apply style
        lv_style_t style;
        lv_style_init(&style);
        lv_style_set_bg_color(&style, lv_color_hex(0x0CC62D));     // Set background color
        lv_style_set_border_color(&style, LV_COLOR_MAKE(0, 0, 0)); // Set border color
        lv_obj_add_style(battery_bar, &style, LV_PART_MAIN);       // Corrected call to add style to the bar

        // SD Card Status
        SD_label = lv_label_create(notification_cont);
        lv_label_set_text(SD_label, LV_SYMBOL_SD_CARD);
        lv_obj_align_to(SD_label, battery_bar, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 15px offset from battery bar

        // if (!sd_card.is_card_mounted())
        //{
        //     lv_obj_add_flag(SD_label, LV_OBJ_FLAG_HIDDEN);
        // }
        // else
        //{
        //     lv_obj_clear_flag(SD_label, LV_OBJ_FLAG_HIDDEN);
        // }

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
        time_label = lv_label_create(notification_cont);

        // lv_label_set_text(time_label, local_time.getTime()); // Set the time string to the label

        if (!time_updater)
        {
            time_updater = lv_timer_create(updateTimeLabel, 1000, nullptr); // Create a timer to update the time label every second
            //
            updateTimeLabel(nullptr);
        }

        lv_obj_align_to(time_label, Charging_label, LV_ALIGN_OUT_RIGHT_MID, 50, 0); // 30 px offset from Charging label

        // Optional: Adjust the height of the labels if necessary
        lv_obj_set_height(SD_label, 20);
        lv_obj_set_height(WIFI_label, 20);
        lv_obj_set_height(BT_label, 20);
        lv_obj_set_height(Charging_label, 20);
        lv_obj_set_height(time_label, 20);
    }
    else
    {
        // create a label for the app name
        lv_obj_t *label = lv_label_create(notification_cont);
        lv_label_set_text(label, text);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }
}

void notificationBar(lv_obj_t *parent, const char *text, void (*event_handler)(lv_event_t *))
{
    LV_LOG_USER("Creating notification bar...");

    // Create the notification container
    lv_obj_t *notification_cont = lv_obj_create(parent);
    lv_obj_set_width(notification_cont, LV_HOR_RES); // Fix width for screen
    lv_obj_set_height(notification_cont, 35);
    lv_obj_align(notification_cont, LV_ALIGN_TOP_MID, 0, 0); // Align properly

    // Create a label for the app name
    lv_obj_t *label = lv_label_create(notification_cont);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Create a back button
    lv_obj_t *back_btn = lv_btn_create(notification_cont);
    lv_obj_set_size(back_btn, 30, 30);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 10, 0); // Fixed alignment
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);

    // Only add event handlers if valid
    if (event_handler != nullptr)
    {
        lv_obj_add_event_cb(notification_cont, event_handler, LV_EVENT_CLICKED, nullptr);
        lv_obj_add_event_cb(back_btn, event_handler, LV_EVENT_CLICKED, nullptr);
    }

    // make it unscrollable
    lv_obj_clear_flag(notification_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_SCROLLABLE);

    // MAKE IT FOCUSABLE
    lv_group_add_obj(group_interact, notification_cont);

    LV_LOG_USER("Notification bar created!");
}
