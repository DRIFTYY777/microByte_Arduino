#include "user_input.h"
#include <esp32-hal-log.h>
#include <components/system_config/system_manager.h>
#include <components/system_config/system_config.h>
#include <components/ui/helpers.h>

#include "TCA9555.h"

#define DEBOUNCE_DELAY 60 // Adjust as needed (in milliseconds)

static const char *TAG = "user_input";

TCA9555 TCA(TCA_dev_address);

void UserInput::input_init(void)
{
    Wire.begin(I2C_SDA, I2C_SCL, TCA_CLK_SPEED);
    TCA.begin();
    Wire.setClock(TCA_CLK_SPEED);
    // GND___________---__________IOEXPANDER
}

void UserInput::user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static unsigned long lastPressTime[8] = {0}; // Debounce for each button
    static bool buttonPressed[8] = {false};
    unsigned long currentTime = millis();

    uint16_t inputs_value = TCA.read16(); // Read the inputs from the hardware

    data->state = LV_INDEV_STATE_REL; // Default state: Released

    // Up button (index 0)
    if (!((inputs_value >> 0) & 0x01))
    {
        if (!buttonPressed[0] && (currentTime - lastPressTime[0] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "Up Pressed");
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
            ESP_LOGI(TAG, "Down Pressed");
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
    if (!((inputs_value >> 6) & 0x01))
    {
        if (!buttonPressed[2] && (currentTime - lastPressTime[2] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "Back Pressed");
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
    if (!((inputs_value >> 7) & 0x01))
    {
        if (!buttonPressed[3] && (currentTime - lastPressTime[3] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "Enter Pressed");
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
    if (!((inputs_value >> 3) & 0x01))
    {
        if (!buttonPressed[4] && (currentTime - lastPressTime[4] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "Left Pressed");
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

    if (!((inputs_value >> 2) & 0x01))
    {
        if (!buttonPressed[5] && (currentTime - lastPressTime[5] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "Right Pressed");
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
    if (!((inputs_value >> 5) & 0x01))
    {
        if (!buttonPressed[6] && (currentTime - lastPressTime[6] > DEBOUNCE_DELAY))
        {
            ESP_LOGI(TAG, "A Pressed");
            // data->state = LV_INDEV_STATE_PR;
            lastPressTime[6] = currentTime;
            buttonPressed[6] = true;
        }
    }
    else
    {
        buttonPressed[6] = false;
    }
}

uint16_t UserInput::input_read(void)
{
    return TCA.read16();
}

UserInput user_input;
