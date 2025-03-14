#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <components/drivers/display/display_hal.h>

extern uint16_t myPalette[256];
void display_HAL_clear();
void display_HAL_NES_frame(const uint8_t *data);
static uint8_t getPixelNES(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);
void display_HAL_gb_frame(const uint16_t *data);

#endif
