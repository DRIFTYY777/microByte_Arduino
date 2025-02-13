#ifndef BATTERY_H
#define BATTERY_H

#include <components/system_config/system_manager.h>

#define DEFAULT_VREF 1040 // Adjust based on your ESP32's ADC calibration
#define QUEUE_SIZE 5      // Maximum number of elements in the queue

#define SDIO 0x00
#define MMC 0x01
#define SDHC 0x02
#define SDSC 0x03

class BATTERY
{

public:
    void battery_init();
    void battery_game_mode(bool game_mode);
    uint8_t battery_get_percentage();

private:
    static void batteryTask(void *arg); // Non-static member function
};

extern BATTERY battery;

#endif // BATTERY_H
