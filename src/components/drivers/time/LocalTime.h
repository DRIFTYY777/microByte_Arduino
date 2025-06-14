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

    int getYear();  // Declaration
    int getDay();   // Declaration


    char *getRegion();

    char *get_time_online();
    char *get_date_online();

    bool syncTime();
};

extern LocalTime local_time;

#endif // LOCALTIME_H