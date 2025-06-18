//
// Created by dhima on 17-06-2025.
//

#include "avrProgrammer.h"
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>

#include "STK500_commands.h"

avrProgrammer::avrProgrammer(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t reset) {
    sck_pin = sck;
    mosi_pin = mosi;
    miso_pin = miso;
    reset_pin = reset;
    prog_mode = false;
    current_address = 0;
    sck_duration = 1; // Default SCK duration
    hw_version = 2;
    sw_major = 1;
    sw_minor = 18;
    page_size = 128; // Default page size
}

bool avrProgrammer::init() {
    // Initialize pins
    pinMode(sck_pin, OUTPUT);
    pinMode(mosi_pin, OUTPUT);
    pinMode(miso_pin, INPUT);
    pinMode(reset_pin, OUTPUT);

    // Set initial states
    digitalWrite(sck_pin, LOW);
    digitalWrite(mosi_pin, LOW);
    digitalWrite(reset_pin, HIGH); // AVR not in reset

    // Initialize Serial for STK500 communication
    Serial.begin(19200); // Standard Arduino ISP baud rate
    Serial.setTimeout(1000);

    prog_mode = false;
    current_address = 0;

    return true;
}

void avrProgrammer::deinit() {
    if (prog_mode) {
        end_programming();
    }
    prog_mode = false;
}

void avrProgrammer::setPins(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t reset) {
    sck_pin = sck;
    mosi_pin = mosi;
    miso_pin = miso;
    reset_pin = reset;
}

void avrProgrammer::pulse_reset() {
    digitalWrite(reset_pin, HIGH);
    delayMicroseconds(100);
    digitalWrite(reset_pin, LOW);
    delayMicroseconds(100);
    digitalWrite(reset_pin, HIGH);
    delayMicroseconds(100);
}

void avrProgrammer::start_programming() {
    digitalWrite(reset_pin, LOW);
    delayMicroseconds(50);

    // Send programming enable command
    spi_transaction(0xAC, 0x53, 0x00, 0x00);
}

void avrProgrammer::end_programming() {
    digitalWrite(reset_pin, HIGH);
    digitalWrite(sck_pin, LOW);
    digitalWrite(mosi_pin, LOW);
}

uint8_t avrProgrammer::spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    uint8_t result = 0;

    for (int i = 0; i < 4; i++) {
        uint8_t data;
        switch(i) {
            case 0: data = a; break;
            case 1: data = b; break;
            case 2: data = c; break;
            case 3: data = d; break;
        }

        for (int bit = 7; bit >= 0; bit--) {
            digitalWrite(mosi_pin, (data >> bit) & 0x01);
            digitalWrite(sck_pin, HIGH);
            delayMicroseconds(sck_duration);

            if (i == 3) { // Only read result from last byte
                result = (result << 1) | digitalRead(miso_pin);
            }

            digitalWrite(sck_pin, LOW);
            delayMicroseconds(sck_duration);
        }
    }

    return result;
}

bool avrProgrammer::getch(uint8_t* data, unsigned long timeout) {
    unsigned long start = millis();
    while (!Serial.available()) {
        if (millis() - start > timeout) {
            return false;
        }
    }
    *data = Serial.read();
    return true;
}

void avrProgrammer::empty_reply() {
    if (Serial.read() == CRC_EOP) {
        Serial.write(STK_INSYNC);
        Serial.write(STK_OK);
    } else {
        Serial.write(STK_NOSYNC);
    }
}

void avrProgrammer::breply(uint8_t b) {
    if (Serial.read() == CRC_EOP) {
        Serial.write(STK_INSYNC);
        Serial.write(b);
        Serial.write(STK_OK);
    } else {
        Serial.write(STK_NOSYNC);
    }
}

void avrProgrammer::get_version(uint8_t c) {
    switch (c) {
        case 0x80:
            breply(hw_version);
            break;
        case 0x81:
            breply(sw_major);
            break;
        case 0x82:
            breply(sw_minor);
            break;
        default:
            breply(0);
    }
}

void avrProgrammer::set_parameters() {
    // Read 20 device parameters
    for (int i = 0; i < 20; i++) {
        Serial.read();
    }
    empty_reply();
}

void avrProgrammer::enter_program_mode() {
    start_programming();
    prog_mode = true;
    empty_reply();
}

void avrProgrammer::leave_program_mode() {
    end_programming();
    prog_mode = false;
    empty_reply();
}

void avrProgrammer::chip_erase() {
    spi_transaction(0xAC, 0x80, 0x00, 0x00);
    delay(30); // Wait for erase to complete
    empty_reply();
}

void avrProgrammer::load_address() {
    uint16_t addr = Serial.read() | (Serial.read() << 8);
    current_address = addr;
    empty_reply();
}

void avrProgrammer::universal_command() {
    uint8_t a = Serial.read();
    uint8_t b = Serial.read();
    uint8_t c = Serial.read();
    uint8_t d = Serial.read();

    uint8_t result = spi_transaction(a, b, c, d);
    breply(result);
}

void avrProgrammer::read_signature() {
    uint8_t sig1 = spi_transaction(0x30, 0x00, 0x00, 0x00);
    uint8_t sig2 = spi_transaction(0x30, 0x00, 0x01, 0x00);
    uint8_t sig3 = spi_transaction(0x30, 0x00, 0x02, 0x00);

    if (Serial.read() == CRC_EOP) {
        Serial.write(STK_INSYNC);
        Serial.write(sig1);
        Serial.write(sig2);
        Serial.write(sig3);
        Serial.write(STK_OK);
    } else {
        Serial.write(STK_NOSYNC);
    }
}

