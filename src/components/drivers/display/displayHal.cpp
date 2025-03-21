#include "displayHal.h"

#include <components/system_config/system_config.h>

extern "C"
{
#include "ST7789_driver.h"
}

uint16_t *line[LINE_BUFFERS]; // Define the variable here
uint16_t myPalette[256];      // Define myPalette here if needed

static uint16_t getPixelGBC(const uint16_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);
static uint8_t getPixelSMS(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2, bool GAME_GEAR);
static uint8_t getPixelNES(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);

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
    ST7789_fill_area(&display, 0x0000, 0, 0, 320, 240);
}

uint16_t *display_HAL_get_buffer()
{
    return display.current_buffer;
}

size_t display_HAL_get_buffer_size()
{
    return display.buffer_size;
}

void display_set_NES()
{
    clear_screen();
    ST7789_invert_display(&display, false);
    display_HAL_change_endian(true);
}

void display_HAL_boot_frame(uint16_t *buffer)
{
    // The boot animation to the buffer
    display.current_buffer = buffer;
    // Send to the driver layer and change the buffer
    ST7789_swap_buffers(&display);
}

/// @brief by default the screen is little endian
/// @brief if true the screen will be big endian else little
/// @param big_endian
void display_HAL_change_endian(bool big_endian)
{
    if (big_endian)
    {
        ST7789_big_endian(&display);
    }
    else
    {
        ST7789_little_endian(&display);
    }
}

void display_HAL_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
    ST7789_set_window(&display, area->x1, area->y1, area->x2, area->y2);
    display.current_buffer = (st7789_color_t *)color_p;
    display.buffer_size = size;
    ST7789_swap_buffers(&display);
    lv_disp_flush_ready(disp_drv);
}

void display_HAL_NES_frame(const uint8_t *data)
{
    uint16_t calc_line = 0;
    uint16_t sending_line = 0;

    if (data == NULL)
    {
        for (uint16_t y = 0; y < SCR_HEIGHT; y++)
        {

            for (uint16_t x = 0; x < SCR_WIDTH; x++)
            {
                display.current_buffer[x] = 0;
            }

            sending_line = calc_line;
            calc_line = (calc_line == 1) ? 0 : 1;
            ST7789_write_lines(&display, y, 0, SCR_WIDTH, line[sending_line], 1);
        }
    }
    else
    {
        short outputHeight = 240;
        short outputWidth = 240 + (240 - 240);
        short xpos = (240 - outputWidth) / 2;

        for (int y = 0; y < outputHeight; y += LINE_COUNT)
        {
            for (int i = 0; i < LINE_COUNT; ++i)
            {
                if ((y + i) >= outputHeight)
                    break;

                int index = (i)*outputWidth;

                for (int x = 0; x < outputWidth; x++)
                {
                    display.current_buffer[index++] = myPalette[getPixelNES(data, x, (y + i), outputWidth, outputHeight)];
                }
            }

            sending_line = calc_line;
            calc_line = (calc_line == 1) ? 0 : 1;
            ST7789_write_lines(&display, y, xpos, outputWidth, line[sending_line], LINE_COUNT);
        }
    }
}

static uint8_t getPixelNES(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2)
{

    int x_diff, y_diff, xv, yv, red, green, blue, col, a, b, c, d, index;
    
    int x_ratio = (int)(((NES_FRAME_WIDTH - 1) << 16) / w2) + 1;
    int y_ratio = (int)(((NES_FRAME_HEIGHT - 1) << 16) / h2) ;

    xv = (int)((x_ratio * x) >> 16);
    yv = (int)((y_ratio * y) >> 16);

    x_diff = ((x_ratio * x) >> 16) - (xv);
    y_diff = ((y_ratio * y) >> 16) - (yv);

    index = yv * NES_FRAME_WIDTH + xv;

    a = bufs[index];
    b = bufs[index + 1];
    c = bufs[index + NES_FRAME_WIDTH];
    d = bufs[index + NES_FRAME_WIDTH + 1];

    red = (((a >> 11) & 0x1f) * (1 - x_diff) * (1 - y_diff) + ((b >> 11) & 0x1f) * (x_diff) * (1 - y_diff) +
           ((c >> 11) & 0x1f) * (y_diff) * (1 - x_diff) + ((d >> 11) & 0x1f) * (x_diff * y_diff));

    green = (((a >> 5) & 0x3f) * (1 - x_diff) * (1 - y_diff) + ((b >> 5) & 0x3f) * (x_diff) * (1 - y_diff) +
             ((c >> 5) & 0x3f) * (y_diff) * (1 - x_diff) + ((d >> 5) & 0x3f) * (x_diff * y_diff));

    blue = (((a) & 0x1f) * (1 - x_diff) * (1 - y_diff) + ((b) & 0x1f) * (x_diff) * (1 - y_diff) +
            ((c) & 0x1f) * (y_diff) * (1 - x_diff) + ((d) & 0x1f) * (x_diff * y_diff));

    col = ((int)red << 11) | ((int)green << 5) | ((int)blue);

    return col;
}