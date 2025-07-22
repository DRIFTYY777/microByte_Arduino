#include "processSemaphore.h"

process_semaphore_t *semaphore_create(const char *name,
                                      semaphore_type_t type,
                                      uint32_t max_count)
{
    // allocate semaphore struct
    process_semaphore_t *sem = (process_semaphore_t *)heap_caps_malloc(
        sizeof(process_semaphore_t),
        MALLOC_CAP_SPIRAM
        // MALLOC_CAP_INTERNAL
    );

    if (!sem)
    {
        return NULL;
    }

    // init structure
    sem->type = type;
    sem->name = name;

    // create appropriate semaphore based on type
    switch (type)
    {
    case SEAMPHORE_TYPE_BINARY:
        sem->handel = xSemaphoreCreateBinary();
        if (sem->handel)
        {
            // init binary semaphore
            xSemaphoreGive(sem->handel);
        }
        break;
    case SEAMPHORE_TYPE_COUNTING:
        sem->handel = xSemaphoreCreateCounting(max_count, 0);
        break;
    case SEAMPHORE_TYPE_MUTEX:
        sem->handel = xSemaphoreCreateMutex();
        break;
    }

    // check of creation is failed
    if (!sem->handel)
    {
        heap_caps_free(sem);
    }

    return sem;
}

void semaphore_delete(process_semaphore_t *sem)
{
    if (!sem)
    {
        // not existed
        return;
    }
    if (sem->handel)
    {
        vSemaphoreDelete(sem->handel);
        sem->handel = NULL;
    }
    
    heap_caps_free(sem);
}

bool semaphore_take(process_semaphore_t *sem, uint32_t timeout_ms)
{
    if (!semaphore_is_valid(sem))
    {
        return false;
    }

    return xSemaphoreTake(sem->handel, pdMS_TO_TICKS(timeout_ms) == pdTRUE);
}

bool semaphore_give(process_semaphore_t *sem){
    if (!semaphore_is_valid(sem))
    {
        return false;
    }
    return xSemaphoreGive(sem->handel) == pdTRUE;
}
bool semaphore_is_valid(const process_semaphore_t* sem){
    return (sem != NULL) && (sem->handel != NULL);
}

