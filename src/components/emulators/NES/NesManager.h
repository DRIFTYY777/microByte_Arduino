#ifndef NESMANAGER_H
#define NESMANAGER_H

#include <freertos/FreeRTOS.h>
#include <queue.h>

extern QueueHandle_t nofrendo_vidQueue;
extern QueueHandle_t nofrendo_audioQueue;

void NES_start(const char *game_name);
void NES_resume();
void NES_suspend();
void NES_load_game();
void NES_save_game();

#endif // NESMANAGER_H