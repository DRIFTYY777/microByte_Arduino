#if !defined(PWMGENERATOR_)
#define PWMGENERATOR_

/*
   __    __    __    __    __    __
__|  |__|  |__|  |__|  |__|  |__|  |__
        Duty Cycle: 50%
   __    __    __    __    __    __
__|  |__|  |__|  |__|  |__|  |__|  |__
        Frequency: 1000Hz

*/

#include <Arduino.h>

class PWMGenerator
{
private:
    uint8_t pin;
    uint32_t frequency;
    uint8_t dutyCycle;
    uint8_t channel;
    uint8_t timer;
    uint8_t resolution = 8; // 8-bit resolution (0-255)

public:
    PWMGenerator(uint8_t channel, uint8_t timer) : channel(channel), timer(timer) {}

    void Init(uint8_t pin, uint32_t frequency, uint8_t dutyCycle);
    void deInit();
    void setFrequency(uint32_t frequency);
    void setDutyCycle(uint8_t dutyCycle);
    void start();
    void stop();
};
#endif // PWMGENERATOR_
