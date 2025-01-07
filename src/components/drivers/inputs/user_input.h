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