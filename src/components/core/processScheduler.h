//
// Created by dhima on 14-06-2025.
//

#ifndef PROCESSSCHEDULER_H
#define PROCESSSCHEDULER_H

#include "process.h"

// Scheduler initialization
void scheduler_init(void);

// Add process to scheduler
bool scheduler_add_process(ProcessControlBlock* pcb);

// Remove process from scheduler
bool scheduler_remove_process(ProcessControlBlock* pcb);

// Get current running process
ProcessControlBlock* scheduler_get_current_process(void);

// Core-specific scheduling
void scheduler_assign_to_core(ProcessControlBlock* pcb, uint8_t core_id);

#endif //PROCESSSCHEDULER_H
