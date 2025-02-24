#include <cstdint>
#ifndef VIBRATION_H
#define VIBRATION_H

class VIBRATION
{
public:
    void vibration_init();
    void vibration_set(uint8_t level, uint8_t duration);
    void vibration_on();
    void vibration_off();
};
extern VIBRATION vibration;


#endif // VIBRATION_H