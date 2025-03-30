#include "infrared.h"

static const char *TAG = "Infrared";

volatile bool objectPassing = false;
volatile unsigned long startTime = 0, endTime = 0;

void IRAM_ATTR sensorInterrupt()
{
    if (digitalRead(RECEIVE_PIN) == HIGH)
    { // Object enters the beam
        startTime = micros();
        objectPassing = true;
    }
    else
    { // Object exits the beam
        endTime = micros();
        objectPassing = false;
    }
}

void Infrared::setupIRPWM()
{
    ledcSetup(0, 36000, 8); // Channel 0, 36kHz, 8-bit resolution
    ledcAttachPin(TRANSMIT_PIN, 0);
    ledcWrite(0, 127); // 50% duty cycle
}

void Infrared::init(void)
{
    irsend.begin(); // Start the transmitter
    pinMode(TRANSMIT_PIN, OUTPUT);
    irrecv.enableIRIn(); // Start the receiver
}

void Infrared::deinit(void)
{
    irrecv.disableIRIn();         // Stop the receiver
    pinMode(TRANSMIT_PIN, INPUT); // Set the pin to input mode to save power
}

void Infrared::initSpeed(void)
{
    pinMode(RECEIVE_PIN, INPUT);
    attachInterrupt(RECEIVE_PIN, sensorInterrupt, CHANGE);
    setupIRPWM();
}

char Infrared::measureSepeed(void)
{
    float speed = 0; // Declare speed outside the if block
    if (startTime > 0 && endTime > startTime)
    {
        unsigned long timeDiff = endTime - startTime; // Time in microseconds
        float timeInSeconds = timeDiff / 1e6;
        speed = OBJECT_LENGTH / timeInSeconds; // Speed in m/s
        startTime = 0;
        endTime = 0;
    }
    return speed; // Return the speed in m/s
}

void Infrared::readRaw()
{
    if (irrecv.decode(&results))
    {
        Serial.print("IR Code: ");
        Serial.println(results.value, HEX);
        irrecv.resume(); // Receive the next value
    }
    else
    {
        Serial.println("No IR signal received.");
    }
}

Infrared infrared; // Declare an instance of the Infrared class
