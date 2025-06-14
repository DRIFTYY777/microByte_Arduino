//
// Created by dhima on 14-06-2025.
//

#include "processScheduler.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_PROCESSES 32
#define NUM_CORES 2  // ESP32-S3 has 2 cores

typedef struct {
    ProcessControlBlock* processes[MAX_PROCESSES];
    uint32_t process_count;
    ProcessControlBlock* current_process[NUM_CORES];
} Scheduler;

static Scheduler scheduler;

void scheduler_init(void) {
    memset(&scheduler, 0, sizeof(Scheduler));
}

bool scheduler_add_process(ProcessControlBlock* pcb) {
    if (!pcb || scheduler.process_count >= MAX_PROCESSES) {
        return false;
    }

    // Add process to array
    scheduler.processes[scheduler.process_count++] = pcb;
    pcb->state = PROCESS_STATE_READY;

    // Set current process for the assigned core
    scheduler.current_process[pcb->core_id] = pcb;

    return true;
}

bool scheduler_remove_process(ProcessControlBlock* pcb) {
    if (!pcb) return false;

    // Find and remove process
    for (uint32_t i = 0; i < scheduler.process_count; i++) {
        if (scheduler.processes[i] == pcb) {
            // Remove from current process if it's running
            for (uint8_t core = 0; core < NUM_CORES; core++) {
                if (scheduler.current_process[core] == pcb) {
                    scheduler.current_process[core] = nullptr;
                }
            }

            // Shift remaining processes
            for (uint32_t j = i; j < scheduler.process_count - 1; j++) {
                scheduler.processes[j] = scheduler.processes[j + 1];
            }
            scheduler.process_count--;
            return true;
        }
    }

    return false;
}

ProcessControlBlock* scheduler_get_current_process(void) {
    const BaseType_t core_id = xPortGetCoreID();
    for (uint32_t i = 0; i < scheduler.process_count; i++) {
        if (scheduler.processes[i]->core_id == core_id) {
            return scheduler.processes[i];
        }
    }
    return nullptr;
}

void scheduler_assign_to_core(ProcessControlBlock* pcb, uint8_t core_id) {
    if (!pcb || core_id >= NUM_CORES) return;

    // Update process core assignment
    pcb->core_id = core_id;

    // If process is currently running on a different core, reset it
    for (uint8_t core = 0; core < NUM_CORES; core++) {
        if (core != core_id && scheduler.current_process[core] == pcb) {
            scheduler.current_process[core] = nullptr;
        }
    }
}