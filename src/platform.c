//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the “Software”), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

// TODO: remove platform/posix dependencies

#include "debug.h"
#include "platform.h"
#include "utils.h"

// TODO: Manhattan Dealers, the first game using ALIS engine is different. Its using 'OO' and 'SNG' file extensions and main script is in 'man.sng'

static sPlatform platforms[] = {
    { .kind = EPlatformAtari,       .uid = EGameUnknown, .version = 20, .desc = "Atari ST/STe",                 .ext = "AO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .is_little_endian = 0, .path = "" },
    { .kind = EPlatformFalcon,      .uid = EGameUnknown, .version = 20, .desc = "Atari Falcon",                 .ext = "FO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .is_little_endian = 0, .path = "" },
    { .kind = EPlatformAmiga,       .uid = EGameUnknown, .version = 20, .desc = "Amiga",                        .ext = "CO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .is_little_endian = 0, .path = "" },
    { .kind = EPlatformAmigaAGA,    .uid = EGameUnknown, .version = 20, .desc = "Amiga AGA",                    .ext = "DO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .is_little_endian = 0, .path = "" },
    { .kind = EPlatformMac,         .uid = EGameUnknown, .version = 20, .desc = "Macintosh",                    .ext = "MO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 4,  .is_little_endian = 0, .path = "" },
    { .kind = EPlatformPC,          .uid = EGameUnknown, .version = 20, .desc = "MS/DOS",                       .ext = "IO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .is_little_endian = 1, .path = "" },
    { .kind = EPlatformAmstradCPC,  .uid = EGameUnknown, .version = 20, .desc = "Amstrad CPC",                  .ext = "??", .ram_sz = 0x20000,  .video_ram_sz = 0x3e80,  .width = 320, .height = 200, .bpp = 2,  .is_little_endian = 1, .path = "" },
    { .kind = EPlatform3DO,         .uid = EGameUnknown, .version = 20, .desc = "3DO Interactive Multiplayer",  .ext = "3O", .ram_sz = 0x200000, .video_ram_sz = 0x6c000, .width = 384, .height = 288, .bpp = 24, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformJaguar,      .uid = EGameUnknown, .version = 20, .desc = "Atari Jaguar",                 .ext = "??", .ram_sz =        0, .video_ram_sz =      0,  .width = 320, .height = 200, .bpp = 24, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformUnknown,     .uid = EGameUnknown, .version = 20, .desc = "Unknown",                      .ext = "??", .ram_sz =        0, .video_ram_sz =      0,  .width =   0, .height =   0, .bpp = 0,  .is_little_endian = 0, .path = "" },
};

int pl_supported(sPlatform* platform) {
    return platform->kind != EPlatformUnknown;
}


static sPlatform* pl_get(const char * file_path) {
    FILE * file = NULL;
    char * ext = NULL;
    sPlatform* pl = &platforms[EPlatformUnknown];

    // get file extension
    if((file = fopen(file_path, "r")) != NULL) {
        fclose(file);
        ext = strrchr(file_path, '.') + 1;
    
        // check platform by script file extension
        for (int i = 0; i <= EPlatformUnknown; i++) {
            pl = &platforms[i];
            if(!strncasecmp(ext, pl->ext, strlen(ext))) {
                break;
            }
        }
    }
    return pl;
}


sPlatform* pl_guess(const char * path) {
    sPlatform* platform = &platforms[EPlatformUnknown];
    struct dirent * ent;
    DIR * dir = opendir(path);
    if (dir != NULL) {
        // open path ok, loop thru files until we find the main script
        // that matches one of the declared platforms
        char main_path[kPathMaxLen];
        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_type == DT_REG) {
                // look for main script
                if(!strncasecmp(ent->d_name, kMainScriptName, strlen(kMainScriptName))) {
                    memset(main_path, 0, kPathMaxLen);
                    sprintf(main_path, "%s%c%s", path, kPathSeparator, ent->d_name);
                    platform = pl_get(main_path);
                    if(platform->kind != EPlatformUnknown) {
                        strcpy(platform->main, main_path);
                        strcpy(platform->path, path);
                        break;
                    }
                }
            }
        }
        
        closedir(dir);
    }
    else {
        debug(EDebugError,"%s is not a valid data path.", path);
    }
    
    return platform;
}
