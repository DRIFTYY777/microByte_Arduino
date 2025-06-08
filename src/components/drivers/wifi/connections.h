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
    static bool begin_wifi();
    static void wifi_task(void *pvParameters);


public:


    static void wifi_init();
    static void wifi_deinit();
    static WiFiState getState();

    static bool changePassword(char *ssid, char *password);



};

extern WIFI_CONNECTIONS wifi;

class BLUETOOTH_CONNECTIONS
{
};

#endif // CONNECTIONS_H