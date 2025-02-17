

// HSPI or VSPI configuration
#define HSPI_HOST SPI2_HOST // Use SPI2_HOST for ESP32-S3
#define VSPI_HOST SPI3_HOST // Use SPI3_HOST if needed

// HSPI pin configuration
#define HSPI_MISO 13 // SCL or MISO
#define HSPI_MOSI 11 // SDA or MOSI
#define HSPI_CLK 12  // CLK

// VPSI pin configuration
#define VSPI_MOSI 35 // SDA or MOSI
#define VSPI_MISO 37 // SCL or MISO
#define VSPI_CLK 36  // CLK

// I2C pin configuration
#define I2C_SDA 8 // SDA
#define I2C_SCL 9 // SCL

// SD Card pin configuration
#define SD_CS0 39
#define SD_SPEED 4000000 // 4Mhz

// Sound pin configuration (MAX98357AETE)
#define I2S_BCK 27    // I2C SCL
#define I2S_WS 26     // I2C SDA
#define I2S_DATA_O 25 // I2C SDA
#define I2S_NUM I2S_NUM_0

// Input pin configuration (TCA9555)
#define TCA_CLK_SPEED (400 * 1000) // 400Khz
#define TCA_dev_address 0x20

// Fuel Gauge pin configuration (MAX17048)
#define MAX17048_CLK_SPEED (400 * 1000) // 400Khz

// Display Resolution: 240*RGB (H) *320(V) (ST7789)
#define SCR_MODEL ST7789
#define SCR_WIDTH 240
#define SCR_HEIGHT 320
#define SCR_BUFFER_SIZE SCR_WIDTH *SCR_HEIGHT * 2

// Display pin configuration
#define DSP_RST 46                       // RESET
#define DSP_DC 14                        // DC
#define DSP_CS 10                        // CS
#define DSP_BACKLIGTH 3                  // Backlight
#define DSP_CLK_SPEED (60 * 1000 * 1000) // 60Mhz

// LED pin configuration
#define LED_PIN 2
