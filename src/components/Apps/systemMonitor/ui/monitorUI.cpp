//
// Created by dhima on 18-06-2025.
//

#include "monitorUI.h"
#include <components/ui/helpers.h>
#include <components/Apps/systemMonitor/systemMonitor.h>

// void createMonitorScreen() {
//     monitor.init();
//
//     // Create a scrollable root container
//     lv_obj_t *cont = lv_obj_create(nullptr);
//     lv_obj_set_size(cont, 320, 240);
//     lv_obj_set_scroll_dir(cont, LV_DIR_HOR | LV_DIR_VER); // Enable both scrolls
//     lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);
//     lv_obj_center(cont);
//
//     // Inner container for horizontal content
//     lv_obj_t *inner_cont = lv_obj_create(cont);
//     lv_obj_set_size(inner_cont, 600, 240); // Wider than 320 to allow horizontal scroll
//     lv_obj_set_scrollbar_mode(inner_cont, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_set_scroll_dir(inner_cont, LV_DIR_VER);
//     lv_obj_set_style_pad_all(inner_cont, 0, 0);
//     lv_obj_clear_flag(inner_cont, LV_OBJ_FLAG_SCROLLABLE); // Keep child content fixed
//
//     // Create the table
//     lv_obj_t *table = lv_table_create(inner_cont);
//     lv_obj_set_size(table, 600, 220);  // Same width as container
//     lv_obj_center(table);
//
//     // Prevent word wrapping in cells
//     // lv_obj_set_style_text_wrap(table, LV_TEXT_WRAP_NONE, 0);
//
//     // Optional: Use smaller font
//     // lv_obj_set_style_text_font(table, &lv_font_montserrat_10, 0);
//
//     // Set up table
//     lv_table_set_col_cnt(table, 7);
//     lv_table_set_row_cnt(table, 1); // Header row
//
//     // Column widths (total ≈ 600px)
//     lv_table_set_col_width(table, 0, 40);   // ID
//     lv_table_set_col_width(table, 1, 100);  // Name
//     lv_table_set_col_width(table, 2, 60);   // State
//     lv_table_set_col_width(table, 3, 100);   // Priority
//     lv_table_set_col_width(table, 4, 80);   // Stack
//     lv_table_set_col_width(table, 5, 100);  // Runtime
//     lv_table_set_col_width(table, 6, 60);   // Core
//
//     // Header labels
//     lv_table_set_cell_value(table, 0, 0, "ID");
//     lv_table_set_cell_value(table, 0, 1, "Name");
//     lv_table_set_cell_value(table, 0, 2, "State");
//     lv_table_set_cell_value(table, 0, 3, "Priority");
//     lv_table_set_cell_value(table, 0, 4, "Stack");
//     lv_table_set_cell_value(table, 0, 5, "Runtime");
//     lv_table_set_cell_value(table, 0, 6, "Core");
//
//     // Fill table rows
//     process_manager.update_all_processes();
//     std::vector<process_info_t> processes = process_manager.get_all_processes();
//
//     for (size_t i = 0; i < processes.size(); i++) {
//         const auto& p = processes[i];
//         uint32_t row = i + 1;
//         lv_table_set_row_cnt(table, row + 1);
//
//         lv_table_set_cell_value_fmt(table, row, 0, "%lu", p.id);
//         lv_table_set_cell_value_fmt(table, row, 1, "%s", p.name.c_str());
//         lv_table_set_cell_value_fmt(table, row, 2, "%s", monitor.get_process_state_string(p.state));
//         lv_table_set_cell_value_fmt(table, row, 3, "%d", p.priority);
//         lv_table_set_cell_value_fmt(table, row, 4, "%lu", p.stack_high_water_mark);
//         lv_table_set_cell_value_fmt(table, row, 5, "%lu", p.runtime);
//         lv_table_set_cell_value_fmt(table, row, 6, "%s", (p.core_id == 0xFF) ? "Any" : std::to_string(p.core_id).c_str());
//     }
//
//     lv_scr_load(cont);
// }

