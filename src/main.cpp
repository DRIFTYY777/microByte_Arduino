#include <Arduino.h>

#include <components/system_config/system_manager.h>

#include <components/drivers/battery/battery.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/display/display_HAL/display_HAL.h>

#include <esp32-hal-log.h>

uint8_t console_running = NULL;

TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
TimerHandle_t timer;

bool boot_screen_ani = true;

static const char *TAG = "microByte_main";

static void timer_isr(void)
{
    printf("save\r\n");
    // gnuboy_save();
    struct SYSTEM_MODE emulator;
    emulator.mode = MODE_SAVE_GAME;
    // emulator.console = emulator_selected;

    if (xQueueSend(modeQueue, &emulator, (TickType_t)10) != pdPASS)
    {
        ESP_LOGE(TAG, "modeQueue send error");
    }
}

void setup()
{
    /**************** Basic initialization **************/
    sys_manager.system_init_config();

    led_notification.LED_init();
    led_notification.LED_mode(LED_FADE_ON);

    sys_manager.system_info();
    sys_manager.system_memory(MEMORY_SPIRAM),
        sys_manager.system_memory(MEMORY_INTERNAL),
        sys_manager.system_memory(MEMORY_DMA);

    displayHAL.display_HAL_init();

    int32_t status = sys_manager.system_get_state();
    // If the reset was done by a soft reset, it's not necessary to do the intro animation.
    if (status == SYS_SOFT_RESET)
    {
        // boot_screen_ani = false;
        sys_manager.system_set_state(SYS_NORMAL_STATE);
    }

    batteryQueue = xQueueCreate(1, sizeof(BATTERY_STATUS));
    modeQueue = xQueueCreate(1, sizeof(SYSTEM_MODE));

    displayHAL.display_HAL_change_endian();

    sound.audio_init(AUDIO_SAMPLE_16KHZ);
    sd_card.sd_init();
    user_input.input_init();


    bool game_running = false;
    bool game_executed = false;
    struct SYSTEM_MODE management;

    battery.battery_init();
}

void loop()
{
}