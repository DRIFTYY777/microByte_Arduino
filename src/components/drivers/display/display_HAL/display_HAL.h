

#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <cstdint>

// #include "LVGL/lvgl.h"
#include "lvgl.h"


#define BLACK 0x0000
#define WHITE 0xFFFF

#define LINE_BUFFERS (2)
#define LINE_COUNT (20)

#define GBC_FRAME_WIDTH 160
#define GBC_FRAME_HEIGHT 144

#define NES_FRAME_WIDTH 256
#define NES_FRAME_HEIGHT 240

#define SMS_FRAME_WIDTH 256
#define SMS_FRAME_HEIGHT 192

#define GG_FRAME_WIDTH 160
#define GG_FRAME_HEIGHT 144

#define PIXEL_MASK (0x1F)

extern uint16_t myPalette[];

class DisplayHAL
{
private:
    static uint16_t getPixelGBC(const uint16_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);
    static uint8_t getPixelSMS(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2, bool GAME_GEAR);
    static uint8_t getPixelNES(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);

public:
    uint16_t *line[LINE_BUFFERS];

    bool display_HAL_init(void);
    void display_HAL_clear();
    void display_HAL_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
    void display_HAL_gb_frame(const uint16_t *data);
    void display_HAL_NES_frame(const uint8_t *data);
    void display_HAL_SMS_frame(const uint8_t *data, uint16_t color[], bool GAMEGEAR);
    uint16_t *display_HAL_get_buffer();
    size_t display_HAL_get_buffer_size();
    void display_HAL_boot_frame(uint16_t *buffer);
    void display_HAL_change_endian();
};

extern DisplayHAL displayHAL;

#endif // DISPLAY_HAL_H