void avrProgrammer::read_fuse_lock(uint8_t cmd, uint8_t addr) {
    uint8_t result = spi_transaction(cmd, addr, 0x00, 0x00);
    breply(result);
}

void avrProgrammer::prog_fuse_lock(uint8_t cmd, uint8_t addr, uint8_t data) {
    spi_transaction(cmd, addr, 0x00, data);
    delay(5); // Wait for programming
    empty_reply();
}

void avrProgrammer::read_page() {
    uint16_t length = Serial.read() | (Serial.read() << 8);
    char memtype = Serial.read();

    if (Serial.read() == CRC_EOP) {
        Serial.write(STK_INSYNC);

        for (uint16_t i = 0; i < length; i++) {
            uint8_t data;
            if (memtype == 'F') { // Flash memory
                data = spi_transaction(0x20 + ((current_address + i) & 1),
                                     (current_address + i) >> 9,
                                     (current_address + i) >> 1, 0);
            } else { // EEPROM
                data = spi_transaction(0xA0, (current_address + i) >> 8,
                                     current_address + i, 0);
            }
            Serial.write(data);
        }
        Serial.write(STK_OK);
    } else {
        Serial.write(STK_NOSYNC);
    }
}

void avrProgrammer::prog_page() {
    uint16_t length = Serial.read() | (Serial.read() << 8);
    char memtype = Serial.read();

    // Read page data
    for (uint16_t i = 0; i < length; i++) {
        page_buffer[i] = Serial.read();
    }

    if (Serial.read() == CRC_EOP) {
        Serial.write(STK_INSYNC);

        if (memtype == 'F') { // Flash memory
            // Load page buffer
            for (uint16_t i = 0; i < length; i += 2) {
                spi_transaction(0x40, current_address >> 8,
                              (current_address + i) >> 1, page_buffer[i]);
                if (i + 1 < length) {
                    spi_transaction(0x48, current_address >> 8,
                                  (current_address + i) >> 1, page_buffer[i + 1]);
                }
            }

            // Write page
            spi_transaction(0x4C, current_address >> 8,
                          current_address >> 1, 0);
            delay(5); // Wait for page write

        } else { // EEPROM
            for (uint16_t i = 0; i < length; i++) {
                spi_transaction(0xC0, (current_address + i) >> 8,
                              current_address + i, page_buffer[i]);
                delay(5); // Wait for EEPROM write
            }
        }

        Serial.write(STK_OK);
    } else {
        Serial.write(STK_NOSYNC);
    }
}

void avrProgrammer::get_parameter(uint8_t c) {
    switch (c) {
        case STK_HW_VER:
            breply(hw_version);
            break;
        case STK_SW_MAJOR:
            breply(sw_major);
            break;
        case STK_SW_MINOR:
            breply(sw_minor);
            break;
        default:
            breply(0);
    }
}

void avrProgrammer::set_parameter(uint8_t c, uint8_t value) {
    switch (c) {
        case STK_SCK_DURATION:
            sck_duration = value;
            break;
    }
    empty_reply();
}

void avrProgrammer::handle_command() {
    uint8_t cmd;
    if (!getch(&cmd)) return;

    switch (cmd) {
        case STK_GET_SYNC:
            empty_reply();
            break;

        case STK_GET_SIGN_ON:
            if (Serial.read() == CRC_EOP) {
                Serial.write(STK_INSYNC);
                Serial.print("AVR ISP");
                Serial.write(STK_OK);
            }
            break;

        case STK_GET_PARAMETER:
            get_parameter(Serial.read());
            break;

        case STK_SET_PARAMETER:
            {
                uint8_t param = Serial.read();
                uint8_t value = Serial.read();
                set_parameter(param, value);
            }
            break;

        case STK_SET_DEVICE:
            set_parameters();
            break;

        case STK_SET_DEVICE_EXT:
            for (int i = 0; i < 5; i++) Serial.read();
            empty_reply();
            break;

        case STK_ENTER_PROGMODE:
            enter_program_mode();
            break;

        case STK_LEAVE_PROGMODE:
            leave_program_mode();
            break;

        case STK_CHIP_ERASE:
            chip_erase();
            break;

        case STK_LOAD_ADDRESS:
            load_address();
            break;

        case STK_UNIVERSAL:
            universal_command();
            break;

        case STK_READ_PAGE:
            read_page();
            break;

        case STK_PROG_PAGE:
            prog_page();
            break;

        case STK_READ_SIGN:
            read_signature();
            break;

        case STK_READ_FUSE:
            read_fuse_lock(0x50, 0x00);
            break;

        case STK_READ_FUSE_EXT:
            read_fuse_lock(0x50, 0x08);
            break;

        case STK_READ_LOCK:
            read_fuse_lock(0x58, 0x00);
            break;

        case STK_PROG_FUSE:
            {
                uint8_t data = Serial.read();
                prog_fuse_lock(0xAC, 0xA0, data);
            }
            break;

        case STK_PROG_FUSE_EXT:
            {
                uint8_t data = Serial.read();
                prog_fuse_lock(0xAC, 0xA8, data);
            }
            break;

        case STK_PROG_LOCK:
            {
                uint8_t data = Serial.read();
                prog_fuse_lock(0xAC, 0xE0, data);
            }
            break;

        default:
            if (Serial.read() == CRC_EOP) {
                Serial.write(STK_NOSYNC);
            }
            break;
    }
}

void avrProgrammer::loop() {
    if (Serial.available()) {
        handle_command();
    }
}
