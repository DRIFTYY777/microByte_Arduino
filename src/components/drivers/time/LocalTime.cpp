// LocalTime.cpp
#include "LocalTime.h"

#include "esp_sleep.h"
#include "esp_timer.h"
#include <mpu_wrappers.h>
#include <components/system_config/system_config.h>
#include <components/drivers/wifi/connections.h>
#include <components/core/processManager.h>
#include "esp_sntp.h"

static const char *TAG = "LocalTime";

RTC_DATA_ATTR struct tm saved_time; // Stored in RTC memory

// Static member definition
uint32_t LocalTime::time_process_id = 0;

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
    ESP_LOGI(TAG, "Time task started and managed by ProcessManager");

    while (true)
    {
        // Check if process is still alive and should continue running
        if (!process_manager.is_process_alive(time_process_id)) {
            ESP_LOGW(TAG, "Time process is no longer alive, terminating task");
            break;
        }

        time_t now;
        time(&now);
        localtime_r(&now, &saved_time);  // Update saved_time with current system time

        // Debug log if minute changes
        static int last_minute = -1;
        if (saved_time.tm_min != last_minute) {
            ESP_LOGI(TAG, "Current time: %02d:%02d:%02d",
                     saved_time.tm_hour, saved_time.tm_min, saved_time.tm_sec);
            last_minute = saved_time.tm_min;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Time task terminating");
    vTaskDelete(NULL);
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
        ESP_LOGI(TAG, "Invalid time detected, setting initial time...");
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

    // Create time process using ProcessManager instead of xTaskCreatePinnedToCore
    time_process_id = process_manager.create_process(
        "TimeTask",                // Process name
        timeTask,                  // Task function
        2048,                      // Stack size
        this,                      // Parameters
        PROCESS_PRIORITY_LOW,      // Priority
        APP_CPU_NUM                // Core affinity
    );

    if (time_process_id == 0) {
        ESP_LOGE(TAG, "Failed to create time process");
        return;
    }

    ESP_LOGI(TAG, "Time process created with ID: %lu", time_process_id);
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
        ESP_LOGE(TAG, "Failed to set date and time from string: %s", datetime);
        return false;
    }
    saved_time.tm_isdst = -1; // Let mktime determine if DST is in effect
    time_t t = mktime(&saved_time);
    if (t == -1)
    {
        ESP_LOGE(TAG, "Failed to convert struct tm to time_t");
        return false;
    }
    saved_time = *localtime(&t); // Update saved_time with the correct timezone
    
    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    
    ESP_LOGI(TAG, "DateTime set successfully");
    return true;
}

bool LocalTime::setDate(const char *date)
{
    struct tm temp_time = saved_time; // Keep current time
    
    if (strptime(date, "%Y-%m-%d", &temp_time) == nullptr)
    {
        ESP_LOGE(TAG, "Failed to set date from string: %s", date);
        return false;
    }
    
    // Keep the current time, only update date
    temp_time.tm_hour = saved_time.tm_hour;
    temp_time.tm_min = saved_time.tm_min;
    temp_time.tm_sec = saved_time.tm_sec;
    temp_time.tm_isdst = -1;
    
    time_t t = mktime(&temp_time);
    if (t == -1)
    {
        ESP_LOGE(TAG, "Failed to convert struct tm to time_t");
        return false;
    }
    
    saved_time = *localtime(&t);
    
    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    
    ESP_LOGI(TAG, "Date set successfully");
    return true;
}

bool LocalTime::setTime(const char* time_str) {
    // Parse input time string
    int hour, min, sec;
    if (sscanf(time_str, "%d:%d:%d", &hour, &min, &sec) != 3) {
        ESP_LOGE(TAG, "Failed to parse time string: %s", time_str);
        return false;
    }

    // Validate time values
    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        ESP_LOGE(TAG, "Invalid time values: %d:%d:%d", hour, min, sec);
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
        ESP_LOGE(TAG, "Failed to convert time");
        return false;
    }

    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    if (settimeofday(&tv, nullptr) != 0) {
        ESP_LOGE(TAG, "Failed to set system time");
        return false;
    }

    // Update saved_time with the new values
    localtime_r(&t, &saved_time);

    // Debug print
    ESP_LOGI(TAG, "Time set to: %02d:%02d:%02d",
             saved_time.tm_hour, saved_time.tm_min, saved_time.tm_sec);

    return true;
}

bool LocalTime::setDay(const char *day) {
    int day_val = atoi(day);
    
    if (day_val < 1 || day_val > 31) {
        ESP_LOGE(TAG, "Invalid day value: %d", day_val);
        return false;
    }
    
    saved_time.tm_mday = day_val;
    saved_time.tm_isdst = -1;
    
    time_t t = mktime(&saved_time);
    if (t == -1) {
        ESP_LOGE(TAG, "Failed to convert time");
        return false;
    }
    
    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    
    // Update saved_time
    localtime_r(&t, &saved_time);
    
    ESP_LOGI(TAG, "Day set to: %d", day_val);
    return true;
}

bool LocalTime::setYear(const char *year) {
    int year_val = atoi(year);
    
    if (year_val < 1900 || year_val > 3000) {
        ESP_LOGE(TAG, "Invalid year value: %d", year_val);
        return false;
    }
    
    saved_time.tm_year = year_val - 1900; // tm_year is years since 1900
    saved_time.tm_isdst = -1;
    
    time_t t = mktime(&saved_time);
    if (t == -1) {
        ESP_LOGE(TAG, "Failed to convert time");
        return false;
    }
    
    // Set system time
    const struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    
    // Update saved_time
    localtime_r(&t, &saved_time);
    
    ESP_LOGI(TAG, "Year set to: %d", year_val);
    return true;
}

char *LocalTime::getDateTime()
{
    time_t now;
    time(&now);
    localtime_r(&now, &saved_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &saved_time);
    return buffer;
}

char *LocalTime::getDate()
{
    time_t now;
    time(&now);
    localtime_r(&now, &saved_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &saved_time);
    return buffer;
}

char *LocalTime::getTime()
{
    time_t now;
    time(&now);
    localtime_r(&now, &saved_time);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &saved_time);
    return buffer;
}

char *LocalTime::getFormattedDate()
{
    time_t now;
    time(&now);
    localtime_r(&now, &saved_time);
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

char* LocalTime::get_date_online() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        strcpy(buffer, "Failed to obtain date");
        return buffer;
    }
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return buffer;
}

bool LocalTime::syncTime() {
    if (WIFI_CONNECTIONS::getState() != CONNECTED) {
        ESP_LOGW(TAG, "Cannot sync time - no WiFi connection");
        return false;
    }
    
    return init_NTCP();
}

int LocalTime::getYear() {
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_year + 1900;
}

int LocalTime::getDay() {
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_mday;
}

// Process management functions
void LocalTime::stop_time_process() {
    if (time_process_id != 0) {
        ESP_LOGI(TAG, "Stopping time process ID: %lu", time_process_id);
        process_manager.delete_process(time_process_id);
        time_process_id = 0;
    }
}

void LocalTime::suspend_time_process() {
    if (time_process_id != 0) {
        ESP_LOGI(TAG, "Suspending time process ID: %lu", time_process_id);
        process_manager.suspend_process(time_process_id);
    }
}

void LocalTime::resume_time_process() {
    if (time_process_id != 0) {
        ESP_LOGI(TAG, "Resuming time process ID: %lu", time_process_id);
        process_manager.resume_process(time_process_id);
    }
}

uint32_t LocalTime::get_time_process_id() {
    return time_process_id;
}

LocalTime local_time;