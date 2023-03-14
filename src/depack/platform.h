//
//  platform.h
//  silmarils-unpacker
//

#pragma once

#include "config.h"

#define kMainScriptName ("main")

typedef enum {
    EPlatformAtari = 0,
    EPlatformFalcon,
    EPlatformAmiga,
    EPlatformAmigaAGA,
    EPlatformMac,
    EPlatformPC,
    EPlatformUnknown
} EPlatform;


typedef struct {
    EPlatform   kind;
    char        desc[kDescMaxLen];  // platform description
    char        ext[4];             // script file extension
    u32         ram_sz;             // size of ram, in bytes
    u32         video_ram_sz;       // size of video ram, in bytes
    u16         width;              // screen info
    u16         height;
    u8          bpp;
    u8          is_little_endian;
    char        path[kPathMaxLen];  // path to scripts
    char        main[kPathMaxLen];  // path to main script
} sPlatform;

sPlatform*  pl_guess(const char* folder_path);
int         pl_supported(sPlatform* platform);
