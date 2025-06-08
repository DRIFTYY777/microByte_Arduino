#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <cstdint>


#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "freertos/timers.h"

// Apps modes
#define MODE_APPLEJUICE 0x66

// Apps running status
#define STATUS_RUNNING 0x01
#define STATUS_STOPPED 0x00

// clear the mode
#define MODE_NONE 0x00

// just onn off
#define ONN 0x01
#define OFF 0x00

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
#define SYS_VIBRATION 0x04
#define SYS_LED 0x05

#define SYS_WIFI 0x05
#define SYS_BLUETOOTH 0x06

/************ Queue *************/
extern QueueHandle_t modeQueue;
extern QueueHandle_t batteryQueue;
extern QueueHandle_t vidQueue;
extern TimerHandle_t timer;
extern TaskHandle_t videoTask_handler;

struct SYSTEM_MODE
{
    uint8_t mode;
    uint8_t status;
    uint8_t vib_level;
    uint8_t vib_status;
    uint8_t volume_level;
    uint8_t brightness_level;
};
extern SYSTEM_MODE management; // Only declare here

struct APPS
{
    uint8_t mode;
    uint8_t status;
    uint8_t console;
    uint8_t load_save_game;
    char aap_name[200];
};
extern APPS app; // Only declare here

// Struct to save the battery level
struct BATTERY_STATUS
{
    uint8_t percentage;
    uint32_t voltage;
};
extern BATTERY_STATUS battery_status; // Only declare here

struct SYSTEM_WIFI
{
    char ssid[32];
    char password[32];
    uint8_t status;
};
extern SYSTEM_WIFI system_wifi; // Only declare here

// Variables to save machine data
extern char app_version[32];
extern char idf_version[32];
extern char cpu_version[32];

extern uint32_t Internal_ram;
extern uint32_t Used_Internal_ram;

extern uint32_t SPI_ram;
extern uint32_t Used_SPI_ram;

extern uint32_t Free_Flash;
extern uint32_t Flash_Size;

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


    // dedicated to save and get Wi-Fi credentials
    static void saveCredentials(const char *path, const char *value);
    static const char *getCredentials(const char *path);

};

extern SystemManager sys_manager;

#endif