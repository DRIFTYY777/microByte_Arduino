//
// Created by dhima on 14-06-2025.
//

#include "process.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include <string.h>

// Define message struct type for queues
typedef struct {
    void* data;
    uint32_t size;
} ProcessMessage;

static uint32_t next_pid = 1;
static TaskHandle_t task_handles[32] = {NULL}; // Array to store task handles

ProcessControlBlock* process_create(const char* name,
                                  void (*entry_point)(void*),
                                  void* args,
                                  uint32_t stack_size,
                                  uint8_t priority) {
    ProcessControlBlock* pcb = (ProcessControlBlock*)heap_caps_calloc(1,
                                                    sizeof(ProcessControlBlock),
                                                    MALLOC_CAP_INTERNAL);
    if (!pcb) return NULL;


    if (pcb) {
        // ... other initializations ...
        pcb->timer = NULL;  // Initialize timer to NULL
    }

    // Initialize PCB
    pcb->pid = next_pid++;
    pcb->name = name;
    pcb->entry_point = entry_point;
    pcb->args = args;
    pcb->stack_size = stack_size;
    pcb->priority = priority;
    pcb->state = PROCESS_STATE_READY;
    pcb->core_id = 0; // Default to core 0

    // Allocate stack from PSRAM
    // Initialize message queue
    pcb->msg_queue = xQueueCreate(10, sizeof(ProcessMessage));

    if (!pcb->msg_queue) {
        heap_caps_free(pcb->stack_ptr);
        heap_caps_free(pcb);
        return nullptr;
    }

    if (!pcb->msg_queue) {
        heap_caps_free(pcb->stack_ptr);
        heap_caps_free(pcb);
        return nullptr;
    }

    // Initialize stats
    memset(&pcb->stats, 0, sizeof(ProcessStats));

    return pcb;
}

// Add this implementation
TaskHandle_t process_get_task_handle(ProcessControlBlock* pcb) {
    if (!pcb || pcb->pid > 32) {
        return nullptr;
    }
    return task_handles[pcb->pid - 1];
}

bool process_start(ProcessControlBlock* pcb) {
    if (!pcb) return false;

    TaskHandle_t task_handle;
    BaseType_t result = xTaskCreatePinnedToCore(
        pcb->entry_point,
        pcb->name,
        pcb->stack_size / sizeof(StackType_t),
        pcb->args,
        pcb->priority,
        &task_handle,
        pcb->core_id
    );

    if (result == pdPASS) {
        pcb->state = PROCESS_STATE_RUNNING;
        // Store task handle
        task_handles[pcb->pid - 1] = task_handle;
        return true;
    }
    return false;
}

bool process_stop(ProcessControlBlock* pcb) {
    if (!pcb) return false;

    // Clean up timer if it exists
    if (pcb->timer) {
        process_timer_stop(pcb->timer);
        process_timer_free(pcb->timer);
        pcb->timer = nullptr;
    }

    if (task_handles[pcb->pid - 1] != nullptr) {
        // Delete the task
        vTaskDelete(task_handles[pcb->pid - 1]);
        task_handles[pcb->pid - 1] = nullptr;

        // Delete the message queue
        if (pcb->msg_queue) {
            vQueueDelete(pcb->msg_queue);
            pcb->msg_queue = nullptr;
        }

        // Free the stack
        if (pcb->stack_ptr) {
            heap_caps_free(pcb->stack_ptr);
            pcb->stack_ptr = nullptr;
        }

        pcb->state = PROCESS_STATE_TERMINATED;
        return true;
    }
    return false;
}

bool process_set_core(ProcessControlBlock* pcb, uint8_t core_id) {
    if (!pcb || core_id >= portNUM_PROCESSORS) return false;
    pcb->core_id = core_id;
    return true;
}

uint32_t process_get_stack_usage(ProcessControlBlock* pcb) {
    if (!pcb || pcb->pid > 32 || task_handles[pcb->pid - 1] == nullptr) return 0;

    // Get high water mark for the task
    UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(task_handles[pcb->pid - 1]);

    // Calculate used stack space
    return pcb->stack_size - (highWaterMark * sizeof(StackType_t));
}

