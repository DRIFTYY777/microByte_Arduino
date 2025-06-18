//
// Created by dhima on 17-06-2025.
//

#ifndef STK500_COMMANDS_H
#define STK500_COMMANDS_H

// STK500 Constants
#define STK_OK              0x10
#define STK_FAILED          0x11
#define STK_UNKNOWN         0x12
#define STK_INSYNC          0x14
#define STK_NOSYNC          0x15
#define CRC_EOP             0x20

// STK500 Commands
#define STK_GET_SYNC        0x30
#define STK_GET_SIGN_ON     0x31
#define STK_SET_PARAMETER   0x40
#define STK_GET_PARAMETER   0x41
#define STK_SET_DEVICE      0x42
#define STK_SET_DEVICE_EXT  0x45
#define STK_ENTER_PROGMODE  0x50
#define STK_LEAVE_PROGMODE  0x51
#define STK_CHIP_ERASE      0x52
#define STK_CHECK_AUTOINC   0x53
#define STK_LOAD_ADDRESS    0x55
#define STK_UNIVERSAL       0x56
#define STK_PROG_FLASH      0x60
#define STK_PROG_DATA       0x61
#define STK_PROG_FUSE       0x62
#define STK_PROG_LOCK       0x63
#define STK_PROG_PAGE       0x64
#define STK_PROG_FUSE_EXT   0x65
#define STK_READ_FLASH      0x70
#define STK_READ_DATA       0x71
#define STK_READ_FUSE       0x72
#define STK_READ_LOCK       0x73
#define STK_READ_PAGE       0x74
#define STK_READ_SIGN       0x75
#define STK_READ_OSCCAL     0x76
#define STK_READ_FUSE_EXT   0x77
#define STK_READ_OSCCAL_EXT 0x78

// Parameters
#define STK_HW_VER          0x80
#define STK_SW_MAJOR        0x81
#define STK_SW_MINOR        0x82
#define STK_LEDS            0x83
#define STK_VTARGET         0x84
#define STK_VADJUST         0x85
#define STK_OSC_PSCALE      0x86
#define STK_OSC_CMATCH      0x87
#define STK_RESET_DURATION  0x88
#define STK_SCK_DURATION    0x89


#endif //STK500_COMMANDS_H
