#ifndef RADIOHALL_H
#define RADIOHALL_H

#include <components/system_config/system_config.h>

extern "C" {
#include "nrf24.h"
}

class RadioHall {
private:
    static nrf24_config_t nrf24_config;
    static bool initialized;

public:
    // Initialize radio hall
    static bool init();

    // Configure radio parameters
    static void config(uint8_t channel, uint8_t payload_size);

    // Check if radio is connected
    static bool connected();

    // Turn radio on/off
    static void turn_on();
    static void turn_off();

    // Get configuration
    static const nrf24_config_t* get_config();

    // Check if initialized
    static bool is_initialized();
};

// Global instance
extern RadioHall radio_hall;

#endif //RADIOHALL_H