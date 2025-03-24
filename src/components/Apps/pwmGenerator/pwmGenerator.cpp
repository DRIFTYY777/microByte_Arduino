#include "pwmGenerator.h"

void PWMGenerator::Init(uint8_t pin, uint32_t frequency, uint8_t dutyCycle)
{
    this->pin = pin;
    this->frequency = frequency;
    this->dutyCycle = dutyCycle;

    ledcSetup(0, frequency, 8);
    ledcAttachPin(pin, 0);
    ledcWrite(0, dutyCycle);
}

void PWMGenerator::deInit()
{
    ledcDetachPin(pin);
}

void PWMGenerator::setFrequency(uint32_t frequency)
{
    this->frequency = frequency;
    ledcWriteTone(0, frequency);
}

void PWMGenerator::setDutyCycle(uint8_t dutyCycle)
{
    this->dutyCycle = dutyCycle;
    ledcWrite(0, dutyCycle);
}

void PWMGenerator::start()
{
    ledcWrite(0, dutyCycle);
}

void PWMGenerator::stop()
{
    ledcWrite(0, 0);
}