bool process_set_priority(ProcessControlBlock* pcb, uint8_t new_priority) {
    if (!pcb || pcb->pid > 32 || task_handles[pcb->pid - 1] == nullptr) {
        return false;
    }

    vTaskPrioritySet(task_handles[pcb->pid - 1], new_priority);
    pcb->priority = new_priority;
    return true;
}

bool process_suspend(ProcessControlBlock* pcb) {
    if (!pcb || pcb->pid > 32 || task_handles[pcb->pid - 1] == nullptr) {
        return false;
    }

    vTaskSuspend(task_handles[pcb->pid - 1]);
    pcb->state = PROCESS_STATE_SUSPENDED;
    return true;
}

bool process_resume(ProcessControlBlock* pcb) {
    if (!pcb || pcb->pid > 32 || task_handles[pcb->pid - 1] == nullptr) {
        return false;
    }

    // Get the task handle
    TaskHandle_t task = task_handles[pcb->pid - 1];

    // Check current state
    eTaskState task_state = eTaskGetState(task);
    if (task_state != eSuspended) {
        // Task is not suspended, can't resume
        return false;
    }

    // Resume the task
    vTaskResume(task);

    // Give the scheduler a chance to run
    vTaskDelay(pdMS_TO_TICKS(50));  // Increased delay

    // Verify task state after resume
    task_state = eTaskGetState(task);
    if (task_state == eSuspended) {
        // Resume failed
        return false;
    }

    // Update process state
    pcb->state = PROCESS_STATE_RUNNING;
    return true;
}

bool process_send_message(ProcessControlBlock* sender, ProcessControlBlock* receiver,
                         void* message, uint32_t msg_size) {
    if (!sender || !receiver || !message || !receiver->msg_queue || msg_size == 0) {
        return false;
    }

    // Create message structure
    ProcessMessage msg = {
        .data = heap_caps_malloc(msg_size, MALLOC_CAP_SPIRAM),
        .size = msg_size
    };

    if (!msg.data) {
        return false;
    }

    // Copy message data
    memcpy(msg.data, message, msg_size);

    // Send message structure through queue
    if (xQueueSend(receiver->msg_queue, &msg, pdMS_TO_TICKS(100)) != pdPASS) {
        heap_caps_free(msg.data);
        return false;
    }

    return true;
}

bool process_receive_message(ProcessControlBlock* pcb, void* buffer,
                           uint32_t buffer_size, uint32_t timeout_ms) {
    if (!pcb || !buffer || !pcb->msg_queue || buffer_size == 0) {
        return false;
    }

    ProcessMessage msg;

    if (xQueueReceive(pcb->msg_queue, &msg, pdMS_TO_TICKS(timeout_ms)) == pdPASS) {
        // Check buffer size
        if (buffer_size < msg.size) {
            heap_caps_free(msg.data);
            return false;
        }

        // Copy message to provided buffer
        memcpy(buffer, msg.data, msg.size);

        // Free message buffer
        heap_caps_free(msg.data);
        return true;
    }

    return false;
}

ProcessStats process_get_stats(ProcessControlBlock* pcb) {
    ProcessStats stats = {0};
    if (!pcb || pcb->pid > 32 || task_handles[pcb->pid - 1] == nullptr) {
        return stats;
    }

    TaskHandle_t task = task_handles[pcb->pid - 1];

    // Get stack usage
    stats.peak_stack_usage = pcb->stack_size -
                            (uxTaskGetStackHighWaterMark(task) * sizeof(StackType_t));

    // Get run time
    stats.run_time = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    // Simple CPU usage estimation based on run time
    stats.cpu_usage = 50; // Placeholder value as exact CPU usage is complex to calculate

    // Context switches - use task run time as approximation
    stats.context_switches = xTaskGetTickCount();

    // Store stats in PCB
    pcb->stats = stats;
    return stats;
}