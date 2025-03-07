#include "backlight.h"
#include <driver/ledc.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

ledc_channel_config_t backlight_led;

void BACKLIGHT::backlight_init()
{
    // Configure LEDC Timer
    ledc_timer_config_t ledc0_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 8000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc0_timer);

    // Configure LEDC Channel (Initialize the global variable)
    backlight_led.gpio_num = (gpio_num_t)DSP_BACKLIGTH;
    backlight_led.speed_mode = LEDC_LOW_SPEED_MODE;
    backlight_led.channel = LEDC_CHANNEL_1;
    backlight_led.timer_sel = LEDC_TIMER_1;
    backlight_led.duty = 0; // Start with 0 brightness
    backlight_led.hpoint = 0;

    ledc_channel_config(&backlight_led);

    // Enable LEDC Fade
    ledc_fade_func_install(0);

    // Test fading effect (increase brightness over 2 sec)
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 8191, 2000); // use 8191 for maximum duty.
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, LEDC_FADE_NO_WAIT);

    // return sys_manager.system_get_config(SYS_BRIGHT);
    //if (sys_manager.system_get_config(SYS_BRIGHT) == 0)
    //{
    //    sys_manager.system_save_config(SYS_BRIGHT, 50);
    //    backlight_set(50);
    //}
}

void BACKLIGHT::backlight_set(uint8_t level)
{
    if (level < 1 || level > 100)
    {
        return;
    }

    uint32_t backlight_level = (8191 * level) / 100; // Calculate the equivalent duty cycle using 8191 max.

    // Set the change duty cycle to be done in 100 mS.
    ledc_set_fade_with_time(backlight_led.speed_mode, backlight_led.channel, backlight_level, 100);
    ledc_fade_start(backlight_led.speed_mode, backlight_led.channel, LEDC_FADE_NO_WAIT);
    sys_manager.system_save_config(SYS_BRIGHT, level);
}

uint8_t BACKLIGHT::backlight_get()
{
    return sys_manager.system_get_config(SYS_BRIGHT);
}

BACKLIGHT backlight;