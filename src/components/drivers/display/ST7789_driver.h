#pragma once

#include "driver/spi_master.h"

/*******************************
 *      TYPEDEF
 * *****************************/
struct st7789_driver;

typedef struct st7789_transaction_data
{
	struct st7789_driver *driver;
	bool data;
} st7789_transaction_data_t;

typedef uint16_t st7789_color_t;

typedef struct st7789_driver
{
	int pin_reset;
	int pin_dc;
	int pin_cs;
	int pin_mosi;
	int pin_miso;
	int pin_sclk;
	int spi_host;
	int spi_speed;
	int dma_chan;
	uint8_t queue_fill;
	uint16_t display_width;
	uint16_t display_height;
	spi_device_handle_t spi;
	size_t buffer_size;
	st7789_transaction_data_t data;
	st7789_transaction_data_t command;
	st7789_color_t *buffer;
	st7789_color_t *buffer_primary;
	st7789_color_t *buffer_secondary;
	st7789_color_t *current_buffer;
	spi_transaction_t trans_a;
	spi_transaction_t trans_b;
} st7789_driver_t;

typedef struct st7789_command
{
	uint8_t command;
	uint8_t wait_ms;
	uint8_t data_size;
	const uint8_t *data;
} st7789_command_t;

/*********************
 *      FUNCTIONS
 *********************/

/*
 * Function:  ST7789_init
 * --------------------
 *
 * Initialize the SPI peripheral and send the initialization sequence.
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: True if the initialization suceed otherwise false.
 *
 */
bool ST7789_init(st7789_driver_t *driver);

/*
 * Function:  ST7789_reset
 * --------------------
 *
 * Reset the display
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: Nothing.
 *
 */
void ST7789_reset(st7789_driver_t *driver);

/*
 * Function:  ST7789_fill_area
 * --------------------
 *
 * Fill a area of the display with a selected color
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 * 	-color: 16 Bit hexadecimal color to fill the area.
 * 	-start_x: Start point on the X axis.
 * 	-start_y: Start point on the Y axis.
 * 	-width: Width of the area to be fill.
 * 	-height: Height of the area to be fill.
 *
 * Returns: Nothing.
 *
 */
void ST7789_fill_area(st7789_driver_t *driver, st7789_color_t color, uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height);

/*
 * Function:  ST7789_write_pixels
 * --------------------
 *
 * WIP
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: Nothing.
 *
 */
void ST7789_write_pixels(st7789_driver_t *driver, st7789_color_t *pixels, size_t length);

/*
 * Function:  ST7789_write_lines
 * --------------------
 *
 * WIP
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: Nothing.
 *
 */
void ST7789_write_lines(st7789_driver_t *driver, int ypos, int xpos, int width, uint16_t *linedata, int lineCount);

/*
 * Function:  ST7789_swap_buffers
 * --------------------
 *
 * The driver has two buffer, to allow send and render the image at the same type. This function
 * send the data of the actived buffer and change the pointer of current buffer to the next one.
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: Nothing.
 *
 */
void ST7789_swap_buffers(st7789_driver_t *driver);

/*
 * Function:  ST7789_set_window
 * --------------------
 *
 * This screen allows partial update of the screen, so we can specified which part of the windows is going to change.
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 * 	-start_x: X axis start point of the refresh zone.
 * 	-start_y: Y axis start point of the refresh zone.
 *	-end_x: X axis end point of the refresh zone.
 *	-end_y: Y axis end point of the refresh zone.

 * Returns: Nothing.
 *
 */
void ST7789_set_window(st7789_driver_t *driver, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);

/*
 * Function:  ST7789_set_endian
 * --------------------
 *
 * Depper explanation on the display_HAL.h file, but this function change the screen configuration from,
 * little endian message to big endian message.
 *
 * Arguments:
 * 	-driver: Screen driver structure.
 *
 * Returns: Nothing.
 *
 */

void ST7789_little_endian(st7789_driver_t *driver);
void ST7789_big_endian(st7789_driver_t *driver);

void ST7789_invert_display(st7789_driver_t *driver, bool invert);
void ST7789_rotate_display(st7789_driver_t *driver, uint8_t rotation);

void ST7789_drawPixel(st7789_driver_t *driver, int16_t x, int16_t y, uint16_t color);
void ST7789_drawCircle(st7789_driver_t *driver, int16_t x0, int16_t y0, int16_t r, uint16_t color);

