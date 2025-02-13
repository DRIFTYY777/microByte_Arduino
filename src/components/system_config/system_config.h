/********************************
 *    Display pin configuration
 * ******************************/

// Display Resolution: 240*RGB (H) *320(V)
// Frame Memory Size: 240 x 320 x 18-bit = 1,382,400 bits
#define SCR_MODEL ST7789
#define SCR_WIDTH 240
#define SCR_HEIGHT 320
#define SCR_BUFFER_SIZE SCR_WIDTH *SCR_HEIGHT * 2

#define HSPI_MISO 12      // SCL or MISO
#define HSPI_MOSI 13      // SDA or MOSI
#define HSPI_CLK 14       // CLK
#define HSPI_RST 46       // RESET
#define HSPI_DC 21        // DC
#define HSPI_CS 11        // CS
#define HSPI_BACKLIGTH 3 // Backlight

#define HSPI_CLK_SPEED 60 * 1000 * 1000 // 60Mhz

// HSPI_HOST
// #ifdef CONFIG_IDF_TARGET_ESP32
// #define HSPI_HOST SPI2
// #elif defined CONFIG_IDF_TARGET_ESP32S3
// #define HSPI_HOST SPI2  
// #endif

// #define HSPI_HOST GPSPI2  // Use GPSPI2 instead of SPI2
// #define VSPI_HOST GPSPI3  // If needed, use GPSPI3 instead of SPI3





#define HSPI_HOST SPI2_HOST  // ✅ Use SPI2_HOST for ESP32-S3
#define VSPI_HOST SPI3_HOST  // ✅ Use SPI3_HOST if needed



/********************************
 *   SD CARD pin configuration
 * ******************************/
#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_CLK 18
#define VSPI_CS0 5
#define SPI_SPEED 4000000 // 4Mhz

/********************************
 *   Pin Mux pin configuration
 * ******************************/
// #define MUX_SDA 21  // I2C SDA
#define MUX_SDA 32 // I2C SDA
#define MUX_SCL 22
#define MUX_INT 34
#define I2C_CLK 400 * 1000 // 400Khz
#define I2C_dev_address 0x20

/********************************
 *          I2S Pin
 * ******************************/
#define I2S_BCK 27    // I2C SCL
#define I2S_WS 26     // I2C SDA
#define I2S_DATA_O 25 // I2C SDA
#define I2S_NUM I2S_NUM_0

/********************************
 *          Notification LED
 * ******************************/
#define LED_PIN 2

/*
 10K Potentiometer for volume control
*/

// #define POTENTIOMETER_PIN 34

/*
    Vibration motor
*/

// #define VIBRATION_MOTOR 4

/*
    battery voltage divider
*/

// #define BATTERY_PIN 36