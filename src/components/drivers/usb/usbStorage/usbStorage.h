//
// Created by dhima on 18-06-2025.
//

#ifndef USBMSC_H
#define USBMSC_H

#include "USBMSC.h"


class USBSTORAGE
{
private:
    bool usbModeActive = false;
    // USB Mass Storage
    USBMSC MSC;

    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
    // USB MSC Callbacks
    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);

public:
    bool init();
    bool deinit();
};

//extern USBSTORAGE usbStorage;


#endif //USBMSC_H
