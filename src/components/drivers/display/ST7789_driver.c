/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "ST7789_driver.h"
#include "ST7789_regs.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include <esp32-hal-log.h>

/*********************
 *      DEFINES
 *********************/
#define ST7789_SPI_QUEUE_SIZE 7

/**********************
 *      VARIABLES
 **********************/
static const char *TAG = "ST7789_driver";

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ST7789_send_cmd(st7789_driver_t *driver, const st7789_command_t *command);
static void ST7789_config(st7789_driver_t *driver);
static void ST7789_pre_cb(spi_transaction_t *transaction);
static void ST7789_queue_empty(st7789_driver_t *driver);
static void ST7789_multi_cmd(st7789_driver_t *driver, const st7789_command_t *sequence);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
bool ST7789_init(st7789_driver_t *driver)
{
	// Allocate the buffer memory
	driver->buffer = (st7789_color_t *)heap_caps_malloc(driver->buffer_size * 2 * sizeof(st7789_color_t), MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
	if (driver->buffer == NULL)
	{
		ESP_LOGE(TAG, "Display buffer allocation fail");
		return false;
	}

	ESP_LOGI(TAG, "Display buffer allocated with a size of: %i", driver->buffer_size * 2 * sizeof(st7789_color_t));

	// Why set buffer, primary and secondary instead,just primary and secondary??
	// Set-up the display buffers
	driver->buffer_primary = driver->buffer;
	driver->buffer_secondary = driver->buffer + driver->buffer_size;
	driver->current_buffer = driver->buffer_primary;
	driver->queue_fill = 0;

	driver->data.driver = driver;
	driver->data.data = true;
	driver->command.driver = driver;
	driver->command.data = false;

	// Set the RESET and DC PIN
	gpio_pad_select_gpio(driver->pin_reset);
	gpio_pad_select_gpio(driver->pin_dc);
	gpio_set_direction(driver->pin_reset, GPIO_MODE_OUTPUT);
	gpio_set_direction(driver->pin_dc, GPIO_MODE_OUTPUT);

	ESP_LOGI(TAG, "Set RST pin: %i \n Set DC pin: %i", HSPI_RST, HSPI_DC);

	// Set-Up SPI BUS
	spi_bus_config_t buscfg = {
		.mosi_io_num = driver->pin_mosi,
		.miso_io_num = driver->pin_miso,
		.sclk_io_num = driver->pin_sclk,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = driver->buffer_size * 2 * sizeof(st7789_color_t),
		.flags = 0};

	// Configure SPI BUS
	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = driver->spi_speed,
		.mode = 3,
		.spics_io_num = driver->pin_cs,
		.queue_size = ST7789_SPI_QUEUE_SIZE,
		.pre_cb = ST7789_pre_cb,
		.flags = SPI_DEVICE_NO_DUMMY};

	if (spi_bus_initialize(driver->spi_host, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) // try without DMA first.
	{
		ESP_LOGE(TAG, "SPI Bus initialization failed.");
		return false;
	}

	if (spi_bus_add_device(driver->spi_host, &devcfg, &driver->spi) != ESP_OK)
	{
		ESP_LOGE(TAG, "SPI Bus add device failed.");
		return false;
	}

	ESP_LOGI(TAG, "SPI Bus configured correctly.");

	// Set the screen configuration
	ST7789_reset(driver);
	ST7789_config(driver);

	ESP_LOGI(TAG, "Display configured and ready to work.");

	return true;
}

void ST7789_reset(st7789_driver_t *driver)
{
	gpio_set_level(driver->pin_reset, 0);
	vTaskDelay(20 / portTICK_PERIOD_MS);
	gpio_set_level(driver->pin_reset, 1);
	vTaskDelay(130 / portTICK_PERIOD_MS);
}

void ST7789_fill_area(st7789_driver_t *driver, st7789_color_t color, uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height)
{
	// Fill the buffer with the selected color
	for (size_t i = 0; i < driver->buffer_size * 2; ++i)
	{
		driver->buffer[i] = color;
	}

	// Set the working area on the screen
	ST7789_set_window(driver, start_x, start_y, start_x + width - 1, start_y + height - 1);

	size_t bytes_to_write = width * height * 2;
	size_t transfer_size = driver->buffer_size * 2 * sizeof(st7789_color_t);

	spi_transaction_t trans;
	spi_transaction_t *rtrans;

	memset(&trans, 0, sizeof(trans));
	trans.tx_buffer = driver->buffer;
	trans.user = &driver->data;
	trans.length = transfer_size * 8;
	trans.rxlength = 0;

	while (bytes_to_write > 0)
	{
		if (driver->queue_fill >= ST7789_SPI_QUEUE_SIZE)
		{
			spi_device_get_trans_result(driver->spi, &rtrans, portMAX_DELAY);
			driver->queue_fill--;
		}
		if (bytes_to_write < transfer_size)
		{
			transfer_size = bytes_to_write;
		}
		spi_device_queue_trans(driver->spi, &trans, portMAX_DELAY);
		driver->queue_fill++;
		bytes_to_write -= transfer_size;
	}

	ST7789_queue_empty(driver);
}

void ST7789_write_pixels(st7789_driver_t *driver, st7789_color_t *pixels, size_t length)
{
	ST7789_queue_empty(driver);

	spi_transaction_t *trans = driver->current_buffer == driver->buffer_primary ? &driver->trans_a : &driver->trans_b;
	memset(trans, 0, sizeof(&trans));
	trans->tx_buffer = driver->current_buffer;
	trans->user = &driver->data;
	trans->length = length * sizeof(st7789_color_t) * 8;
	trans->rxlength = 0;

	spi_device_queue_trans(driver->spi, trans, portMAX_DELAY);
	driver->queue_fill++;
}

// void ST7789_write_lines(st7789_driver_t *driver, int ypos, int xpos, int width, uint16_t *linedata, int lineCount)
// {
// 	int size = width * 2 * 8 * lineCount;
// 	driver->buffer_size = 240 * 40;
// 	ST7789_set_window(driver, 0, ypos, 240, ypos + 20);
// 	ST7789_swap_buffers(driver);
// }

void ST7789_write_lines(st7789_driver_t *driver, int ypos, int xpos, int width, uint16_t *linedata, int lineCount)
{
	int size = width * lineCount * sizeof(uint16_t);
	int max_size = 4096;
	ST7789_set_window(driver, xpos, ypos, xpos + width - 1, ypos + lineCount - 1);
	for (int i = 0; i < size; i += max_size)
	{
		int chunk_size = (size - i > max_size) ? max_size : (size - i);
		ST7789_write_pixels(driver, &linedata[i / 2], chunk_size);
	}
	ST7789_swap_buffers(driver); // Swap only after writing pixels
}

void ST7789_swap_buffers(st7789_driver_t *driver)
{
	ST7789_write_pixels(driver, driver->current_buffer, driver->buffer_size);
	driver->current_buffer = driver->current_buffer == driver->buffer_primary ? driver->buffer_secondary : driver->buffer_primary;
}

void ST7789_set_window(st7789_driver_t *driver, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
	uint8_t caset[4];
	uint8_t raset[4];

	caset[0] = (uint8_t)(start_x >> 8) & 0xFF;
	caset[1] = (uint8_t)(start_x & 0xff);
	caset[2] = (uint8_t)(end_x >> 8) & 0xFF;
	caset[3] = (uint8_t)(end_x & 0xff);
	raset[0] = (uint8_t)(start_y >> 8) & 0xFF;
	raset[1] = (uint8_t)(start_y & 0xff);
	raset[2] = (uint8_t)(end_y >> 8) & 0xFF;
	raset[3] = (uint8_t)(end_y & 0xff);

	st7789_command_t sequence[] = {
		{ST7789_CMD_CASET, 0, 4, caset},
		{ST7789_CMD_RASET, 0, 4, raset},
		{ST7789_CMD_RAMWR, 0, 0, NULL},
		{ST7789_CMDLIST_END, 0, 0, NULL},
	};

	ST7789_multi_cmd(driver, sequence);
}

void ST7789_little_endian(st7789_driver_t *driver)
{
	const st7789_command_t init_sequence[] = {
		{ST7789_CMD_RAMCTRL, 0, 2, (const uint8_t *)"\x00\xc8"},
		{ST7789_CMDLIST_END, 0, 0, NULL},
	};
	ST7789_multi_cmd(driver, init_sequence);
}

void ST7789_big_endian(st7789_driver_t *driver)
{
	const st7789_command_t init_sequence[] = {
		{ST7789_CMD_RAMCTRL, 0, 2, (const uint8_t *)"\x00\x00"},
		{ST7789_CMDLIST_END, 0, 0, NULL},
	};
	ST7789_multi_cmd(driver, init_sequence);
}

void ST7789_invert_display(st7789_driver_t *driver, bool invert)
{
	const st7789_command_t init_sequence2[] = {
		{invert ? ST7789_CMD_INVON : ST7789_CMD_INVOFF, 0, 0, NULL},
		{ST7789_CMDLIST_END, 0, 0, NULL},
	};
	ST7789_multi_cmd(driver, init_sequence2);
}

void ST7789_rotate_display(st7789_driver_t *driver, uint8_t rotation)
{
	uint8_t madctl = 0;

	switch (rotation)
	{
	case 0:
		madctl = 0x00; // Normal
		break;
	case 1:
		madctl = 0x60; // Rotate 90 degrees
		break;
	case 2:
		madctl = 0xC0; // Rotate 180 degrees
		break;
	case 3:
		madctl = 0xA0; // Rotate 270 degrees
		break;
	default:
		ESP_LOGE(TAG, "Invalid rotation value: %d", rotation);
		return;
	}

	const st7789_command_t init_sequence[] = {
		{ST7789_CMD_MADCTL, 0, 1, &madctl},
		{ST7789_CMDLIST_END, 0, 0, NULL},
	};

	ST7789_multi_cmd(driver, init_sequence);

	// Update display dimensions
	if (rotation == 1 || rotation == 3)
	{
		uint16_t temp = driver->display_width;
		driver->display_width = driver->display_height;
		driver->display_height = temp;
	}
	else
	{
		// Restore original dimensions
		if (driver->display_width != 320 || driver->display_height != 240)
		{
			driver->display_width = 320;
			driver->display_height = 240;
		}
	}

	// Refresh display
	ST7789_set_window(driver, 0, 0, driver->display_width - 1, driver->display_height - 1);
	ST7789_fill_area(driver, 0x0000, 0, 0, driver->display_width, driver->display_height);
}

void ST7789_drawPixel(st7789_driver_t *driver, int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= driver->display_width) || (y < 0) || (y >= driver->display_height))
	{
		return;
	}
	ST7789_set_window(driver, x, y, x, y);
	ST7789_fill_area(driver, color, x, y, 1, 1);
}

