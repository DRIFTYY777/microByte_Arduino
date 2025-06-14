/*

https://maximeborges.github.io/esp-stacktrace-decoder/
*/

#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>

#include <components/drivers/backlight/backlight.h>
#include <components/drivers/display/displayHal.h>
#include <components/drivers/wifi/connections.h>

#include <components/core/processManager.h>

// #include <components/drivers/battery/battery.h>
#include <esp_task_wdt.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/LED/LED_notification.h>
#include <components/drivers/sd_card/sd_card.h>
#include <components/drivers/sound/sound.h>
#include <components/drivers/vb/vibration.h>
#include <components/drivers/time/LocalTime.h>
#include <components/drivers/sd_card/formatSD.h>
#include <components/drivers/nrf24/radioHall.h>

#include <components/system_config/system_config.h>
#include <components/system_config/system_manager.h>

#include "components/ui/ui.h"
#include <components/Apps/EvilApple/evilApple.h>
#include <components/external_app/external_app.h>
#include <components/ota/update_firmware.h>
#include <components/emulators/NES/NesManager.h>
#include <components/emulators/GBC/GboyManager.h>

#include "components/drivers/spiManager/spiManager.h"

extern "C"
{
#include <components/boot/boot_screen.h>
}

/*

Amazing Spider-Man.gb
[ 22059][E][vfs_api.cpp:105]
 open(): /sd/Emulator/GameBoy/Save_Data/Amazing Spider-Man.gb.sav does not exist, no permits for creation
 gbu loader.c
*/

#include <SD.h>

/*
    LVGL Version: 8.3.9
    old TFT_eSPI: 2.5.43 // not stable
*/

/// add blue ducky
/// MAX9814 mic
/// arduboy




TaskHandle_t gui_handler;
TaskHandle_t intro_handler;
uint32_t gui_process_id = 0;
uint32_t main_process_id = 0;

static const char *TAG = "Main";

uint8_t console_running;
bool boot_screen_ani = true;
bool game_running = false;
bool game_executed = false;



// Process manager callback functions
void on_process_created(const process_info_t* process) {
    ESP_LOGI(TAG, "Process created callback: %s (ID: %lu)", process->name.c_str(), process->id);
}

void on_process_deleted(uint32_t process_id) {
    ESP_LOGI(TAG, "Process deleted callback: ID %lu", process_id);
}

void on_resource_alert(const system_resources_t* resources) {
    ESP_LOGW(TAG, "Resource alert! Free heap: %lu bytes, Task count: %d",
             resources->free_heap, resources->task_count);
}

// System monitoring task
[[noreturn]] void system_monitor_task(void* parameter) {
    const TickType_t monitor_delay = pdMS_TO_TICKS(30000); // 30 seconds

    while (true) {
        // Print system status
        ESP_LOGI(TAG, "=== System Status ===");
        process_manager.print_system_resources();
        process_manager.print_process_table();

        // Cleanup dead processes
        process_manager.cleanup_dead_processes();
        vTaskDelay(monitor_delay);
    }
}


void setup()
{
    Serial.begin(115200);
    Serial.print("\n");

    // Initialize Process Manager first
    if (!process_manager.init()) {
        ESP_LOGE(TAG, "Failed to initialize Process Manager");
        return;
    }

    // Register process manager callbacks
    process_manager.register_creation_callback(on_process_created);
    process_manager.register_deletion_callback(on_process_deleted);
    process_manager.register_resource_callback(on_resource_alert);

    // Set resource thresholds
    process_manager.set_heap_warning_threshold(20480); // 20KB
    process_manager.set_stack_warning_threshold(1024);  // 1KB

    // Start process monitoring
    process_manager.start_monitoring();

    // Initialize the SPI bus once in your main application
    spi_bus_manager_init(VSPI_HOST, HSPI_MOSI, HSPI_MISO, HSPI_CLK,  19200); // precalculated buffer;

    /* Init of Wi-Fi */
    WIFI_CONNECTIONS::wifi_init();

    /* Increase of Watchdog */
    esp_task_wdt_init(10, true); // 10-second timeout

    /* System Init for hardware state */
    sys_manager.system_init_config();
    sys_manager.system_info();
    app.mode = MODE_NONE;

    /* Internal RTC  Init. */
    local_time.init();

    /* 1 Sec Delay */
    vTaskDelay(500 / portTICK_RATE_MS);

    /* SD Card */
    sd_card.sd_init();

    /* Responsible for User Input */
    user_input.input_init();

    /* Display Drivers Init. */
    display_hall_init();

    /* Radio */
    RadioHall::init();

    /* 1 Sec Delay */
    vTaskDelay(500 / portTICK_RATE_MS);

    /* Testing NRF */
    Serial.println(RadioHall::connected() ? "NRF24L01 connected" : "NRF24L01 not connected");

    /* Init LED for Notification */
    LED_NOTIFICATION::LED_init();


    ESP_LOGE(TAG, "Memory Status:\r\n -SPI_RAM: %i Bytes\r\n -INTERNAL_RAM: %i Bytes\r\n -DMA_RAM: %i Bytes\r\n",
             sys_manager.system_memory(MEMORY_SPIRAM),
             sys_manager.system_memory(MEMORY_INTERNAL),
             sys_manager.system_memory(MEMORY_DMA));

    /* Init of Display Backlight */
    BACKLIGHT::backlight_init();

    /* Lvgl driver init */
    ui_init();
    // xTaskCreatePinnedToCore(GUI_task, "Graphical User Interface", 1024 * 8, nullptr, 2, &gui_handler, 0);

    gui_process_id = process_manager.create_process("GUI_Task", GUI_task, 1024 * 8,
                                                   nullptr, PROCESS_PRIORITY_HIGH, 0);
    if (gui_process_id == 0) {
        ESP_LOGE(TAG, "Failed to create GUI process");
        return;
    }
    // Create a system monitoring task
    const uint32_t monitor_process_id = process_manager.create_process("SystemMonitor", system_monitor_task,
                                                               1024 * 4, nullptr, PROCESS_PRIORITY_LOW, 1);
    if (monitor_process_id == 0) {
        ESP_LOGW(TAG, "Failed to create system monitor process");
    }

    /* Init of GUI */
    GUI_frontend();

    /* Init of NTP for time sync */
    local_time.init_NTCP();

    /* Queue for creating or ... */
    modeQueue = xQueueCreate(1, sizeof(app));
}

