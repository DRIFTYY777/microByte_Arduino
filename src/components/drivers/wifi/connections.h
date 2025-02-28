#ifndef CONNECTIONS_H
#define CONNECTIONS_H

class WIFI_CONNECTIONS
{
public:
    void wifi_init();
    void wifi_deinit();
};

extern WIFI_CONNECTIONS wifi;

class BLUETOOTH_CONNECTIONS
{
};

#endif // CONNECTIONS_H