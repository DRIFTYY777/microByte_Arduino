#ifndef __EXTERNAL_APP_H__
#define __EXTERNAL_APP_H__

/*
    Basic we re pushing firmware in the
    flash via OTA and make that partation
    bootable soo we can run the
    external app from the flash.

    Another mether is using ELF file loader in PSRAM/DRAM
    and run the external app from there.
*/

class EXTERNAL_APP
{
public:
    void external_app_init(const char *app_name);
};

extern EXTERNAL_APP external_app;
#endif
