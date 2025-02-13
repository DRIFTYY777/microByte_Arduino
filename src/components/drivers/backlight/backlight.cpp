#include "backlight.h"
#include <driver/ledc.h>

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

ledc_channel_config_t backlight_led;

void BACKLIGHT::backlight_init()
{
    // Configure LEDC Timer
    ledc_timer_config_t ledc0_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // Use LOW_SPEED_MODE for ESP32-S3
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 8000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc0_timer);

    // Configure LEDC Channel
    ledc_channel_config_t backlight_led = {
        .gpio_num = (gpio_num_t)HSPI_BACKLIGTH, // Cast to gpio_num_t
        .speed_mode = LEDC_LOW_SPEED_MODE,      // ESP32-S3 only supports LOW_SPEED_MODE
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0, // Start with 0 brightness
        .hpoint = 0,
    };
    ledc_channel_config(&backlight_led);

    // Enable LEDC Fade
    ledc_fade_func_install(0);

    // Test fading effect (increase brightness over 2 sec)
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 8000, 2000);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, LEDC_FADE_NO_WAIT);
}

void BACKLIGHT::backlight_set(uint8_t level)
{
    if (level < 1 || level > 100)
        return;

    uint16_t backligth_level = (8000 * level) / 100; // Calcute the equivalent duty cycle

    // Set the change duty cycle to be done in 100 mS.
    ledc_set_fade_with_time(backlight_led.speed_mode, backlight_led.channel, backligth_level, 100);
    ledc_fade_start(backlight_led.speed_mode, backlight_led.channel, LEDC_FADE_NO_WAIT);
    sys_manager.system_save_config(SYS_BRIGHT, level);
}

uint8_t BACKLIGHT::backlight_get()
{
    return sys_manager.system_get_config(SYS_BRIGHT);
}

BACKLIGHT backlight;
