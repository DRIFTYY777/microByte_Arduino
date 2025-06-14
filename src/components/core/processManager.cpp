#include "processManager.h"

static const char* TAG = "ProcessManager";

ProcessManager process_manager;

ProcessManager::ProcessManager() {
    monitoring_enabled = false;
    monitor_interval_ms = 1000; // 1 second default
    next_process_id = 1;
    heap_warning_threshold = 10240; // 10KB default
    stack_warning_threshold = 512;  // 512 bytes default
    monitor_task_handle = nullptr;
    monitor_timer = nullptr;
    processes_mutex = nullptr;
}

ProcessManager::~ProcessManager() {
    stop_monitoring();
    if (processes_mutex) {
        vSemaphoreDelete(processes_mutex);
    }
}

bool ProcessManager::init() {
    // Create mutex for thread-safe access to processes map
    processes_mutex = xSemaphoreCreateMutex();
    if (!processes_mutex) {
        ESP_LOGE(TAG, "Failed to create processes mutex");
        return false;
    }

    // Create monitoring timer
    monitor_timer = xTimerCreate("ProcessMonitorTimer",
                               pdMS_TO_TICKS(monitor_interval_ms),
                               pdTRUE, // Auto-reload
                               this,   // Timer ID
                               monitor_timer_callback);

    if (!monitor_timer) {
        ESP_LOGE(TAG, "Failed to create monitor timer");
        return false;
    }

    ESP_LOGI(TAG, "Process Manager initialized successfully");
    return true;
}

void ProcessManager::set_monitor_interval(uint32_t interval_ms) {
    monitor_interval_ms = interval_ms;
    if (monitor_timer) {
        xTimerChangePeriod(monitor_timer, pdMS_TO_TICKS(interval_ms), portMAX_DELAY);
    }
}

void ProcessManager::set_heap_warning_threshold(uint32_t threshold) {
    heap_warning_threshold = threshold;
}

void ProcessManager::set_stack_warning_threshold(uint32_t threshold) {
    stack_warning_threshold = threshold;
}

uint32_t ProcessManager::create_process(const char* name, TaskFunction_t task_function,
                                      uint32_t stack_size, void* parameters,
                                      process_priority_t priority, BaseType_t core_id) {

    TaskHandle_t task_handle;
    BaseType_t result;

    // Convert priority to FreeRTOS priority
    UBaseType_t freertos_priority = priority + 1; // FreeRTOS priorities start from 1

    if (core_id == tskNO_AFFINITY) {
        result = xTaskCreate(task_function, name, stack_size, parameters,
                           freertos_priority, &task_handle);
    } else {
        result = xTaskCreatePinnedToCore(task_function, name, stack_size, parameters,
                                       freertos_priority, &task_handle, core_id);
    }

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create process: %s", name);
        return 0;
    }

    // Create process info
    process_info_t process_info;
    process_info.handle = task_handle;
    process_info.name = std::string(name);
    process_info.id = next_process_id++;
    process_info.state = PROCESS_STATE_READY;
    process_info.priority = priority;
    process_info.stack_size = stack_size;
    process_info.stack_high_water_mark = 0;
    process_info.runtime = 0;
    process_info.core_id = (core_id == tskNO_AFFINITY) ? 0xFF : core_id;
    process_info.created_time = xTaskGetTickCount();
    process_info.is_monitored = true;

    // Add to processes map
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        processes[process_info.id] = process_info;
        xSemaphoreGive(processes_mutex);

        // Notify callbacks
        for (auto callback : creation_callbacks) {
            callback(&process_info);
        }

        ESP_LOGI(TAG, "Process created: %s (ID: %lu)", name, process_info.id);
        return process_info.id;
    }

    // If we couldn't add to map, delete the task
    vTaskDelete(task_handle);
    return 0;
}

bool ProcessManager::delete_process(uint32_t process_id) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.find(process_id);
        if (it != processes.end()) {
            TaskHandle_t handle = it->second.handle;
            processes.erase(it);
            xSemaphoreGive(processes_mutex);

            // Delete the FreeRTOS task
            vTaskDelete(handle);

            // Notify callbacks
            for (auto callback : deletion_callbacks) {
                callback(process_id);
            }

            ESP_LOGI(TAG, "Process deleted: ID %lu", process_id);
            return true;
        }
        xSemaphoreGive(processes_mutex);
    }

    ESP_LOGW(TAG, "Process not found for deletion: ID %lu", process_id);
    return false;
}

bool ProcessManager::suspend_process(uint32_t process_id) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.find(process_id);
        if (it != processes.end()) {
            vTaskSuspend(it->second.handle);
            it->second.state = PROCESS_STATE_SUSPENDED;
            xSemaphoreGive(processes_mutex);
            ESP_LOGI(TAG, "Process suspended: %s", it->second.name.c_str());
            return true;
        }
        xSemaphoreGive(processes_mutex);
    }
    return false;
}

