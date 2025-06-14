//
// Created by dhima on 14-06-2025.
//

#ifndef PROCESSTIMER_H
#define PROCESSTIMER_H

#include <cstdint>
#include "freertos/timers.h"

// Process Timer structure
typedef struct ProcessTimer {
    TimerHandle_t handle;
    void (*callback)(void* context);
    void* context;
    uint32_t period_ms;
    bool is_running;
    bool is_repeat;
} ProcessTimer;
// Function prototypes for Process Timer operations
ProcessTimer* process_timer_create(const char* name,
                                 uint32_t period_ms,
                                 bool repeat,
                                 void (*callback)(void* context),
                                 void* context);
// Function to free the Process Timer
void process_timer_free(ProcessTimer* timer);
// Function to start, stop, reset, and get remaining time of the Process Timer
bool process_timer_start(ProcessTimer* timer);
// Function to stop the Process Timer
bool process_timer_stop(ProcessTimer* timer);
// Function to reset the Process Timer
bool process_timer_reset(ProcessTimer* timer);
// Function to check if the Process Timer is running
uint32_t process_timer_get_remaining(ProcessTimer* timer);

#endif //PROCESSTIMER_H
