#if !defined(PROCESS_SEMAPHORE_H)
#define PROCESS_SEMAPHORE_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>

// Seamphore types for diffrent use cases
typedef enum
{
    SEAMPHORE_TYPE_BINARY,   // 0 or 1 for signaling
    SEAMPHORE_TYPE_MUTEX,    // for resourse protection
    SEAMPHORE_TYPE_COUNTING, // resource pool management
} semaphore_type_t;

typedef struct
{
    /* data */
    SemaphoreHandle_t handel;
    semaphore_type_t type;
    const char *name;
} process_semaphore_t;

// core apis methods
process_semaphore_t *semaphore_create(const char *name, semaphore_type_t type, uint32_t max_count);
void semaphore_delete(process_semaphore_t *sem);
bool semaphore_take(process_semaphore_t *sem, uint32_t timeout_ms);
bool semaphore_give(process_semaphore_t *sem);
bool semaphore_is_valid(const process_semaphore_t *sem);

#endif // PROCESS_SEMAPHORE_H
