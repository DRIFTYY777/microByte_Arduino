//
// Created by dhima on 18-06-2025.
//

#include "systemMonitor.h"
#include <components/core/processManager.h>

static const char *TAG = "SystemMonitor";

[[noreturn]] void systemMonitor::system_monitor_task(void *parameter)
{
    const TickType_t monitor_delay = pdMS_TO_TICKS(10000); // 30 seconds
    while (true) {
        // process_manager.print_system_resources();
        process_manager.update_all_processes();
        // process_manager.print_process_table();

        // Cleanup dead processes
        process_manager.cleanup_dead_processes();
        vTaskDelay(monitor_delay);
    }
}

void systemMonitor::init() {

    // Start process monitoring
    process_manager.start_monitoring();

    const uint32_t monitor_process_id = process_manager.create_process("SystemMonitor", systemMonitor::system_monitor_task,
                                                               1024 * 4, nullptr, PROCESS_PRIORITY_LOW, 1);
    if (monitor_process_id == 0) {
        ESP_LOGW(TAG, "Failed to create system monitor process");
    }
}

void systemMonitor::deinit() {

    // Stop process monitoring
    process_manager.stop_monitoring();

    // delete the SystemMonitor process if it exists
    auto all_processes = process_manager.get_all_processes();
    for (const auto& process : all_processes) {
        if (process.name == "SystemMonitor") {
            process_manager.delete_process(process.id);
            ESP_LOGI(TAG, "SystemMonitor process deleted");
            break;
        }
    }
}

char * systemMonitor::get_all_processes_info(char *buffer, size_t buffer_size) {

    if (!buffer || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer provided");
        return nullptr;
    }
    // Clear the buffer
    memset(buffer, 0, buffer_size);
    // Update process information first
    process_manager.update_all_processes();
    // Get all processes
    std::vector<process_info_t> processes = process_manager.get_all_processes();

    // Create header
    char header[] = "ID   | Name                 | State     | Priority | Stack | Runtime | Core\n"
                   "-----+----------------------+-----------+----------+-------+---------+------\n";

    // Check if header fits in buffer
    if (strlen(header) >= buffer_size) {
        ESP_LOGE(TAG, "Buffer too small for header");
        return nullptr;
    }

    // Copy header to buffer
    strcpy(buffer, header);
    size_t current_length = strlen(buffer);

    // Add each process
    for (const auto& process : processes) {
        char process_line[256];
        format_single_process_info(process, process_line, sizeof(process_line));

        // Check if adding this process line would exceed buffer size
        if (current_length + strlen(process_line) + 1 >= buffer_size) {
            ESP_LOGW(TAG, "Buffer size limit reached, truncating process list");
            break;
        }

        // Append process line to buffer
        strcat(buffer, process_line);
        current_length = strlen(buffer);
    }

    // Add footer with total count
    char footer[128];
    snprintf(footer, sizeof(footer), "\nTotal Processes: %zu\n", processes.size());

    // Check if footer fits
    if (current_length + strlen(footer) < buffer_size) {
        strcat(buffer, footer);
    }

    return buffer;
}

char * systemMonitor::get_all_processes_info_json(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return nullptr;
    }

    memset(buffer, 0, buffer_size);
    process_manager.update_all_processes();
    std::vector<process_info_t> processes = process_manager.get_all_processes();

    strcpy(buffer, "{\n  \"processes\": [\n");
    size_t current_length = strlen(buffer);

    for (size_t i = 0; i < processes.size(); i++) {
        const auto& process = processes[i];
        char process_json[256];

        snprintf(process_json, sizeof(process_json),
                 "    {\n"
                 "      \"id\": %lu,\n"
                 "      \"name\": \"%s\",\n"
                 "      \"state\": \"%s\",\n"
                 "      \"priority\": %d,\n"
                 "      \"stack\": %lu,\n"
                 "      \"runtime\": %lu,\n"
                 "      \"core\": %s\n"
                 "    }%s\n",
                 process.id,
                 process.name.c_str(),
                 get_process_state_string(process.state),
                 process.priority,
                 process.stack_high_water_mark,
                 process.runtime,
                 (process.core_id == 0xFF) ? "\"Any\"" : std::to_string(process.core_id).c_str(),
                 (i < processes.size() - 1) ? "," : "");

        if (current_length + strlen(process_json) >= buffer_size - 50) {
            break;
        }

        strcat(buffer, process_json);
        current_length = strlen(buffer);
    }

    strcat(buffer, "  ],\n");
    char footer[64];
    snprintf(footer, sizeof(footer), "  \"total_count\": %zu\n}\n", processes.size());
    strcat(buffer, footer);

    return buffer;
}

void systemMonitor::format_single_process_info(const process_info_t &process, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return;
    }

    // Get state string
    const char* state_str = get_process_state_string(process.state);

    // Get core string
    char core_str[8];
    if (process.core_id == 0xFF) {
        strcpy(core_str, "Any");
    } else {
        snprintf(core_str, sizeof(core_str), "%d", process.core_id);
    }

    // Truncate name if too long
    char truncated_name[21];
    if (process.name.length() > 20) {
        strncpy(truncated_name, process.name.c_str(), 17);
        truncated_name[17] = '.';
        truncated_name[18] = '.';
        truncated_name[19] = '.';
        truncated_name[20] = '\0';
    } else {
        strcpy(truncated_name, process.name.c_str());
    }

    // Format the line
    snprintf(buffer, buffer_size,
             "%-4lu | %-20s | %-9s | %-8d | %-5lu | %-7lu | %-4s\n",
             process.id,
             truncated_name,
             state_str,
             process.priority,
             process.stack_high_water_mark,
             process.runtime,
             core_str);

}

const char * systemMonitor::get_process_state_string(process_state_t state) {
    switch (state) {
        case PROCESS_STATE_READY:     return "Ready";
        case PROCESS_STATE_RUNNING:   return "Running";
        case PROCESS_STATE_BLOCKED:   return "Blocked";
        case PROCESS_STATE_SUSPENDED: return "Suspended";
        case PROCESS_STATE_DELETED:   return "Deleted";
        default:                      return "Unknown";
    }

}

systemMonitor monitor;

