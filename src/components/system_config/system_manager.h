#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <cstdint>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Modes available
#define MODE_GAME 0x01
#define MODE_SAVE_GAME 0x02
#define MODE_LOAD_GAME 0x03
#define MODE_EXT_APP 0x04
#define MODE_UPDATE 0x05
#define MODE_OUT 0x06
#define MODE_BATTERY_ALERT 0x07

// Emulators available to select
#define GAMEBOY 0x00
#define GAMEBOY_COLOR 0x01
#define NES 0x02
#define SNES 0x03
#define SMS 0x04
#define GG 0x05

// Memory types
#define MEMORY_DMA 0x00
#define MEMORY_INTERNAL 0x01
#define MEMORY_SPIRAM 0X02
#define MEMORY_ALL 0x03

// System States
#define SYS_NORMAL_STATE 0x00
#define SYS_SOFT_RESET 0x01

// System configuration variables
#define SYS_BRIGHT 0x00
#define SYS_VOLUME 0x01
#define SYS_GUI_COLOR 0x02
#define SYS_STATE_SAV_BTN 0x03

/************ Queue *************/
extern QueueHandle_t modeQueue;
extern QueueHandle_t batteryQueue;

struct SYSTEM_MODE
{
    uint8_t mode;
    uint8_t status;
    uint8_t console;
    uint8_t load_save_game;
    uint8_t volume_level;
    uint8_t brightness_level;
    char game_name[200];
};
// Struct to save the battery level
struct BATTERY_STATUS
{
    uint8_t percentage;
    uint32_t voltage;
};

// Variables to save machine data
extern char app_version[32];
extern char idf_version[32];
extern char cpu_version[32];
extern uint32_t RAM_size;
extern uint32_t FLASH_size;

class SystemManager
{

private:
public:
    // Public methods
    void system_info();
    int system_memory(uint8_t memory);
    void system_init_config();
    void system_set_state(int8_t state);
    int8_t system_get_state();
    void system_save_config(uint8_t config, int8_t value);
    int8_t system_get_config(uint8_t config);
};

extern SystemManager sys_manager;

#endif