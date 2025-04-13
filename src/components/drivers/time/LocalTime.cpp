#include "LocalTime.h"

#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <mpu_wrappers.h>

RTC_DATA_ATTR struct tm saved_time; // Stored in RTC memory

void LocalTime::timeTask(void *pvParameters)
{
    while (1)
    {
        time_t now;
        time(&now);                      // Get the current system time
        localtime_r(&now, &saved_time);  // Update saved_time with the current time
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void LocalTime::init()
{
    struct tm current_time;
    time_t now;

    // First boot: Set the initial time
    if (saved_time.tm_year == 0)
    {
        ESP_LOGI("RTC", "First boot, setting initial time...");
        time(&now);
        localtime_r(&now, &saved_time);
    }
    else
    {
        // Increase the time based on deep sleep duration
        ESP_LOGI("RTC", "Restoring saved time...");
        time(&now);
        now += esp_timer_get_time() / 1000000; // Adjust for elapsed time
        localtime_r(&now, &saved_time);
    }
    print_time(&saved_time);

    // Create a task to update the time every second
    xTaskCreatePinnedToCore(timeTask, "TimeTask", 2048, NULL, 1, NULL, APP_CPU_NUM);
}

void LocalTime::print_time(tm *t)
{
    printf("Time: %04d-%02d-%02d %02d:%02d:%02d\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

bool LocalTime::setDateTime(const char *datetime)
{
    if (strptime(datetime, "%Y-%m-%d %H:%M:%S", &saved_time) == NULL)
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
    if (strptime(date, "%Y-%m-%d", &saved_time) == NULL)
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

bool LocalTime::setTime(const char *time)
{
    if (strptime(time, "%H:%M:%S", &saved_time) == NULL)
    {
        ESP_LOGE("RTC", "Failed to set time from string: %s", time);
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

LocalTime local_time;
