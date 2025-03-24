#ifndef SD_CARD_H
#define SD_CARD_H

#include "stdint.h"
#include "stdbool.h"
#include <stddef.h>

/*
  begin sd card with default
  #define VSPI_MOSI 41 // SDA or MOSI
  #define VSPI_MISO 38 // SCL or MISO
  #define VSPI_CLK 40  // CLK
  #define SD_CS 39
  #define SD_SPEED 4000000 // 4Mhz
*/

#define MOUNT_POINT "/sdcard"

#define SDIO 0x00
#define MMC 0x01
#define SDHC 0x02
#define SDSC 0x03

struct SD_CARD_INFO
{
    uint8_t card_type = 0;
    uint16_t card_size = 0;
    uint8_t card_status = 0;
    uint8_t card_mounted = 0;
    uint8_t free_space = 0;
    uint8_t used_space = 0;
};
extern SD_CARD_INFO sd_card_info;

class SD_CARD
{

private:
    bool SD_mount = false;
    static void organize_list(char *list[30], uint8_t index);
    /* Emulator Directory Handler */
    void emulator_dir();
    /* System Directory Handler*/
    void system_dir();

public:
    bool sd_init();
    bool sd_init2();
    bool sd_default();
    bool is_card_mounted();

    uint8_t sd_app_list(char *app_list[100], bool update);

    /* Emulators Function */
    uint8_t sd_game_list(char *game_list[100], uint8_t console);
    bool sd_sav_exist(char *file_name, uint8_t emulator);
    void sd_sav_remove(char *file_name, uint8_t emulator);
};

extern SD_CARD sd_card;

#endif // SD_CARD_H