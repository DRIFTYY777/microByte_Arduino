#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <Arduino.h>

class LocalTime
{
private:
    struct tm timeinfo;
    char buffer[32];

    static void timeTask(void *pvParameters);

public:
    void init();

    void print_time(struct tm *t);

    bool setDateTime(const char *datetime);
    bool setDate(const char *date);

    bool setTime(const char *time);

    char *getDateTime();
    char *getDate();
    char *getTime();
    char *getFormattedDate();
};

extern LocalTime local_time;

#endif // LOCALTIME_H