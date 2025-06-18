//
// Created by dhima on 18-06-2025.
//

#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H
#include "components/core/processManager.h"


class systemMonitor {
private:
    static void system_monitor_task(void* parameter);
public:
    // systemMonitor();
    // ~systemMonitor();
    
    void init();
    void deinit();

    // return all processes name, id, state, priority, stack, runtime, core in char

    // return all processes name, id, state, priority, stack, runtime, core in char
    char* get_all_processes_info(char* buffer, size_t buffer_size);
    char* get_all_processes_info_json(char* buffer, size_t buffer_size);
    // Helper method to get formatted process info for a single process
    void format_single_process_info(const process_info_t& process, char* buffer, size_t buffer_size);
    // Get process state as string
    const char* get_process_state_string(process_state_t state);


};

extern systemMonitor monitor;



#endif //SYSTEMMONITOR_H