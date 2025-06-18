//
// Created by dhima on 18-06-2025.
//

#ifndef USBCDC_H
#define USBCDC_H

#include <stdint.h>  // Add this line to define int32_t and other standard integer types
#include <esp_event_base.h>
#include "USB.h"

class usbCDC {

private:
    // USBCDC USBSerial;
    static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

public:
    void init();
    uint32_t baudRate();

    int usbSerialAvailable();
    int read(void);
    size_t read(uint8_t *buffer, size_t size);
    size_t write(const uint8_t *buffer, size_t size);
};

#endif //USBCDC_H