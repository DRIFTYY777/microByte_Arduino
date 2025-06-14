//
// Created by dhima on 14-06-2025.
//

#ifndef PROCESSSEMAPHORE_H
#define PROCESSSEMAPHORE_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Semaphore types for different use cases
typedef enum {
    SEMAPHORE_TYPE_BINARY,     // For yes/no signaling
    SEMAPHORE_TYPE_MUTEX,      // For resource protection
    SEMAPHORE_TYPE_COUNTING    // For resource pool management
} semaphore_type_t;

// Core semaphore structure
typedef struct {
    SemaphoreHandle_t handle;  // FreeRTOS semaphore handle
    semaphore_type_t type;     // Type of this semaphore
    const char* name;          // Optional name for debugging
} process_semaphore_t;

// Core API functions
process_semaphore_t* semaphore_create(const char* name, semaphore_type_t type, uint32_t max_count);
// Function to delete a semaphore
void semaphore_delete(process_semaphore_t* sem);
// Function to reset a semaphore
bool semaphore_take(process_semaphore_t* sem, uint32_t timeout_ms);
// Function to take a semaphore with a timeout
bool semaphore_give(process_semaphore_t* sem);
// check if a semaphore is available
bool semaphore_is_valid(const process_semaphore_t* sem);

#endif //PROCESSSEMAPHORE_H
