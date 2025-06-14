// LocalTime.cpp
#include "LocalTime.h"

#include "esp_sleep.h"
#include "esp_timer.h"
#include <mpu_wrappers.h>
#include <components/system_config/system_config.h>
#include <components/drivers/wifi/connections.h>
#include "esp_sntp.h"

RTC_DATA_ATTR struct tm saved_time; // Stored in RTC memory

bool LocalTime::init_NTCP() {
    if (WIFI_CONNECTIONS::getState() == CONNECTED) {
        configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
        
        Serial.println("Waiting for NTP time sync...");
        
        time_t now = time(nullptr);
        int retry = 0;
        while (now < 8 * 3600 * 2 && retry < 10) {
            delay(500);
            now = time(nullptr);
            retry++;
        }
        
        if (now > 8 * 3600 * 2) {
            struct tm timeinfo;
            getLocalTime(&timeinfo);
            Serial.println("NTP time sync completed!");
            return true;
        }
    }
    Serial.println("No wifi Connection");
    return false;
}

[[noreturn]] void LocalTime::timeTask(void *pvParameters)
{
    // LocalTime* localTime = (LocalTime*)pvParameters;
    while (true)
    {
        time_t now;
        time(&now);
        localtime_r(&now, &saved_time);  // Update saved_time with current system time

        // Debug log if minute changes
        static int last_minute = -1;
        if (saved_time.tm_min != last_minute) {
            ESP_LOGI("RTC", "Current time: %02d:%02d:%02d",
                     saved_time.tm_hour, saved_time.tm_min, saved_time.tm_sec);
            last_minute = saved_time.tm_min;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



void LocalTime::init()
{
    time_t now;
    // Initialize the saved_time with the current time
    time(&now);
    localtime_r(&now, &timeinfo);

    // Set the timezone to your local timezone
    setenv("TZ", "GMT0", 1);
    tzset();

    // First boot or invalid time: Set the initial time
    if (saved_time.tm_year == 0 || saved_time.tm_year < 100)  // Check for invalid year
    {
        ESP_LOGI("RTC", "Invalid time detected, setting initial time...");
        saved_time.tm_year = 2024 - 1900;  // Set to the current year
        saved_time.tm_mon = 0;    // January
        saved_time.tm_mday = 1;   // 1st day
        saved_time.tm_hour = 0;   // 00 hours
        saved_time.tm_min = 0;    // 00 minutes
        saved_time.tm_sec = 0;    // 00 seconds
        saved_time.tm_isdst = -1; // Let the system determine DST

        const time_t t = mktime(&saved_time);
        const struct timeval tv = { .tv_sec = t };
        settimeofday(&tv, nullptr);  // Set the system time using a set time of day instead of stime
    }
    
    time(&now);
    localtime_r(&now, &saved_time);
    print_time(&saved_time);

    // Create a task to update the time every second, passing 'this' pointer
    TaskHandle_t timeTaskHandle;
    xTaskCreatePinnedToCore(timeTask, "TimeTask", 2048, this, 1, &timeTaskHandle, APP_CPU_NUM);
}

void LocalTime::print_time(tm *t)
{
    printf("Time: %04d-%02d-%02d %02d:%02d:%02d\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

bool LocalTime::setDateTime(const char *datetime)
{
    if (strptime(datetime, "%Y-%m-%d %H:%M:%S", &saved_time) == nullptr)
    {
        ESP_LOGE("RTC", "Failed to set date and time from string: %s", datetime);
        return false;
    }
    saved_time.tm_isdst = -1; // Let mktime determine if DST is in effect
    time_t t = mktime(&saved_time);
    if (t == -1)
    {
        ESP_LOGE("RTC", "Failed to convert struct tm to time_t");
        return false;
    }
    saved_time = *localtime(&t); // Update saved_time with the correct timezone
    return true;
}

bool LocalTime::setDate(const char *date)
{
    if (strptime(date, "%Y-%m-%d", &saved_time) == nullptr)
    {
        ESP_LOGE("RTC", "Failed to set date from string: %s", date);
        return false;
    }
    saved_time.tm_isdst = -1; // Let mktime determine if DST is in effect
    time_t t = mktime(&saved_time);
    if (t == -1)
    {
        ESP_LOGE("RTC", "Failed to convert struct tm to time_t");
        return false;
    }
    saved_time = *localtime(&t); // Update saved_time with the correct timezone
    return true;
}

bool LocalTime::setTime(const char* time_str) {
    // Parse input time string
    int hour, min, sec;
    if (sscanf(time_str, "%d:%d:%d", &hour, &min, &sec) != 3) {
        ESP_LOGE("RTC", "Failed to parse time string: %s", time_str);
        return false;
    }

    // Keep the current date but update time in saved_time
    saved_time.tm_hour = hour;
    saved_time.tm_min = min;
    saved_time.tm_sec = sec;
    saved_time.tm_isdst = -1;

    // Convert to time_t
    const time_t t = mktime(&saved_time);
    if (t == -1) {
        ESP_LOGE("RTC", "Failed to convert time");
        return false;
    }

    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    if (settimeofday(&tv, nullptr) != 0) {
        ESP_LOGE("RTC", "Failed to set system time");
        return false;
    }

    // Update saved_time with the new values
    localtime_r(&t, &saved_time);

    // Debug print
    ESP_LOGI("RTC", "Time set to: %02d:%02d:%02d",
             saved_time.tm_hour, saved_time.tm_min, saved_time.tm_sec);

    return true;
}

bool LocalTime::setDay(const char *day) {


}

char *LocalTime::getDateTime()
{
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &saved_time);
    return buffer;
}

char *LocalTime::getDate()
{
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &saved_time);
    return buffer;
}

char *LocalTime::getTime()
{
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &saved_time);
    return buffer;
}

char *LocalTime::getFormattedDate()
{
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &saved_time);
    return buffer;
}

char * LocalTime::getRegion() {
    // Assuming the region is set to GMT0, you can modify this as needed
    strcpy(buffer, "GMT0");
    return buffer;
}

char* LocalTime::get_time_online() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        strcpy(buffer, "Failed to obtain time");
        return buffer;
    }
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return buffer;
}

bool LocalTime::syncTime() {




}

int LocalTime::getYear() {
    // Implementation
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
        return timeinfo.tm_year + 1900;
    }
    return 0;
}

int LocalTime::getDay() {
    // Implementation
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
        return timeinfo.tm_mday;
    }
    return 0;
}

LocalTime local_time;