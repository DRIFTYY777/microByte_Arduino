#ifndef DISPLAYHAL_H
#define DISPLAYHAL_H

#include <lvgl.h>

void display_hall_init();
void clear_screen();

#ifdef __cplusplus
extern "C"
{
#endif

    uint16_t *display_HAL_get_buffer();
    size_t display_HAL_get_buffer_size();
    void display_HAL_boot_frame(uint16_t *buffer);
    void display_HAL_change_endian();

#ifdef __cplusplus
}
#endif

void display_HAL_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

#endif // DISPLAYHAL_H