void loop()
{
    if (xQueueReceive(modeQueue, &app, portMAX_DELAY) == pdTRUE) {
        if (app.mode == MODE_APPLEJUICE) {
            if (app.status == STATUS_RUNNING) {
                // Create process for AppleJuice attack
                uint32_t apple_process_id = process_manager.create_process("AppleJuice",
                    [](void* param) {
                        while (app.status == STATUS_RUNNING) {
                            evilApple.startAdvertising();
                        }
                        evilApple.stopAdvertising();
                        vTaskDelete(nullptr);
                    }, 1024 * 4, nullptr, PROCESS_PRIORITY_NORMAL);

                if (apple_process_id == 0) {
                    ESP_LOGE(TAG, "Failed to create AppleJuice process");
                }
            }
        }
        else if (app.mode == MODE_EXT_APP) {
            if (app.status == STATUS_RUNNING) {
                ESP_LOGI(TAG, "Loading external App");

                // Suspend GUI process before switching
                if (gui_process_id != 0) {
                    process_manager.suspend_process(gui_process_id);
                }

                vTaskDelay(1000 / portTICK_RATE_MS);
                external_app.external_app_init(app.aap_name);
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
            }
        }
        else if (app.mode == MODE_UPDATE) {
            if (app.status == STATUS_RUNNING) {
                ESP_LOGE(TAG, "Loading OTA");

                // Stop monitoring during OTA update
                process_manager.stop_monitoring();

                // Suspend GUI process
                if (gui_process_id != 0) {
                    process_manager.suspend_process(gui_process_id);
                }

                vTaskDelay(1000 / portTICK_RATE_MS);
                update_firmware.update_init(app.aap_name);
                vTaskDelay(250 / portTICK_RATE_MS);
                esp_restart();
            }
        }
        else if (app.mode == MODE_GAME) {
            if (app.console == NES) {
                if (app.status == STATUS_RUNNING) {
                    // Create NES emulator process
                    uint32_t nes_process_id = process_manager.create_process("NES_Emulator",
                        [](void* param) {
                            if (gui_process_id != 0) {
                                process_manager.suspend_process(gui_process_id);
                            }
                            display_set_NES();
                            NES_start(app.aap_name);
                            if (app.load_save_game) {
                                vTaskDelay(1500 / portTICK_RATE_MS);
                                NES_load_game();
                            }
                            game_executed = true;
                            game_running = true;
                            vTaskDelete(nullptr);
                        }, 1024 * 16, nullptr, PROCESS_PRIORITY_HIGH);

                    if (nes_process_id == 0) {
                        ESP_LOGE(TAG, "Failed to create NES emulator process");
                    }
                }
            }
            else if (app.console == GAMEBOY_COLOR || app.console == GAMEBOY) {
                if (app.status == STATUS_RUNNING) {
                    // Create GameBoy emulator process
                    uint32_t gb_process_id = process_manager.create_process("GB_Emulator",
                        [](void* param) {
                            if (gui_process_id != 0) {
                                process_manager.suspend_process(gui_process_id);
                            }
                            gnuboy_execute_game(app.aap_name, app.console, app.load_save_game);
                            gnuboy_start();
                            game_executed = true;
                            game_running = true;
                            vTaskDelete(nullptr);
                        }, 1024 * 16, nullptr, PROCESS_PRIORITY_HIGH);

                    if (gb_process_id == 0) {
                        ESP_LOGE(TAG, "Failed to create GameBoy emulator process");
                    }
                }
            }
        }
    }
}