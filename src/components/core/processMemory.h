//
// Created by dhima on 14-06-2025.
//

#ifndef PROCESSMEMORY_H
#define PROCESSMEMORY_H

#include "processSemaphore.h"

// Memory block structure
typedef struct {
    void* address;
    uint32_t size;
    bool is_free;
} MemoryBlock;

// Memory manager structure
extern process_semaphore_t* memory_mutex;  // Global memory mutex

// Memory manager API
void memory_manager_init(void);
// Memory allocation and deallocation
void* memory_allocate(uint32_t size);
// Memory deallocation
bool memory_free(const void* ptr);
// Memory management utilities
uint32_t memory_get_free_size(void);
// Get total memory size
uint32_t memory_get_used_size(void);

#endif //PROCESSMEMORY_H