void ST7789_drawCircle(st7789_driver_t *driver, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ST7789_drawPixel(driver, x0, y0 + r, color);
	ST7789_drawPixel(driver, x0, y0 - r, color);
	ST7789_drawPixel(driver, x0 + r, y0, color);
	ST7789_drawPixel(driver, x0 - r, y0, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		ST7789_drawPixel(driver, x0 + x, y0 + y, color);
		ST7789_drawPixel(driver, x0 - x, y0 + y, color);
		ST7789_drawPixel(driver, x0 + x, y0 - y, color);
		ST7789_drawPixel(driver, x0 - x, y0 - y, color);
		ST7789_drawPixel(driver, x0 + y, y0 + x, color);
		ST7789_drawPixel(driver, x0 - y, y0 + x, color);
		ST7789_drawPixel(driver, x0 + y, y0 - x, color);
		ST7789_drawPixel(driver, x0 - y, y0 - x, color);
	}
}
/**********************
 *   STATIC FUNCTIONS
 **********************/

static void ST7789_pre_cb(spi_transaction_t *transaction)
{
	const st7789_transaction_data_t *data = (st7789_transaction_data_t *)transaction->user;
	st7789_driver_t *driver = data->driver; // Retrieve the driver from the user field
	gpio_set_level(driver->pin_dc, data->data);
}

