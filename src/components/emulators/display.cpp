#include "display.h"

/*********************
 *      DEFINES
 *********************/
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

uint16_t *line[LINE_BUFFERS];

uint16_t myPalette[256];

void display_HAL_clear()
{
    tft.fillScreen(TFT_BLACK);
}

uint16_t getPixelNESPixel(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t outputWidth, uint16_t outputHeight);
static uint16_t getPixelGBC(const uint16_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2);

void display_HAL_NES_frame(const uint8_t *data)
{
    if (data == NULL)
    {
        tft.fillScreen(TFT_BLACK);
        return;
    }

    const short outputHeight = 240;
    const short outputWidth = 240;
    const short xpos = (240 - outputWidth) / 2;
    const short ypos = 0;

    uint16_t lineBuffer[outputWidth * LINE_COUNT];

    for (int y = 0; y < outputHeight; y += LINE_COUNT)
    {
        for (int i = 0; i < LINE_COUNT; ++i)
        {
            if ((y + i) >= outputHeight)
                break;

            int index = i * outputWidth;

            for (int x = 0; x < outputWidth; x++)
            {
                lineBuffer[index++] = myPalette[getPixelNESPixel(data, x, (y + i), outputWidth, outputHeight)];
            }
        }

        tft.pushImage(xpos, y, outputWidth, LINE_COUNT, lineBuffer);
    }
}



void display_HAL_gb_frame(const uint16_t *data)
{
    if (data == NULL)
    {
        tft.fillScreen(TFT_BLACK);
        return;
    }

    const short outputHeight = 144;
    const short outputWidth = 160;
    const short xpos = (240 - outputWidth) / 2;
    const short ypos = 0;

    uint16_t lineBuffer[outputWidth * LINE_COUNT];

    for (int y = 0; y < outputHeight; y += LINE_COUNT)
    {
        for (int i = 0; i < LINE_COUNT; ++i)
        {
            if ((y + i) >= outputHeight)
                break;

            int index = i * outputWidth;

            for (int x = 0; x < outputWidth; x++)
            {
                lineBuffer[index++] = myPalette[getPixelGBC(data, x, (y + i), outputWidth, outputHeight)];
            }
        }

        tft.pushImage(xpos, y, outputWidth, LINE_COUNT, lineBuffer);
    }
}

uint16_t getPixelNESPixel(const uint8_t *bufs, uint16_t x, uint16_t y, uint16_t outputWidth, uint16_t outputHeight)
{
    if (!bufs)
        return 0; // Handle null pointer

    int xRatio = ((NES_FRAME_WIDTH - 1) << 16) / outputWidth;
    int yRatio = ((NES_FRAME_HEIGHT - 1) << 16) / outputHeight;

    int xv = (x * xRatio) >> 16;
    int yv = (y * yRatio) >> 16;

    // Clamp to valid NES frame coordinates
    xv = min(xv, NES_FRAME_WIDTH - 2);  // -2 to ensure index+1 is valid
    yv = min(yv, NES_FRAME_HEIGHT - 2); // -2 to ensure index+NES_FRAME_WIDTH is valid

    int index = yv * NES_FRAME_WIDTH + xv;

    uint8_t a = bufs[index];
    uint8_t b = bufs[index + 1];
    uint8_t c = bufs[index + NES_FRAME_WIDTH];
    uint8_t d = bufs[index + NES_FRAME_WIDTH + 1];

    int xDiff = ((x * xRatio) >> 16) - xv;
    int yDiff = ((y * yRatio) >> 16) - yv;

    // Bilinear interpolation
    int red = (((a >> 11) & 0x1F) * (1 - xDiff) * (1 - yDiff) +
               ((b >> 11) & 0x1F) * (xDiff) * (1 - yDiff) +
               ((c >> 11) & 0x1F) * (1 - xDiff) * (yDiff) +
               ((d >> 11) & 0x1F) * (xDiff) * (yDiff));

    int green = (((a >> 5) & 0x3F) * (1 - xDiff) * (1 - yDiff) +
                 ((b >> 5) & 0x3F) * (xDiff) * (1 - yDiff) +
                 ((c >> 5) & 0x3F) * (1 - xDiff) * (yDiff) +
                 ((d >> 5) & 0x3F) * (xDiff) * (yDiff));

    int blue = (((a) & 0x1F) * (1 - xDiff) * (1 - yDiff) +
                ((b) & 0x1F) * (xDiff) * (1 - yDiff) +
                ((c) & 0x1F) * (1 - xDiff) * (yDiff) +
                ((d) & 0x1F) * (xDiff) * (yDiff));

    return ((red & 0x1F) << 11) | ((green & 0x3F) << 5) | (blue & 0x1F);
}
static uint16_t getPixelGBC(const uint16_t *bufs, uint16_t x, uint16_t y, uint16_t w2, uint16_t h2)
{

    int x_diff, y_diff, xv, yv, red, green, blue, col, a, b, c, d, index;
    int x_ratio = (int)(((GBC_FRAME_WIDTH - 1) << 16) / w2) + 1;
    int y_ratio = (int)(((GBC_FRAME_HEIGHT - 1) << 16) / h2) + 1;

    xv = (int)((x_ratio * x) >> 16);
    yv = (int)((y_ratio * y) >> 16);

    x_diff = ((x_ratio * x) >> 16) - (xv);
    y_diff = ((y_ratio * y) >> 16) - (yv);

    index = yv * GBC_FRAME_WIDTH + xv;

    a = bufs[index];
    b = bufs[index + 1];
    c = bufs[index + GBC_FRAME_WIDTH];
    d = bufs[index + GBC_FRAME_WIDTH + 1];

    red = (((a >> 11) & 0x1f) * (1 - x_diff) * (1 - y_diff) + ((b >> 11) & 0x1f) * (x_diff) * (1 - y_diff) +
           ((c >> 11) & 0x1f) * (y_diff) * (1 - x_diff) + ((d >> 11) & 0x1f) * (x_diff * y_diff));

    green = (((a >> 5) & 0x3f) * (1 - x_diff) * (1 - y_diff) + ((b >> 5) & 0x3f) * (x_diff) * (1 - y_diff) +
             ((c >> 5) & 0x3f) * (y_diff) * (1 - x_diff) + ((d >> 5) & 0x3f) * (x_diff * y_diff));

    blue = (((a) & 0x1f) * (1 - x_diff) * (1 - y_diff) + ((b) & 0x1f) * (x_diff) * (1 - y_diff) +
            ((c) & 0x1f) * (y_diff) * (1 - x_diff) + ((d) & 0x1f) * (x_diff * y_diff));

    col = ((int)red << 11) | ((int)green << 5) | ((int)blue);

    return col;
}