bool ProcessManager::resume_process(uint32_t process_id) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.find(process_id);
        if (it != processes.end()) {
            vTaskResume(it->second.handle);
            it->second.state = PROCESS_STATE_READY;
            xSemaphoreGive(processes_mutex);
            ESP_LOGI(TAG, "Process resumed: %s", it->second.name.c_str());
            return true;
        }
        xSemaphoreGive(processes_mutex);
    }
    return false;
}

bool ProcessManager::set_process_priority(uint32_t process_id, process_priority_t priority) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.find(process_id);
        if (it != processes.end()) {
            UBaseType_t freertos_priority = priority + 1;
            vTaskPrioritySet(it->second.handle, freertos_priority);
            it->second.priority = priority;
            xSemaphoreGive(processes_mutex);
            ESP_LOGI(TAG, "Process priority changed: %s -> %d", it->second.name.c_str(), priority);
            return true;
        }
        xSemaphoreGive(processes_mutex);
    }
    return false;
}

bool ProcessManager::start_monitoring() {
    if (!monitor_timer) {
        ESP_LOGE(TAG, "Monitor timer not initialized");
        return false;
    }

    monitoring_enabled = true;
    if (xTimerStart(monitor_timer, 0) == pdPASS) {
        ESP_LOGI(TAG, "Process monitoring started");
        return true;
    }

    ESP_LOGE(TAG, "Failed to start monitoring timer");
    return false;
}

bool ProcessManager::stop_monitoring() {
    monitoring_enabled = false;
    if (monitor_timer && xTimerStop(monitor_timer, 0) == pdPASS) {
        ESP_LOGI(TAG, "Process monitoring stopped");
        return true;
    }
    return false;
}

void ProcessManager::monitor_timer_callback(TimerHandle_t timer) {
    ProcessManager* pm = static_cast<ProcessManager*>(pvTimerGetTimerID(timer));
    if (pm && pm->monitoring_enabled) {
        pm->update_all_processes();
        pm->check_resource_thresholds();
    }
}

void ProcessManager::update_all_processes() {
    if (xSemaphoreTake(processes_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (auto& pair : processes) {
            update_process_info(pair.second);
        }
        xSemaphoreGive(processes_mutex);
    }
}

void ProcessManager::update_process_info(process_info_t& process) {
    eTaskState task_state = eTaskGetState(process.handle);

    if (task_state == eDeleted) {
        process.state = PROCESS_STATE_DELETED;
        return;
    }

    process.state = get_task_state(task_state);

    // Get stack high water mark
    process.stack_high_water_mark = uxTaskGetStackHighWaterMark(process.handle);

    // Runtime statistics would require additional FreeRTOS configuration
    // For now, we'll use a placeholder
    process.runtime = xTaskGetTickCount() - process.created_time;
}

process_state_t ProcessManager::get_task_state(eTaskState freertos_state) {
    switch (freertos_state) {
        case eReady: return PROCESS_STATE_READY;
        case eRunning: return PROCESS_STATE_RUNNING;
        case eBlocked: return PROCESS_STATE_BLOCKED;
        case eSuspended: return PROCESS_STATE_SUSPENDED;
        case eDeleted: return PROCESS_STATE_DELETED;
        default: return PROCESS_STATE_UNKNOWN;
    }
}

void ProcessManager::check_resource_thresholds() {
    system_resources_t resources = get_system_resources();

    bool alert_triggered = false;

    // Check heap threshold
    if (resources.free_heap < heap_warning_threshold) {
        ESP_LOGW(TAG, "Low heap warning: %lu bytes free (threshold: %lu)",
                 resources.free_heap, heap_warning_threshold);
        alert_triggered = true;
    }

    // Check stack usage for all processes
    if (xSemaphoreTake(processes_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (const auto& pair : processes) {
            if (pair.second.stack_high_water_mark < stack_warning_threshold) {
                ESP_LOGW(TAG, "Low stack warning for %s: %lu bytes remaining",
                         pair.second.name.c_str(), pair.second.stack_high_water_mark);
                alert_triggered = true;
            }
        }
        xSemaphoreGive(processes_mutex);
    }

    // Notify resource callbacks if alert triggered
    if (alert_triggered) {
        for (auto callback : resource_callbacks) {
            callback(&resources);
        }
    }
}

std::vector<process_info_t> ProcessManager::get_all_processes() {
    std::vector<process_info_t> result;

    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        for (const auto& pair : processes) {
            result.push_back(pair.second);
        }
        xSemaphoreGive(processes_mutex);
    }

    return result;
}

process_info_t* ProcessManager::get_process_info(uint32_t process_id) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.find(process_id);
        if (it != processes.end()) {
            process_info_t* result = &it->second;
            xSemaphoreGive(processes_mutex);
            return result;
        }
        xSemaphoreGive(processes_mutex);
    }
    return nullptr;
}