static void ST7789_config(st7789_driver_t *driver)
{

	const uint8_t caset[4] = {
		0x00,
		0x00,
		(driver->display_width - 1) >> 8,
		(driver->display_width - 1) & 0xff};
	const uint8_t raset[4] = {
		0x00,
		0x00,
		(driver->display_height - 1) >> 8,
		(driver->display_height - 1) & 0xff};
	const st7789_command_t init_sequence[] = {
		// Sleep
		{ST7789_CMD_SLPIN, 10, 0, NULL},	// Sleep
		{ST7789_CMD_SWRESET, 200, 0, NULL}, // Reset
		{ST7789_CMD_SLPOUT, 120, 0, NULL},	// Sleep out

		{ST7789_CMD_MADCTL, 0, 1, (const uint8_t *)"\x00"}, // Page / column address order
		{ST7789_CMD_COLMOD, 0, 1, (const uint8_t *)"\x55"}, // 16 bit RGB
		{ST7789_CMD_INVON, 0, 0, NULL},						// Inversion on
		{ST7789_CMD_CASET, 0, 4, (const uint8_t *)&caset},	// Set width
		{ST7789_CMD_RASET, 0, 4, (const uint8_t *)&raset},	// Set height

		// Porch setting
		{ST7789_CMD_PORCTRL, 0, 5, (const uint8_t *)"\x0c\x0c\x00\x33\x33"},
		// Set VGH to 12.54V and VGL to -9.6V
		{ST7789_CMD_GCTRL, 0, 1, (const uint8_t *)"\x14"},
		// Set VCOM to 1.475V
		{ST7789_CMD_VCOMS, 0, 1, (const uint8_t *)"\x37"},
		// Enable VDV/VRH control
		{ST7789_CMD_VDVVRHEN, 0, 2, (const uint8_t *)"\x01\xff"},
		// VAP(GVDD) = 4.45+(vcom+vcom offset+vdv)
		{ST7789_CMD_VRHSET, 0, 1, (const uint8_t *)"\x12"},
		// VDV = 0V
		{ST7789_CMD_VDVSET, 0, 1, (const uint8_t *)"\x20"},
		// AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V
		{ST7789_CMD_PWCTRL1, 0, 2, (const uint8_t *)"\xa4\xa1"},
		// 60 fps
		{ST7789_CMD_FRCTR2, 0, 1, (const uint8_t *)"\x0f"},
		// Gama 2.2
		{ST7789_CMD_GAMSET, 0, 1, (const uint8_t *)"\x01"},
		// Gama curve
		{ST7789_CMD_PVGAMCTRL, 0, 14, (const uint8_t *)"\xd0\x08\x11\x08\x0c\x15\x39\x33\x50\x36\x13\x14\x29\x2d"},
		{ST7789_CMD_NVGAMCTRL, 0, 14, (const uint8_t *)"\xd0\x08\x10\x08\x06\x06\x39\x44\x51\x0b\x16\x14\x2f\x31"},

		// Little endian
		{ST7789_CMD_RAMCTRL, 0, 2, (const uint8_t *)"\x00\xc8"},
		{ST7789_CMDLIST_END, 0, 0, NULL}, // End of commands
	};
	ST7789_multi_cmd(driver, init_sequence);
	ST7789_fill_area(driver, 0x0000, 0, 0, driver->display_width, driver->display_height);
	const st7789_command_t init_sequence2[] = {
		{ST7789_CMD_DISPON, 100, 0, NULL}, // Display on
		{ST7789_CMD_SLPOUT, 100, 0, NULL}, // Sleep out
		{ST7789_CMD_CASET, 0, 4, caset},
		{ST7789_CMD_RASET, 0, 4, raset},
		{ST7789_CMD_RAMWR, 0, 0, NULL},
		{ST7789_CMDLIST_END, 0, 0, NULL}, // End of commands
	};
	ST7789_multi_cmd(driver, init_sequence2);
}

