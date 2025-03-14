
/*
 * Function:  input_read
 *
 *  Gets the value of the buttons attached to the mux and if the menu button is pushed,
 *  it peforms some special functions such as brightness and volumen set or open the on
 *  game menu.
 *
 *  Returns: An unsigned interger of 16bit with the status of each button. See the next
 *  table to know the position of each button.
 *  Bit position of each button
 *  - 0 -> Down     - 2 -> Up
 *  - 1 -> Left     - 3 -> Right
 *
 *  - 12 -> Select  - 11 -> Menu
 *  - 10 -> Start
 *
 *  - 9 -> A    - 8 -> B    - 7 -> Y
 *  - 6 -> X    - 5 -> R    - 13 -> L
 */

#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <cstdint>
#include <lvgl.h>

/// @brief Class to handle the input from the user
/// @details This class is used to get the input from the user
/// @details It uses the TCA9555 to get the input from the user
/// @details It also has a special function to be used by the LVGL library
/// @details It has a function to read the value returning 16bit unsigned integer
class UserInput
{
public:
    /// @brief Initialize the input device TCA9555
    /// @details Necessary to for the input device to work
    /// @param
    void input_init(void);

    /// @brief Special function for LVGL Inputs dont use it
    /// @details This function is used by the LVGL library to get the input from the user
    /// @details Using 7 buttons for up, down, left, right, ok, back and menu
    static void user_input_task(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

    /// @brief Read the input from the user
    /// @details This function is used to get the input from the user
    /// @details It returns a 16bit unsigned integer with the status of each button
    /// @details Bit position of each button
    uint16_t input_read(void);
};

extern UserInput user_input;

#endif // USER_INPUT_H