// LV_KEY_ESC btn is pressed go to main menu
// LV_KEY_ENTER btn is pressed go to process details


void createMonitorScreen() {
    monitor.init();
    isInMenu = false;

    // // Root screen
    // lv_obj_t *scr = lv_obj_create(nullptr);
    // lv_obj_set_size(scr, 320, 240);
    // lv_obj_set_scroll_dir(scr, LV_DIR_ALL);
    // lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_AUTO);

    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(nullptr);
    lv_obj_set_size(new_screen, 320, 240);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_scroll_dir(new_screen, LV_DIR_VER);
    // lv_obj_set_scroll_dir(new_screen, LV_DIR_HOR);
    // lv_obj_set_scrollbar_mode(new_screen, LV_SCROLLBAR_MODE_AUTO);


    // Table
    lv_obj_t *table = lv_table_create(new_screen);
    lv_obj_set_size(table, 700, 220);
    lv_obj_set_pos(table, 0, 0);
    lv_obj_add_flag(table, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_scroll_dir(table, LV_DIR_VER);
    lv_obj_set_scroll_dir(table, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(table, LV_SCROLLBAR_MODE_AUTO);

    lv_table_set_col_cnt(table, 7);
    lv_table_set_row_cnt(table, 1);
    lv_table_set_col_width(table, 0, 40);
    lv_table_set_col_width(table, 1, 120);
    lv_table_set_col_width(table, 2, 100);
    lv_table_set_col_width(table, 3, 50);
    lv_table_set_col_width(table, 4, 100);
    lv_table_set_col_width(table, 5, 180);
    lv_table_set_col_width(table, 6, 60);

    lv_table_set_cell_value(table, 0, 0, "ID");
    lv_table_set_cell_value(table, 0, 1, "Name");
    lv_table_set_cell_value(table, 0, 2, "State");
    lv_table_set_cell_value(table, 0, 3, "Prio");
    lv_table_set_cell_value(table, 0, 4, "Stack");
    lv_table_set_cell_value(table, 0, 5, "Runtime");
    lv_table_set_cell_value(table, 0, 6, "Core");

    process_manager.update_all_processes();
    std::vector<process_info_t> processes = process_manager.get_all_processes();

    for (size_t i = 0; i < processes.size(); i++) {
        const auto &p = processes[i];
        uint32_t row = i + 1;
        lv_table_set_row_cnt(table, row + 1);

        lv_table_set_cell_value_fmt(table, row, 0, "%lu", p.id);
        lv_table_set_cell_value_fmt(table, row, 1, "%s", p.name.c_str());
        lv_table_set_cell_value_fmt(table, row, 2, "%s", monitor.get_process_state_string(p.state));
        lv_table_set_cell_value_fmt(table, row, 3, "%d", p.priority);
        lv_table_set_cell_value_fmt(table, row, 4, "%lu", p.stack_high_water_mark);
        lv_table_set_cell_value_fmt(table, row, 5, "%lu", p.runtime);
        lv_table_set_cell_value_fmt(table, row, 6, "%s", (p.core_id == 0xFF) ? "Any" : std::to_string(p.core_id).c_str());
    }

    // Focus handling
    static lv_group_t *g = lv_group_create();
    lv_group_add_obj(g, table);
    lv_group_focus_obj(table);

    lv_indev_t *indev = lv_indev_get_next(nullptr);
    while (indev) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER ||
            lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(indev, g);
        }
        indev = lv_indev_get_next(indev);
    }

    // Add key event listener
    lv_obj_add_event_cb(table, [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_KEY) {
            uint32_t key = lv_indev_get_key(lv_indev_get_act());
            if (key == LV_KEY_ESC) {
                ESP_LOGI("MONITOR", "Back button pressed from UI");
                monitor.deinit();  // Clean up monitor resources

                backToMenu(e); // Go back to main menu
            }
        }
    }, LV_EVENT_KEY, nullptr);


    lv_scr_load(new_screen);

}




