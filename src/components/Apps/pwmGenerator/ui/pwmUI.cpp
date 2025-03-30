#include "pwmUI.h"
#include <components/ui/helpers.h>
#include <components/ui/notificationBar.h>

static const char *TAG = "PWM UI";

void createPWMscreen()
{
    // Create a new screen
    lv_obj_t *new_screen = lv_obj_create(NULL);
    lv_obj_set_size(new_screen, 300, 180);
    lv_obj_align(new_screen, LV_ALIGN_CENTER, 0, 0);

    // title bar
    notificationBar(new_screen, "PWM Generator", NULL);
    lv_obj_clear_flag(new_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Frequency Slider */
    lv_obj_t *freq_slider = lv_slider_create(new_screen);
    lv_obj_set_size(freq_slider, 200, 20);
    lv_obj_align(freq_slider, LV_ALIGN_CENTER, 0, -20);
    lv_slider_set_range(freq_slider, 100, 10000);

    /* Duty Cycle Slider */
    lv_obj_t *duty_slider = lv_slider_create(new_screen);
    lv_obj_set_size(duty_slider, 200, 20);
    lv_obj_align(duty_slider, LV_ALIGN_CENTER, 0, 20);
    lv_slider_set_range(duty_slider, 0, 100);

    /* Frequency Label */
    lv_obj_t *freq_label = lv_label_create(new_screen);
    lv_label_set_text(freq_label, "Frequency: 1000Hz");
    lv_obj_align(freq_label, LV_ALIGN_CENTER, 0, -50);

    /* Duty Cycle Label */
    lv_obj_t *duty_label = lv_label_create(new_screen);
    lv_label_set_text(duty_label, "Duty Cycle: 50%");
    lv_obj_align(duty_label, LV_ALIGN_CENTER, 0, 50);

    /* Graph for pwm*/
    lv_obj_t *chart = lv_chart_create(new_screen);
    lv_obj_set_size(chart, 300, 100);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 80);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    // lv_chart_set_range(chart, 0, 100);
    lv_chart_set_point_count(chart, 10);
    lv_chart_set_div_line_count(chart, 3, 3);

    lv_scr_load(new_screen);
}