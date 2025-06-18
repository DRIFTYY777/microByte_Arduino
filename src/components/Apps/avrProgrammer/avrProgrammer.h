//
// Created by dhima on 17-06-2025.
//

#ifndef AVRPROGRAMMER_H
#define AVRPROGRAMMER_H
#include <cstdint>

/*
 * MPSSE mode uses serial
 * Bit 1 -  SCK
 * Bit 2 -  MOSI
 * Bit 3 -  MISO
 * Bit 4 -  RESET
*/
class avrProgrammer {
private:
    // Pin definitions
    uint8_t sck_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t reset_pin;

    // State variables
    bool prog_mode;
    uint16_t current_address;
    uint8_t sck_duration;

    // Parameters
    uint8_t hw_version;
    uint8_t sw_major;
    uint8_t sw_minor;

    // Buffer for page programming
    uint8_t page_buffer[256];
    uint16_t page_size;

    // Internal methods
    void pulse_reset();
    void start_programming();
    void end_programming();
    uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    void universal_command();
    void get_version(uint8_t c);
    void set_parameters();
    void enter_program_mode();
    void leave_program_mode();
    void chip_erase();
    void read_signature();
    void read_fuse_lock(uint8_t cmd, uint8_t addr);
    void prog_fuse_lock(uint8_t cmd, uint8_t addr, uint8_t data);
    void read_page();
    void prog_page();
    void load_address();
    void empty_reply();
    void breply(uint8_t b);
    void get_parameter(uint8_t c);
    void set_parameter(uint8_t c, uint8_t value);
    bool getch(uint8_t* data, unsigned long timeout = 1000);
    void handle_command();



public:
    // Constructor with default pins
    avrProgrammer(uint8_t sck = 18, uint8_t mosi = 23, uint8_t miso = 19, uint8_t reset = 5);

    // Initialize the programmer
    bool init();

    // Deinitialize the programmer
    void deinit();

    // Main loop - call this regularly to handle STK500 commands
    void loop();

    // Check if programmer is active
    bool isActive() { return prog_mode; }

    // Set custom pins (call before init())
    void setPins(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t reset);


};


extern avrProgrammer avrProg;


#endif //AVRPROGRAMMER_H
