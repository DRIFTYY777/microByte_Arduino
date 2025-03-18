#include "displayHal.h"

#include <components/system_config/system_config.h>

extern "C"
{
#include "ST7789_driver.h"
}

st7789_driver_t display = {
    .pin_reset = DSP_RST,
    .pin_dc = DSP_DC,
    .pin_cs = DSP_CS,
    .pin_mosi = HSPI_MOSI,
    .pin_miso = -1,
    .pin_sclk = HSPI_CLK,
    .spi_host = SPI3_HOST,
    .spi_speed = DSP_CLK_SPEED,
    .dma_chan = 1,
    .display_width = SCR_WIDTH,
    .display_height = SCR_HEIGHT,
    .buffer_size = 20 * 240, // 2 buffers with 20 lines
};

void display_hall_init()
{
    ST7789_init(&display);
    clear_screen();
    ST7789_rotate_display(&display, 1);
}

void clear_screen()
{
    ST7789_fill_area(&display, 0x0000, 0, 0, display.display_width, display.display_height);
}

void display_HAL_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

    // Set the area to print on the screen
    ST7789_set_window(&display, area->x1, area->y1, area->x2, area->y2);

    // Save the buffer data and the size of the data to send
    display.current_buffer = (st7789_color_t *)color_p;
    display.buffer_size = size;

    // Send it
    // ST7789_write_pixels(&display, display.current_buffer, display.buffer_size);
    ST7789_swap_buffers(&display);

    // Tell to LVGL that is ready to send another frame
    lv_disp_flush_ready(disp_drv);
}