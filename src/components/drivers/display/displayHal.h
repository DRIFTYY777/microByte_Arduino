#ifndef DISPLAYHAL_H
#define DISPLAYHAL_H

#include <lvgl.h>

#define LINE_BUFFERS (10) // 2
#define LINE_COUNT (20)

#define NES_FRAME_WIDTH 256 /// 256 original
#define NES_FRAME_HEIGHT 240

#define GBC_FRAME_WIDTH 160
#define GBC_FRAME_HEIGHT 144

#define SMS_FRAME_WIDTH 256
#define SMS_FRAME_HEIGHT 192

#define GG_FRAME_WIDTH 160
#define GG_FRAME_HEIGHT 144

#define PIXEL_MASK (0x1F)

extern uint16_t myPalette[];

void display_hall_init();

#ifdef __cplusplus
extern "C"
{
#endif
    void clear_screen();
    uint16_t *display_HAL_get_buffer();
    size_t display_HAL_get_buffer_size();
    void display_HAL_boot_frame(uint16_t *buffer);
    void display_HAL_change_endian(bool change);
    void display_HAL_NES_frame(const uint8_t *data);
    void display_HAL_gb_frame(const uint16_t *data);

#ifdef __cplusplus
}
#endif

void display_HAL_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void display_set_NES();
void display_set_GB();

#endif // DISPLAYHAL_H