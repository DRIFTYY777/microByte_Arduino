#include "time.h"

static const char *TAG = "LOCAL_TIME";

void LocalTime::begin()
{
    ntp.begin();
}

bool LocalTime::syncWithNTP()
{
    if (!WiFi.isConnected())
    {
        ESP_LOGD(TAG, "WiFi not connected. Cannot sync with NTP.");
        return false;
    }
    ntp.update();
    unsigned long epochTime = ntp.getEpochTime();
    time.setTime(epochTime);
    return true;
}

void LocalTime::setDate(unsigned long date, unsigned long month, unsigned long year)
{
    time.setTime(time.getHour(), time.getMinute(), time.getSecond(), date, month, year);
}

void LocalTime::setTime(unsigned long hour, unsigned long minute, unsigned long second)
{
    time.setTime(hour, minute, second, time.getDay(), time.getMonth(), time.getYear());
}

String LocalTime::getFormattedTime()
{
    // Get current time from RTC
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             time.getYear(), time.getMonth(), time.getDay(),
             time.getHour(), time.getMinute(), time.getSecond());
    return String(buffer);
}

bool LocalTime::isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String LocalTime::timeAMPM()
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d %s",
             time.getHour(), time.getMinute(), time.getSecond(),
             time.getHour() >= 12 ? "PM" : "AM");
    return String(buffer);
}

String LocalTime::time24()
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             time.getHour(), time.getMinute(), time.getSecond());
    return String(buffer);
}

LocalTime local_time;