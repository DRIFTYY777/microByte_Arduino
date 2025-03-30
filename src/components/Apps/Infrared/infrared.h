#if !defined(INFRARED_H)
#define INFRARED_H

#include <IRrecv.h>
#include <assert.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <components/system_config/system_config.h>

#define OBJECT_LENGTH 0.05 // Length of object in meters (e.g., 5 cm)

/*
Universal remote
new remote
save remote
*/

IRsend irsend(TRANSMIT_PIN);
IRrecv irrecv(RECEIVE_PIN);
decode_results results;

class Infrared
{

private:
    /// @brief This function is used to setup the PWM for the IR transmitter
    /// @note The PWM frequency is set to 36kHz and the resolution is set to 8 bits
    /// @note The IR transmitter pin is set to output mode
    /// @note The PWM duty cycle is set to 50%
    void setupIRPWM();

public:
    /// @brief IrTransmitter and Receiver is initialized
    /// @note IR_RECV_PIN is in Input mode and IR_SEND_PIN is in Output mode
    /// @note IR_RECV_PIN change when using the speed measurement
    void init(void);

    /// @brief IrTransmitter and Receiver is deinitialized
    /// @note IR_RECV_PIN is in Input mode and IR_SEND_PIN is in Output mode
    void deinit(void);

    /// @brief Its a method to initialize the speed measurement
    /// @note Reusing the IR_RECV_PIN and IR_SEND_PIN for speed measurement
    /// @note The IR_RECV_PIN is set to input mode and the IR_SEND_PIN is set to output mode
    void initSpeed(void); // method to initialize the speed measurement

    /// @brief Its a method to measure the speed of the object passing through the infrared beam
    /// @note Should be called in the loop to get the speed of the object
    /// @note The speed is calculated based on the time taken for the object to pass through the beam
    /// @return The speed of the object in m/s (char due to lvgl)
    char measureSepeed(void); // method to measure the speed of the infrared signal

    void readRaw();
};

extern Infrared infrared; // Declare an instance of the Infrared class

#endif // INFRARED_H)