process_info_t* ProcessManager::get_process_info_by_name(const char* name) {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        for (auto& pair : processes) {
            if (pair.second.name == name) {
                process_info_t* result = &pair.second;
                xSemaphoreGive(processes_mutex);
                return result;
            }
        }
        xSemaphoreGive(processes_mutex);
    }
    return nullptr;
}

uint16_t ProcessManager::get_process_count() {
    uint16_t count = 0;
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        count = processes.size();
        xSemaphoreGive(processes_mutex);
    }
    return count;
}

system_resources_t ProcessManager::get_system_resources() {
    system_resources_t resources = {0};

    // Heap information
    resources.total_heap = ESP.getHeapSize();
    resources.free_heap = ESP.getFreeHeap();
    resources.min_free_heap = ESP.getMinFreeHeap();
    resources.largest_free_block = ESP.getMaxAllocHeap();

    // PSRAM information (if available)
    if (ESP.getPsramSize() > 0) {
        resources.total_psram = ESP.getPsramSize();
        resources.free_psram = ESP.getFreePsram();
    } else {
        resources.total_psram = 0;
        resources.free_psram = 0;
    }

    // Task count
    resources.task_count = uxTaskGetNumberOfTasks();

    // CPU usage would require additional implementation with runtime stats
    resources.cpu_usage_core0 = 0; // Placeholder
    resources.cpu_usage_core1 = 0; // Placeholder

    return resources;
}

void ProcessManager::print_process_table() {
    Serial.println("\n=== Process Table ===");
    Serial.printf("%-4s %-20s %-8s %-10s %-8s %-8s %-12s\n",
                  "ID", "Name", "State", "Priority", "Stack", "Runtime", "Core");
    Serial.println("--------------------------------------------------------------------");

    std::vector<process_info_t> processes_list = get_all_processes();
    for (const auto& process : processes_list) {
        const char* state_str;
        switch (process.state) {
            case PROCESS_STATE_READY: state_str = "Ready"; break;
            case PROCESS_STATE_RUNNING: state_str = "Running"; break;
            case PROCESS_STATE_BLOCKED: state_str = "Blocked"; break;
            case PROCESS_STATE_SUSPENDED: state_str = "Suspend"; break;
            case PROCESS_STATE_DELETED: state_str = "Deleted"; break;
            default: state_str = "Unknown"; break;
        }

        Serial.printf("%-4lu %-20s %-8s %-10d %-8lu %-8lu %-12s\n",
                      process.id,
                      process.name.c_str(),
                      state_str,
                      process.priority,
                      process.stack_high_water_mark,
                      process.runtime,
                      (process.core_id == 0xFF) ? "Any" : String(process.core_id).c_str());
    }
    Serial.println("====================================================================\n");
}

void ProcessManager::print_system_resources() {
    system_resources_t resources = get_system_resources();

    Serial.println("\n=== System Resources ===");
    Serial.printf("Total Heap:     %lu bytes\n", resources.total_heap);
    Serial.printf("Free Heap:      %lu bytes\n", resources.free_heap);
    Serial.printf("Min Free Heap:  %lu bytes\n", resources.min_free_heap);
    Serial.printf("Largest Block:  %lu bytes\n", resources.largest_free_block);
    if (resources.total_psram > 0) {
        Serial.printf("Total PSRAM:    %lu bytes\n", resources.total_psram);
        Serial.printf("Free PSRAM:     %lu bytes\n", resources.free_psram);
    }
    Serial.printf("Task Count:     %d\n", resources.task_count);
    Serial.println("========================\n");
}

void ProcessManager::register_creation_callback(process_created_callback_t callback) {
    creation_callbacks.push_back(callback);
}

void ProcessManager::register_deletion_callback(process_deleted_callback_t callback) {
    deletion_callbacks.push_back(callback);
}

void ProcessManager::register_resource_callback(resource_alert_callback_t callback) {
    resource_callbacks.push_back(callback);
}

uint32_t ProcessManager::get_total_cpu_usage() {
    // Placeholder implementation
    // Would require runtime statistics to be enabled in FreeRTOS configuration
    return 0;
}

uint32_t ProcessManager::get_process_cpu_usage(uint32_t process_id) {
    // Placeholder implementation
    // Would require runtime statistics to be enabled in FreeRTOS configuration
    return 0;
}

bool ProcessManager::is_process_alive(uint32_t process_id) {
    process_info_t* process = get_process_info(process_id);
    return process && process->state != PROCESS_STATE_DELETED;
}

void ProcessManager::cleanup_dead_processes() {
    if (xSemaphoreTake(processes_mutex, portMAX_DELAY) == pdTRUE) {
        auto it = processes.begin();
        while (it != processes.end()) {
            if (it->second.state == PROCESS_STATE_DELETED) {
                ESP_LOGI(TAG, "Cleaning up dead process: %s", it->second.name.c_str());
                it = processes.erase(it);
            } else {
                ++it;
            }
        }
        xSemaphoreGive(processes_mutex);
    }
}