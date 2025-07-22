#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>
#include <vector>
#include <map>
#include <string>
#include "processMemory.h"

// Process states
typedef enum
{
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_SUSPENDED,
    PROCESS_STATE_DELETED,
    PROCESS_STATE_UNKNOWN
} process_state_t;

// Process priority levels
typedef enum
{
    PROCESS_PRIORITY_IDLE = 0,
    PROCESS_PRIORITY_LOW = 1,
    PROCESS_PRIORITY_NORMAL = 2,
    PROCESS_PRIORITY_HIGH = 3,
    PROCESS_PRIORITY_CRITICAL = 4
} process_priority_t;

// Process information structure
typedef struct
{
    TaskHandle_t handle;
    std::string name;
    uint32_t id;
    process_state_t state;
    uint8_t priority;
    uint32_t stack_size;
    uint32_t stack_high_water_mark;
    uint32_t runtime;
    uint8_t core_id;
    uint32_t created_time;
    bool is_monitored;
} process_info_t;

// Resource usage structure
typedef struct
{
    uint32_t total_heap;
    uint32_t free_heap;
    uint32_t min_free_heap;
    uint32_t largest_free_block;
    uint32_t total_psram;
    uint32_t free_psram;
    uint8_t cpu_usage_core0;
    uint8_t cpu_usage_core1;
    uint16_t task_count;
} system_resources_t;

// Process manager callbacks
typedef void (*process_created_callback_t)(const process_info_t *process);
typedef void (*process_deleted_callback_t)(uint32_t process_id);
typedef void (*resource_alert_callback_t)(const system_resources_t *resources);

class ProcessManager
{
private:
    std::map<uint32_t, process_info_t> processes;
    std::vector<process_created_callback_t> creation_callbacks;
    std::vector<process_deleted_callback_t> deletion_callbacks;
    std::vector<resource_alert_callback_t> resource_callbacks;

    TaskHandle_t monitor_task_handle;
    TimerHandle_t monitor_timer;
    SemaphoreHandle_t processes_mutex;

    bool monitoring_enabled;
    uint32_t monitor_interval_ms;
    uint32_t next_process_id;

    // Resource thresholds
    uint32_t heap_warning_threshold;
    uint32_t stack_warning_threshold;

    // static void monitor_task(void* parameter);
    static void monitor_timer_callback(TimerHandle_t timer);

    void update_process_info(process_info_t &process);
    void check_resource_thresholds();
    process_state_t get_task_state(eTaskState freertos_state);

public:
    ProcessManager();
    ~ProcessManager();

    // Initialization and configuration
    bool init();
    void set_monitor_interval(uint32_t interval_ms);
    void set_heap_warning_threshold(uint32_t threshold);
    void set_stack_warning_threshold(uint32_t threshold);

    // Process management
    uint32_t create_process(const char *name, TaskFunction_t task_function,
                            uint32_t stack_size, void *parameters,
                            process_priority_t priority, BaseType_t core_id = tskNO_AFFINITY);

    bool delete_process(uint32_t process_id);
    bool delete_process_by_name(const char *name);
    bool suspend_process(uint32_t process_id);
    bool resume_process(uint32_t process_id);
    bool set_process_priority(uint32_t process_id, process_priority_t priority);

    //

    // Process monitoring
    bool start_monitoring();
    bool stop_monitoring();
    void update_all_processes();

    // Process information
    std::vector<process_info_t> get_all_processes();
    process_info_t *get_process_info(uint32_t process_id);
    process_info_t *get_process_info_by_name(const char *name);
    uint16_t get_process_count();

    // System resources
    system_resources_t get_system_resources();
    void print_process_table();
    void print_system_resources();

    // Callback management
    void register_creation_callback(process_created_callback_t callback);
    void register_deletion_callback(process_deleted_callback_t callback);
    void register_resource_callback(resource_alert_callback_t callback);

    // Utility functions
    uint32_t get_total_cpu_usage();
    uint32_t get_process_cpu_usage(uint32_t process_id);
    bool is_process_alive(uint32_t process_id);
    void cleanup_dead_processes();
    // uint32_t process_get_stack_usage(uint32_t process_id);

    bool process_send_message(uint32_t process_id, const void *message, size_t size);
};

extern ProcessManager process_manager;

#endif // PROCESS_MANAGER_H