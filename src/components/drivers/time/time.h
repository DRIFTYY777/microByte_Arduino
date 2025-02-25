#ifndef __TIME_H__
#define __TIME_H__

#include <ESP32Time.h>
#include <WiFi.h>
#include <NTPClient.h>

class LocalTime
{
private:
    ESP32Time time;
    NTPClient ntp;
    WiFiUDP udp;

public:
    LocalTime(const char *ntpServer = "pool.ntp.org",
              int offsetSec = 19800, int updateInterval = 60000)
        : ntp(udp, ntpServer, offsetSec, updateInterval)
    {
    }
    void begin();
    void syncWithNTP();
    void setDate(unsigned long date, unsigned long month, unsigned long year);
    void setTime(unsigned long hour, unsigned long minute, unsigned long second);

    String getFormattedTime();
    bool isWiFiConnected();
    String timeAMPM();
    String time24();
};
extern LocalTime local_time;

#endif