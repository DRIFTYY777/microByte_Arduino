#ifndef DISPLAYHAL_H
#define DISPLAYHAL_H

#include <lvgl.h>



void display_hall_init();
void clear_screen();

void display_HAL_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);


#endif // DISPLAYHAL_H