#ifndef BATTERY_H
#define BATTERY_H

#include <components/system_config/system_manager.h>

#define SDIO 0x00
#define MMC 0x01
#define SDHC 0x02
#define SDSC 0x03

class BATTERY
{
public:
    bool game_mode_active = false;
    bool battery_alert;
    int channel = 0; // ADC channel for reading battery voltage

    void battery_init(void);
    void battery_game_mode(bool game_mode);
    uint8_t battery_get_percentage();

    void batteryTask(void *arg);               // Non-static member function
    static void batteryTaskWrapper(void *arg); // Static wrapper function
};

extern BATTERY battery;

#endif
