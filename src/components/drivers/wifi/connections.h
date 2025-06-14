#ifndef CONNECTIONS_H
#define CONNECTIONS_H
#include <WString.h>

// WiFi connection states
enum WiFiState {
    CONNECTED,
    AVAILABLE,
    WRONG_PASSWORD,
    NO_CREDENTIALS,
    CONNECTING,
    CONNECTION_FAILED
};

class WIFI_CONNECTIONS
{
private:
    static WiFiState currentState;
    static uint32_t wifi_process_id;
    static bool begin_wifi();
    static void wifi_task(void *pvParameters);

public:
    static void wifi_init();
    static void wifi_deinit();
    static WiFiState getState();
    static bool changePassword(char *ssid, char *password);
    
    // Process management functions
    static void stop_wifi_process();
    static void suspend_wifi_process();
    static void resume_wifi_process();
    static uint32_t get_wifi_process_id();
    static bool restart_wifi_connection();
};

extern WIFI_CONNECTIONS wifi;

class BLUETOOTH_CONNECTIONS
{
};

#endif // CONNECTIONS_H