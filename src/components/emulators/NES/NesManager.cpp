#include "NesManager.h"
#include <esp_task_wdt.h>
#include <queue.h>

// inlcude c files in cpp file

extern "C"
{
#include "nofrendo/bitmap.h"
#include "nofrendo/nes/nesstate.h"
#include "nofrendo/nofrendo.h"
}

#include <components/system_config/system_manager.h>
#include <components/drivers/display/displayHal.h>

// TaskHandle_t videoTask_handler;
TaskHandle_t nofrendoTask_handler;

QueueHandle_t nofrendo_vidQueue;
QueueHandle_t nofrendo_audioQueue;

/*********Static definition of tasks**************/
static void nofrendo_task(void *arg);
static void nofrendo_video_task(void *arg);
static void nofrendo_audio_task(void *arg);

void NES_start(const char *game_name)
{
    TaskHandle_t idle_0 = xTaskGetIdleTaskHandleForCPU(0);
    esp_task_wdt_delete(idle_0);

    nofrendo_vidQueue = xQueueCreate(7, sizeof(bitmap_t *));
    // nofrendo_audioQueue = xQueueCreate(10, sizeof(int16_t *));

    xTaskCreatePinnedToCore(&nofrendo_video_task, "nofrendo_video_task", 1024 * 12, NULL, 1, &videoTask_handler, 0);
    xTaskCreatePinnedToCore(&nofrendo_task, "nofrendo_main_task", 2024 * 5, (void *)game_name, 1, &nofrendoTask_handler, 1);
}

void NES_resume()
{
    vTaskResume(videoTask_handler);
    // vTaskResume(audioTask_handler);
    vTaskResume(nofrendoTask_handler);
}

void NES_suspend()
{
    vTaskSuspend(nofrendoTask_handler);
    vTaskSuspend(videoTask_handler);
    // vTaskSuspend(audioTask_handler);
}

void NES_load_game()
{
    state_load();
}

void NES_save_game()
{
    state_save();
}

static void nofrendo_task(void *arg)
{
    // Create game route on the SD card
    char *argv[1];
    argv[0] = (char *)malloc(512); // 256
    sprintf(argv[0], "/sdcard/Emulator/NES/%s", (char *)arg);

    // Execute nofrendo emulator
    nofrendo_main(1, argv);
    free(argv[0]);
}

static void nofrendo_video_task(void *arg)
{
    bitmap_t *bmp = NULL;
    while (1)
    {
        xQueueReceive(nofrendo_vidQueue, &bmp, portMAX_DELAY);
        display_HAL_NES_frame((const uint8_t *)bmp->line[0]);
    }
}

static void nofrendo_audio_task(void *arg)
{
}