#include "backlight_ctrl.h"
#include <driver/ledc.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

ledc_channel_config_t backlight_led;

void BacklightCtrl::backlight_init()
{
    // Initialize LED timer
    ledc_timer_config_t ledc0_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 8000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc0_timer);
    // Configure the LEDC hardware
    backlight_led.channel = LEDC_CHANNEL_1;
    backlight_led.duty = 8000;
    backlight_led.gpio_num = HSPI_BACKLIGTH;
    backlight_led.speed_mode = LEDC_LOW_SPEED_MODE;
    backlight_led.hpoint = 0;
    backlight_led.timer_sel = LEDC_TIMER_1;

    ledc_channel_config(&backlight_led);

    ledc_fade_func_install(0);
}

void BacklightCtrl::backlight_set(uint8_t level)
{
    if (level < 1 || level > 100)
        return;

    uint16_t backligth_level = (8000 * level) / 100; // Calcute the equivalent duty cycle

    // Set the change duty cycle to be done in 100 mS.
    ledc_set_fade_with_time(backlight_led.speed_mode, backlight_led.channel, backligth_level, 100);
    ledc_fade_start(backlight_led.speed_mode, backlight_led.channel, LEDC_FADE_NO_WAIT);
    sys_manager.system_save_config(SYS_BRIGHT, level);
}

uint8_t BacklightCtrl::backlight_get()
{
    return sys_manager.system_get_config(SYS_BRIGHT);

}
BacklightCtrl backlight;
