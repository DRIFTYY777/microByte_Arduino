#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <cstdint>


/// @brief Class for controlling the backlight of the screen
class BACKLIGHT
{
public:
    /// @brief Initialize the backlight
    /// @details Get the last saved value from the NVS and set the backlight to that value
    void backlight_init();

    /// @brief Set the backlight level
    /// @param level The level of the backlight
    /// @details The level is a value between 0 and 100
    void backlight_set(uint8_t level);

    /// @brief Get the current backlight level saved in the NVS
    /// @return The current backlight level
    uint8_t backlight_get();
};

extern BACKLIGHT backlight;

#endif // BACKLIGHT_H