//
// Created by dhima on 14-06-2025.
//

#include "processTimer.h"
#include "esp_heap_caps.h"


static void timer_callback_wrapper(TimerHandle_t xTimer) {
    ProcessTimer* timer = (ProcessTimer*)pvTimerGetTimerID(xTimer);
    if (timer && timer->callback) {
        timer->callback(timer->context);
    }
}

ProcessTimer* process_timer_create(const char* name,
                                 uint32_t period_ms,
                                 bool repeat,
                                 void (*callback)(void* context),
                                 void* context) {
    if (!callback || period_ms == 0) {
        return nullptr;
    }

    ProcessTimer* timer = (ProcessTimer*)heap_caps_malloc(
        sizeof(ProcessTimer),
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    );

    if (!timer) {
        return nullptr;
    }

    timer->handle = xTimerCreate(
        name,
        pdMS_TO_TICKS(period_ms),
        repeat ? pdTRUE : pdFALSE,
        timer,
        timer_callback_wrapper
    );

    if (!timer->handle) {
        heap_caps_free(timer);
        return nullptr;
    }

    timer->callback = callback;
    timer->context = context;
    timer->period_ms = period_ms;
    timer->is_running = false;
    timer->is_repeat = repeat;

    return timer;
}

void process_timer_free(ProcessTimer* timer) {
    if (!timer) {
        return;
    }

    if (timer->handle) {
        xTimerDelete(timer->handle, pdMS_TO_TICKS(100));
    }
    heap_caps_free(timer);
}

bool process_timer_start(ProcessTimer* timer) {
    if (!timer || !timer->handle) {
        return false;
    }

    BaseType_t result = xTimerStart(timer->handle, pdMS_TO_TICKS(100));
    if (result == pdPASS) {
        timer->is_running = true;
        return true;
    }
    return false;
}

bool process_timer_stop(ProcessTimer* timer) {
    if (!timer || !timer->handle) {
        return false;
    }

    BaseType_t result = xTimerStop(timer->handle, pdMS_TO_TICKS(100));
    if (result == pdPASS) {
        timer->is_running = false;
        return true;
    }
    return false;
}

bool process_timer_reset(ProcessTimer* timer) {
    if (!timer || !timer->handle) {
        return false;
    }

    return (xTimerReset(timer->handle, pdMS_TO_TICKS(100)) == pdPASS);
}

uint32_t process_timer_get_remaining(ProcessTimer* timer) {
    if (!timer || !timer->handle || !timer->is_running) {
        return 0;
    }

    TickType_t ticks = xTimerGetExpiryTime(timer->handle) - xTaskGetTickCount();
    return pdTICKS_TO_MS(ticks);
}
