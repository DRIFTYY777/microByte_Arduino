
/*
 * Function:  input_read
 * --------------------
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

class UserInput
{
public:
    uint32_t menu_btn_time = 0;

    void input_init(void);
    uint16_t input_read(void);
};

extern UserInput user_input;

#endif // USER_INPUT_H