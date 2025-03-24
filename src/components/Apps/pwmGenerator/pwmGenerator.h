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

public:
    void Init(uint8_t pin, uint32_t frequency, uint8_t dutyCycle);
    void deInit();
    void setFrequency(uint32_t frequency);
    void setDutyCycle(uint8_t dutyCycle);
    void start();
    void stop();
};
#endif // PWMGENERATOR_
