#ifndef LED_NOTIFICATION_H
#define LED_NOTIFICATION_H

#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// LED modes
#define LED_TURN_ON 1
#define LED_TURN_OFF 2
#define LED_BLINK_HS 3
#define LED_BLINK_LS 4
#define LED_FADE_ON 5
#define LED_FADE_OFF 6
#define LED_LOAD_ANI 7

extern QueueHandle_t LEDQueue;

class LED_NOTIFICATION
{
public:
    /// @brief Initialize the LED notification system
    /// @details Sets up LEDC, creates queue, and starts LED process using ProcessManager
    static void LED_init();

    /// @brief Set LED mode
    /// @param mode LED mode to set
    /// @details Saves mode to NVS and sends to queue
    static void LED_mode(uint8_t mode);

    /// @brief Set LED state directly
    /// @param state 1 for on, 0 for off
    static void LED_set(uint8_t state);

    /// @brief Stop LED process
    /// @details Cleanly stops the LED process managed by ProcessManager
    static void LED_stop();

    /// @brief Suspend LED process
    /// @details Suspends the LED process without destroying it
    static void LED_suspend();

    /// @brief Resume LED process
    /// @details Resumes a suspended LED process
    static void LED_resume();

    /// @brief Get LED process ID
    /// @return Process ID of the LED task
    static uint32_t LED_get_process_id();

private:
    /// @brief LED task function managed by ProcessManager
    /// @param arg Task parameters (unused)
    static void LED_task(void *arg);
};

extern LED_NOTIFICATION led_notification;

#endif // LED_NOTIFICATION_H