#include "connections.h"
#include "components/system_config/system_manager.h"
#include "components/system_config/system_config.h"

#include <WiFi.h>

static const char *TAG = "CONNECTIONS";

void WIFI_CONNECTIONS::wifi_init()
{
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    system_wifi.status = ONN;
    sys_manager.system_save_config(SYS_WIFI, ONN);
}

void WIFI_CONNECTIONS::wifi_deinit()
{
    WiFi.disconnect();
    system_wifi.status = OFF;
    sys_manager.system_save_config(SYS_WIFI, OFF);
}

WIFI_CONNECTIONS wifi;
