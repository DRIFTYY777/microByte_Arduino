//
// Created by dhima on 14-06-2025.
//

#include "processSemaphore.h"
#include "esp_heap_caps.h"
#include <string.h>

process_semaphore_t* semaphore_create(const char* name, semaphore_type_t type, uint32_t max_count) {
    // Allocate semaphore structure
    process_semaphore_t* sem = (process_semaphore_t*)heap_caps_malloc(
        sizeof(process_semaphore_t),
        MALLOC_CAP_INTERNAL
    );

    if (!sem) {
        return nullptr;
    }

    // Initialize structure
    sem->type = type;
    sem->name = name;

    // Create appropriate semaphore based on type
    switch(type) {
        case SEMAPHORE_TYPE_BINARY:
            sem->handle = xSemaphoreCreateBinary();
            if (sem->handle) {
                // Initialize binary semaphore to available state
                xSemaphoreGive(sem->handle);
            }
            break;

        case SEMAPHORE_TYPE_MUTEX:
            sem->handle = xSemaphoreCreateMutex();
            break;

        case SEMAPHORE_TYPE_COUNTING:
            sem->handle = xSemaphoreCreateCounting(max_count, 0);
            break;
    }

    // Check if creation failed
    if (!sem->handle) {
        heap_caps_free(sem);
        return nullptr;
    }

    return sem;
}

void semaphore_delete(process_semaphore_t* sem) {
    if (!sem) {
        return;
    }

    if (sem->handle) {
        vSemaphoreDelete(sem->handle);
        sem->handle = nullptr;
    }

    heap_caps_free(sem);
}

bool semaphore_take(process_semaphore_t* sem, uint32_t timeout_ms) {
    if (!semaphore_is_valid(sem)) {
        return false;
    }

    return xSemaphoreTake(sem->handle, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

bool semaphore_give(process_semaphore_t* sem) {
    if (!semaphore_is_valid(sem)) {
        return false;
    }

    return xSemaphoreGive(sem->handle) == pdTRUE;
}

bool semaphore_is_valid(const process_semaphore_t* sem) {
    return (sem != nullptr) && (sem->handle != nullptr);
}