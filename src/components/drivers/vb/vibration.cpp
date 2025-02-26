#include "vibration.h"
#include <driver/ledc.h>
#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

ledc_channel_config_t ledc_channel;

void VIBRATION::vibration_init()
{
    // Initialize the vibration motor VIBRATION_PIN 42
    ledc_timer_config_t ledc0_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc0_timer);

    // Configure LEDC Channel (Initialize the global variable)
    ledc_channel.gpio_num = (gpio_num_t)VIBRATION_PIN;
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel.duty = 0;
    ledc_channel.hpoint = 0;

    ledc_channel_config(&ledc_channel);

    // Enable LEDC Fade
    ledc_fade_func_install(0);
}

void VIBRATION::vibration_set(uint8_t level, uint8_t duration)
{
    if (sys_manager.system_get_config(SYS_VIBRATION) == ONN)
    {
        if (level < 1 || level > 100)
        {
            return;
        }
        uint32_t vibration_level = (8191 * level) / 100;
        ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, vibration_level, duration);
        ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);
    }
}

void VIBRATION::vibration_on()
{
    management.vib_status = ONN;
    sys_manager.system_save_config(SYS_VIBRATION, management.vib_status);
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 8191);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

void VIBRATION::vibration_off()
{
    management.vib_status = OFF;
    sys_manager.system_save_config(SYS_VIBRATION, management.vib_status);
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

VIBRATION vibration;
