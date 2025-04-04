#include "pwmUI.h"
#include <components/ui/helpers.h>
#include <components/ui/notificationBar.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PWM Chart UI";

// UI Elements
static lv_obj_t *chart;
static lv_chart_series_t *series;
static lv_obj_t *label;

static int duty_cycle = 50;  // 0 - 100%
static int frequency = 1000; // Simulated, in Hz
static TaskHandle_t chartTaskHandle = NULL;

// Constants
#define POINT_COUNT 100

// Generate square wave pattern based on duty cycle
void updateSquareWaveChart()
{
    int high_points = (POINT_COUNT * duty_cycle) / 100;
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        if ((i % POINT_COUNT) < high_points)
            series->y_points[i] = 100; // High
        else
            series->y_points[i] = 0; // Low
    }

    lv_chart_refresh(chart);

    char buf[64];
    snprintf(buf, sizeof(buf), "Duty: %d%%   Freq: %dHz", duty_cycle, frequency);
    lv_label_set_text(label, buf);
}

// Handle Left/Right input
static void handle_key_input(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    if (key == LV_KEY_LEFT && duty_cycle > 0)
        duty_cycle -= 5;
    else if (key == LV_KEY_RIGHT && duty_cycle < 100)
        duty_cycle += 5;
    lv_async_call((lv_async_cb_t)updateSquareWaveChart, NULL);
}

// Chart update task
void pwmChartTask(void *pvParameters)
{
    while (1)
    {
        frequency = 900 + (esp_random() % 200); // Simulate frequency update
        lv_async_call((lv_async_cb_t)updateSquareWaveChart, NULL);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void createPWMscreen()
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 300, 240);
    lv_obj_align(screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    notificationBar(screen, "PWM Square Wave", NULL);

    // Create chart
    chart = lv_chart_create(screen);
    lv_obj_set_size(chart, 300, 160);
    lv_obj_align(chart, LV_ALIGN_TOP_MID, 0, 20);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, POINT_COUNT);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_div_line_count(chart, 2, 2);

    // Add series
    series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    // Create label
    label = lv_label_create(screen);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);

    // Button input
    lv_group_t *g = lv_group_get_default();
    if (g)
    {
        lv_group_add_obj(g, chart);
        lv_obj_add_event_cb(chart, handle_key_input, LV_EVENT_KEY, NULL);
    }

    lv_scr_load(screen);
    updateSquareWaveChart();

    // Start chart update task
    if (chartTaskHandle == NULL)
    {
        xTaskCreatePinnedToCore(pwmChartTask, "PWM Chart Task", 4096, NULL, 1, &chartTaskHandle, 1);
    }
}
