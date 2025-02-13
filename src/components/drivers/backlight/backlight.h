#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <cstdint>

class BACKLIGHT
{
public:
    void backlight_init();
    void backlight_set(uint8_t level);
    uint8_t backlight_get();
};

extern BACKLIGHT backlight;

#endif // BACKLIGHT_H