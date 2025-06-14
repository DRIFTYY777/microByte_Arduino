//
// Created by dhima on 14-06-2025.
//

#ifndef PROCESS_H
#define PROCESS_H

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"  // Add this for QueueHandle_t
#include "processTimer.h"  // Add this include

// Process states
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_SUSPENDED,
    PROCESS_STATE_TERMINATED
} ProcessState;

// Process statistics
typedef struct {
    uint32_t cpu_usage;         // CPU usage percentage (0-100)
    uint32_t peak_stack_usage;  // Maximum stack usage in bytes
    uint64_t run_time;          // Total run time in milliseconds
    uint32_t context_switches;  // Number of context switches
} ProcessStats;

// Process control block
typedef struct {
    uint32_t pid;                // Process ID
    const char* name;            // Process name
    void* stack_ptr;            // Stack pointer
    uint32_t stack_size;        // Stack size
    ProcessState state;         // Current state
    uint8_t priority;           // Process priority
    uint8_t core_id;            // Core ID where a process runs
    void (*entry_point)(void*); // Process entry point
    void* args;                 // Process arguments
    ProcessStats stats;         // Process statistics
    QueueHandle_t msg_queue;    // Message queue for IPC
    ProcessTimer* timer;        // timer field
} ProcessControlBlock;

// Add this declaration to the header file
TaskHandle_t process_get_task_handle(ProcessControlBlock* pcb);

// Process API
ProcessControlBlock* process_create(const char* name,
                                  void (*entry_point)(void*),
                                  void* args,
                                  uint32_t stack_size,
                                  uint8_t priority);
// Process management
bool process_start(ProcessControlBlock* pcb);
// Process termination
bool process_stop(ProcessControlBlock* pcb);
// Process core management
bool process_set_core(ProcessControlBlock* pcb, uint8_t core_id);
// Process stack management
uint32_t process_get_stack_usage(ProcessControlBlock* pcb);
// Priority management
bool process_set_priority(ProcessControlBlock* pcb, uint8_t new_priority);
// Process suspension and resumption
bool process_suspend(ProcessControlBlock* pcb);
// Resuming a suspended process
bool process_resume(ProcessControlBlock* pcb);
// Inter-process communication
bool process_send_message(ProcessControlBlock* sender, ProcessControlBlock* receiver, void* message, uint32_t msg_size);
// Receiving messages
bool process_receive_message(ProcessControlBlock* pcb, void* buffer, uint32_t buffer_size, uint32_t timeout_ms);
// Process statistics
ProcessStats process_get_stats(ProcessControlBlock* pcb);

#endif //PROCESS_H
