//
// Created by dhima on 14-06-2025.
//

#include "processMemory.h"
#include "esp_heap_caps.h"

#define MEMORY_POOL_SIZE (4 * 1024 * 1024)  // 4MB memory pool
#define MAX_BLOCKS 256

process_semaphore_t* memory_mutex = nullptr;

static uint8_t* memory_pool = nullptr;
static MemoryBlock* blocks = nullptr;
static uint32_t num_blocks = 0;

void memory_manager_init(void) {
    // Initialize memory mutex with proper parameters
    memory_mutex = semaphore_create("memory", SEMAPHORE_TYPE_MUTEX, 1);
    if (!memory_mutex) return;

    // Allocate memory pool from PSRAM
    memory_pool = static_cast<uint8_t *>(heap_caps_malloc(MEMORY_POOL_SIZE, MALLOC_CAP_SPIRAM));
    // Allocate blocks array from internal memory
    blocks = static_cast<MemoryBlock *>(heap_caps_malloc(MAX_BLOCKS * sizeof(MemoryBlock), MALLOC_CAP_INTERNAL));

    if (memory_pool && blocks) {
        // Initialize first block as free, spanning entire pool
        blocks[0].address = memory_pool;
        blocks[0].size = MEMORY_POOL_SIZE;
        blocks[0].is_free = true;
        num_blocks = 1;
    }
}

void* memory_allocate(uint32_t size) {
    // Take the semaphore first
    if (!semaphore_take(memory_mutex, portMAX_DELAY)) {
        return nullptr;
    }
    // Check for zero size
    if (size == 0) {
        semaphore_give(memory_mutex);
        return nullptr;
    }
    // Add alignment
    size = (size + 7) & ~7;
    // Find first fit block
    for (uint32_t i = 0; i < num_blocks; i++) {
        if (blocks[i].is_free && blocks[i].size >= size) {
            // Split block if remaining size is worth tracking
            if (blocks[i].size > size + sizeof(MemoryBlock)) {
                // Check if we have room for a new block
                if (num_blocks >= MAX_BLOCKS) {
                    semaphore_give(memory_mutex);
                    return nullptr;
                }
                // Shift existing blocks
                for (uint32_t j = num_blocks; j > i + 1; j--) {
                    blocks[j] = blocks[j-1];
                }
                // Setup new block from remaining space
                blocks[i + 1].address = (uint8_t*)blocks[i].address + size;
                blocks[i + 1].size = blocks[i].size - size;
                blocks[i + 1].is_free = true;
                // Update current block size
                blocks[i].size = size;
                num_blocks++;
            }
            // Mark block as used and return its address
            blocks[i].is_free = false;
            void* allocated_address = blocks[i].address;
            // Release semaphore before returning
            semaphore_give(memory_mutex);
            return allocated_address;
        }
    }
    // No suitable block found
    semaphore_give(memory_mutex);
    return nullptr;
}

bool memory_free(const void *ptr)
{
    if (!ptr) return false;

    // Find block containing ptr
    for (uint32_t i = 0; i < num_blocks; i++) {
        if (blocks[i].address == ptr) {
            blocks[i].is_free = true;

            // Merge with next block if free
            if (i < num_blocks - 1 && blocks[i + 1].is_free) {
                blocks[i].size += blocks[i + 1].size;
                for (uint32_t j = i + 1; j < num_blocks - 1; j++) {
                    blocks[j] = blocks[j + 1];
                }
                num_blocks--;
            }

            // Merge with previous block if free
            if (i > 0 && blocks[i - 1].is_free) {
                blocks[i - 1].size += blocks[i].size;
                for (uint32_t j = i; j < num_blocks - 1; j++) {
                    blocks[j] = blocks[j + 1];
                }
                num_blocks--;
            }
            return true;
        }
    }
    return false;
}

uint32_t memory_get_free_size(void)
{
    uint32_t free_size = 0;
    for (uint32_t i = 0; i < num_blocks; i++) {
        if (blocks[i].is_free) {
            free_size += blocks[i].size;
        }
    }
    return free_size;

}

uint32_t memory_get_used_size(void)
{
    return MEMORY_POOL_SIZE - memory_get_free_size();
}