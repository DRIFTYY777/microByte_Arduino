#ifndef __UPDATE_FIRMWARE_H__
#define __UPDATE_FIRMWARE_H__

class UPDATE_FIRMWARE
{
public:
    void update_check();
    int update_init(const char *fw_name);
};

extern UPDATE_FIRMWARE update_firmware;

#endif // __UPDATE_FIRMWARE_H__