#include "processMemory.h"
#include "esp_heap_caps.h"

#define MEMORY_POOL_SIZE (4 * 1024 * 1024) // 4MB memory pool
#define MAX_BLOCKS 256
process_semaphore_t *memory_mutex = NULL;

static uint8_t *memory_pool = nullptr;
static MemoryBlock *block = nullptr;
static uint32_t num_blocks = 0;

void memory_manager_init()
{
    // Initalize memory mutex with proper param
    memory_mutex = semaphore_create("memory", SEAMPHORE_TYPE_MUTEX, 1);
    if (!memory_mutex)
        return;

    // Allocate memory pool from PSRAM
    memory_pool = (uint8_t *)heap_caps_malloc(MEMORY_POOL_SIZE, MALLOC_CAP_SPIRAM);
    // Allocate blocks array from internal memory
    block = (MemoryBlock *)heap_caps_malloc(MAX_BLOCKS * sizeof(MemoryBlock), MALLOC_CAP_INTERNAL);

    if (memory_pool && block)
    {
        // Initialize first block as free, spanning entire pool
        block[0].address = memory_pool;
        block[0].size = MEMORY_POOL_SIZE;
        block[0].is_free = true;
        num_blocks = 1;
    }
}

void *memory_allocate(size_t size)
{
    // memory allocation with semaphore protection
    if (!semaphore_take(memory_mutex, portMAX_DELAY))
        return nullptr;
    if (size == 0)
    {
        semaphore_give(memory_mutex);
        return nullptr;
    }
    // add alignments
    size = (size + 7) & ~7;

    // finding block
    for (uint32_t i = 0; i < num_blocks; i++)
    {
        if (block[i].is_free && block[i].size >= size)
        {
            // split the block if remaning size is worth tracking
            if (block[i].size > size + sizeof(MemoryBlock))
            {
                if (num_blocks >= MAX_BLOCKS)
                {
                    semaphore_give(memory_mutex);
                    return nullptr;
                }
                // shift existing blocks
                for (uint32_t j = num_blocks; j > i + 1; j--)
                {
                    block[j] = block[j - 1];
                }
                // setup new blocks
                block[i + 1].address = block[i].address + size;
                block[i + 1].is_free = true;
                block[i + 1].size = block[i].size - size;

                block[i].size = size;
                num_blocks++;
            }
            // mark used block and return its address
            block[i].is_free = false;
            void *allocated_address = block[i].address;

            // free the semaphore
            semaphore_give(memory_mutex);
            return allocated_address;
        }
    }
    // no block found error
    semaphore_give(memory_mutex);
    return nullptr;
}

bool memory_free(void *ptr)
{
    if (!ptr)
        return false;

    // find block container pointer
    for (uint32_t i = 0; i < num_blocks; i++)
    {
        if (block[i].address == ptr)
        {
            block[i].is_free = true;

            // merge with next block
            if (i < num_blocks - 1 && block[i + 1].is_free)
            {
                block[i].size += block[i + 1].is_free;
                for (uint32_t j = i + 1; j < num_blocks - 1; j++)
                {
                    block[j] = block[j + 1];
                }
                num_blocks--;
            }

            // merge with previous block if free
            if (i > 0 && block[i - 1].is_free)
            {
                block[i - 1].size += block[i].is_free;
                for (uint32_t j = i; j < num_blocks - 1; j++)
                {
                    block[j] = block[j + 1];
                }
                num_blocks--;
            }
            return true;
        }
    }
    return false;
}

uint32_t memory_get_free_size()
{
    uint32_t free_size = 0;
    for (uint32_t i = 0; i < num_blocks; i++)
    {
        if (block[i].is_free)
        {
            free_size += block[i].size;
        }
    }

    return free_size;
}

uint32_t memory_get_used_size()
{
    return MEMORY_POOL_SIZE - memory_get_free_size();
}

uint32_t memory_get_total_size()
{
    // 4 mb from psram
    return MEMORY_POOL_SIZE;
}
