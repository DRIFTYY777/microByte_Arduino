#include "LocalTime.h"
#include <esp32-hal-log.h>
#include <esp_sntp.h>

void LocalTime::updateSystemTime()
{
    struct timeval tv;
    tv.tv_sec = mktime(&timeinfo);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}

/// @brief Initialize SNTP client
/// This function initializes the SNTP client with the default server
/// @note This function be called when Intrnet connection is available
void LocalTime::init()
{
    ESP_LOGI(TAG, "Initializing SNTP...");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

bool LocalTime::setDateTime(const char *datetime)
{
    if (sscanf(datetime, "%d-%d-%d %d:%d:%d",
               &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday,
               &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec) != 6)
    {
        return false;
    }
    timeinfo.tm_year -= 1900; // Convert to struct tm format
    timeinfo.tm_mon -= 1;
    updateSystemTime();
    return true;
}

bool LocalTime::setDate(const char *date)
{
    if (sscanf(date, "%d-%d-%d",
               &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday) != 3)
    {
        return false;
    }
    timeinfo.tm_year -= 1900;
    timeinfo.tm_mon -= 1;
    updateSystemTime();
    return true;
}

bool LocalTime::setTime(const char *time)
{
    if (sscanf(time, "%d:%d:%d",
               &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec) != 3)
    {
        return false;
    }
    updateSystemTime();
    return true;
}

char *LocalTime::getDateTime()
{
    time_t now = time(NULL);
    localtime_r(&now, &timeinfo);
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return buffer;
}

char *LocalTime::getDate()
{
    time_t now = time(NULL);
    localtime_r(&now, &timeinfo);
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    return buffer;
}

char *LocalTime::getTime()
{
    time_t now = time(NULL);
    localtime_r(&now, &timeinfo);
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return buffer;
}

char *LocalTime::getFormattedDate()
{
    time_t now = time(NULL);
    localtime_r(&now, &timeinfo);
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d",
             timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    return buffer;
}
LocalTime local_time;
