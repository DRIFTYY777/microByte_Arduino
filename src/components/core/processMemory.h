#if !defined(PROCESS_MEMORY)
#define PROCESS_MEMORY

#include <cstdint>
#include "processSemaphore.h"

// Memory management for process memory
typedef struct
{
    void *address; // Pointer to the allocated memory
    size_t size;   // Size of the allocated memory
    bool is_free;
} MemoryBlock;

extern process_semaphore_t *memory_semaphore; // Semaphore for memory management

void memory_manager_init();
void *memory_allocate(size_t size);
bool memory_free(void *ptr);
uint32_t memory_get_free_size();
uint32_t memory_get_used_size();
uint32_t memory_get_total_size();

#endif // PROCESS_MEMORY
