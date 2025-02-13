#ifndef LED_NOTIFICATION_H
#define LED_NOTIFICATION_H

#include <Arduino.h>

// LED Commands
#define LED_BLINK_HS 0x00 // Blink High Speed -> Period of 100 mS
#define LED_BLINK_LS 0x01 // Blink Low Speed -> Period of 500 mS
#define LED_TURN_ON 0x02
#define LED_TURN_OFF 0x03
#define LED_FADE_ON 0x04
#define LED_FADE_OFF 0x05
#define LED_LOAD_ANI 0x06

class LED_NOTIFICATION
{
public:
    void LED_init();
    void LED_mode(uint8_t mode);

private:
    static void LED_task(void *arg);
};

extern LED_NOTIFICATION led_notification;

#endif // LED_NOTIFICATION_H
