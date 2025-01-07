#ifndef BACKLIGHT_CTRL_H
#define BACKLIGHT_CTRL_H

#include <cstdint>

class BacklightCtrl
{

public:
    void backlight_init();
    void backlight_set(uint8_t level);
    uint8_t backlight_get();
};
extern BacklightCtrl backlight;

#endif // BACKLIGHT_CTRL_H