#include "pwmGenerator.h"

void PWMGenerator::Init(uint8_t pin, uint32_t frequency, uint8_t dutyCycle)
{
    this->pin = pin;
    this->frequency = frequency;
    this->dutyCycle = dutyCycle;

    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
    ledcWrite(channel, dutyCycle * ((1 << resolution) - 1) / 100);
}

void PWMGenerator::deInit()
{
    ledcDetachPin(pin);
}

void PWMGenerator::setFrequency(uint32_t frequency)
{
    this->frequency = frequency;
    ledcSetup(channel, frequency, resolution);
}

void PWMGenerator::setDutyCycle(uint8_t dutyCycle)
{
    this->dutyCycle = dutyCycle;
    ledcWrite(channel, dutyCycle * ((1 << resolution) - 1) / 100);
}

void PWMGenerator::start()
{
    ledcWrite(channel, dutyCycle * ((1 << resolution) - 1) / 100);
}

void PWMGenerator::stop()
{
    ledcWrite(channel, 0);
}
