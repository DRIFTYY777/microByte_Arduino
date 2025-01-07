#ifndef SD_CARD_H
#define SD_CARD_H

#include "stdint.h"
#include "stdbool.h"
#include <stddef.h>

#define MOUNT_POINT "/sdcard"
#define SPI_DMA_CHAN 2

extern struct sd_card_info sd_card_info;

struct sd_card_info
{
    char card_name[32];
    uint8_t card_type;
    uint16_t card_size;
    uint32_t card_speed;
};

class SD_CARD
{

private:
    bool SD_mount = false;
    static void organize_list(char *list[30], uint8_t index);

public:
    bool sd_init();
    uint8_t sd_game_list(char *game_list[100], uint8_t console);
    uint8_t sd_app_list(char *app_list[100], bool update);
    size_t sd_file_size(const char *path);
    void sd_get_file(const char *path, void *data);
    char *sd_get_file_flash(const char *path);
    bool sd_mounted();
    bool sd_sav_exist(char *file_name, uint8_t emulator);
    void sd_sav_remove(char *file_name, uint8_t emulator);
};

extern SD_CARD sd_card;

#endif // SD_CARD_H