static void ST7789_send_cmd(st7789_driver_t *driver, const st7789_command_t *command)
{
	spi_transaction_t *return_trans;
	spi_transaction_t data_trans;

	// Check if the SPI queue is empty
	ST7789_queue_empty(driver);

	// Send the command
	memset(&data_trans, 0, sizeof(data_trans));
	data_trans.length = 8; // 8 bits
	data_trans.tx_buffer = &command->command;
	data_trans.user = &driver->command;

	spi_device_queue_trans(driver->spi, &data_trans, portMAX_DELAY);
	spi_device_get_trans_result(driver->spi, &return_trans, portMAX_DELAY);

	// Send the data if the command has.
	if (command->data_size > 0)
	{
		memset(&data_trans, 0, sizeof(data_trans));
		data_trans.length = command->data_size * 8;
		data_trans.tx_buffer = command->data;
		data_trans.user = &driver->data;

		spi_device_queue_trans(driver->spi, &data_trans, portMAX_DELAY);
		spi_device_get_trans_result(driver->spi, &return_trans, portMAX_DELAY);
	}

	// Wait the required time
	if (command->wait_ms > 0)
	{
		vTaskDelay(command->wait_ms / portTICK_PERIOD_MS);
	}
}

static void ST7789_multi_cmd(st7789_driver_t *driver, const st7789_command_t *sequence)
{
	while (sequence->command != ST7789_CMDLIST_END)
	{
		ST7789_send_cmd(driver, sequence);
		sequence++;
	}
}

static void ST7789_queue_empty(st7789_driver_t *driver)
{
	spi_transaction_t *return_trans;

	while (driver->queue_fill > 0)
	{
		spi_device_get_trans_result(driver->spi, &return_trans, portMAX_DELAY);
		driver->queue_fill--;
	}
}
