#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <Arduino.h>

class LocalTime
{
private:
    struct tm timeinfo;
    char buffer[32];
    const long gmtOffset_sec = 19800;
    const int daylightOffset_sec = 0;

    // Process ID for time task
    static uint32_t time_process_id;

    static void timeTask(void *pvParameters);

public:
    void init();

    bool init_NTCP();

    static void print_time(struct tm *t);

    static bool setDateTime(const char *datetime);
    static bool setDate(const char *date);
    static bool setTime(const char *time);
    static bool setDay(const char *day);
    static bool setYear(const char *year);

    char *getDateTime();
    char *getDate();
    char *getTime();
    char *getFormattedDate();

    int getYear();
    int getDay();

    char *getRegion();

    char *get_time_online();
    char *get_date_online();

    bool syncTime();

    // Process management functions
    void stop_time_process();
    void suspend_time_process();
    void resume_time_process();
    uint32_t get_time_process_id();
};

extern LocalTime local_time;

#endif // LOCALTIME_H