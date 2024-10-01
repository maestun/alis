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

// TODO: Manhattan Dealers (Operation: Cleanstreets in the US), the first game on the ALIS engine, is different.
// Atari ST: it uses 'OO' file extension and the main script is in 'man.sng' (not 'OO'!),
// Amiga:    it uses 'OO' file extension and the main script is in 'prog.oo',
// PC:       it uses 'IO' file extension and the main script is in 'prog.io'.

static sPlatform platforms[] = {
    { .kind = EPlatformAtari,       .uid = EGameUnknown, .version = 20, .desc = "Atari ST/STe",                 .ext = "AO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformFalcon,      .uid = EGameUnknown, .version = 20, .desc = "Atari Falcon/Macintosh",       .ext = "FO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformAmiga,       .uid = EGameUnknown, .version = 20, .desc = "Amiga",                        .ext = "CO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformAmigaAGA,    .uid = EGameUnknown, .version = 20, .desc = "Amiga AGA",                    .ext = "DO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformMac,         .uid = EGameUnknown, .version = 20, .desc = "Macintosh",                    .ext = "MO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 480, .height = 300, .bpp = 1,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformPC,          .uid = EGameUnknown, .version = 20, .desc = "IBM PC",                       .ext = "IO", .ram_sz = 0x400000, .video_ram_sz = 0xfa00,  .width = 320, .height = 200, .bpp = 8,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformAmstradCPC,  .uid = EGameUnknown, .version = 20, .desc = "Amstrad CPC",                  .ext = "??", .ram_sz = 0x20000,  .video_ram_sz = 0x3e80,  .width = 320, .height = 200, .bpp = 2,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatform3DO,         .uid = EGameUnknown, .version = 20, .desc = "3DO Interactive Multiplayer",  .ext = "3O", .ram_sz = 0x200000, .video_ram_sz = 0x6c000, .width = 384, .height = 288, .bpp = 24, .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformJaguar,      .uid = EGameUnknown, .version = 20, .desc = "Atari Jaguar",                 .ext = "??", .ram_sz =        0, .video_ram_sz =      0,  .width = 320, .height = 200, .bpp = 24, .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformPS1,         .uid = EGameUnknown, .version = 20, .desc = "Sony PlayStation",             .ext = "PO", .ram_sz =        0, .video_ram_sz =      0,  .width = 512, .height = 240, .bpp = 24, .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformPS2,         .uid = EGameUnknown, .version = 20, .desc = "Sony PlayStation 2",           .ext = "2O", .ram_sz =        0, .video_ram_sz =      0,  .width = 640, .height = 512, .bpp = 24, .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 1, .path = "" },
    { .kind = EPlatformOldAtari,    .uid = EGameUnknown, .version = 20, .desc = "Atari ST/STe",                 .ext = "OO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformOldAmiga,    .uid = EGameUnknown, .version = 20, .desc = "Amiga",                        .ext = "OO", .ram_sz = 0x100000, .video_ram_sz = 0x8000,  .width = 320, .height = 200, .bpp = 4,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
    { .kind = EPlatformUnknown,     .uid = EGameUnknown, .version = 20, .desc = "Unknown",                      .ext = "??", .ram_sz =        0, .video_ram_sz =      0,  .width =   0, .height =   0, .bpp = 0,  .px_format = EPxFormatChunky, .dbl_buf = 1, .is_little_endian = 0, .path = "" },
};

int pl_supported(sPlatform* platform) {
    return platform->kind != EPlatformUnknown;
}

static sPlatform* pl_get(const char * main_script) {
    char * ext = NULL;
    sPlatform* pl = &platforms[EPlatformUnknown];

     // Manhattan Dealers on Atari and Amiga use files with the same 'OO' extension,
     // but the main script is in the 'main.sng' and 'main.oo' files respectively.
     // We identify them by the main script name, not by used extension
     if (!strncasecmp(main_script, kManAtariScriptName, strlen(kManAtariScriptName))) {
         pl = &platforms[EPlatformOldAtari];
         return pl;
         }
     if (!strncasecmp(main_script, kManAmigaScriptName, strlen(kManAmigaScriptName))) {
         pl = &platforms[EPlatformOldAmiga];
         return pl;
         } 

    // get file extension
    ext = strrchr(main_script, '.') + 1;

    // check platform by script file extension
    for (int i = 0; i <= EPlatformUnknown; i++) {
        pl = &platforms[i];
        if(!strncasecmp(ext, pl->ext, strlen(ext))) {
            break;
        }
     }
 return pl;
}

sPlatform* pl_guess(const char * path) {
    sPlatform* platform = &platforms[EPlatformUnknown];
    FILE * file = NULL;
    if (path == NULL)
    {
        debug(EDebugError,"Enter valid data path.\n");
        return platform;
    }
    
    size_t pathlen = strlen(path);
    if (pathlen <= 0)
    {
        debug(EDebugError,"Enter valid data path.\n");
        return platform;
    }
    
    char data_path[kPathMaxLen];
    strcpy(data_path, path);

    if (path[pathlen - 1] != kPathSeparator)
    {
        data_path[pathlen] = kPathSeparator;
        data_path[pathlen + 1] = 0;
    }
    
    struct dirent * ent;
    DIR * dir = opendir(data_path);
    if (dir == NULL)
    {
        debug(EDebugError,"%s is not a valid data path.\n", data_path);
        return platform;
    }

    // open path ok, loop thru files until we find the main script
    // that matches one of the declared platforms
    char main_path[kPathMaxLen];
    while ((ent = readdir(dir)) != NULL)
    {
        if(ent->d_type == DT_REG)
        {
            // look for main script
            if(!strncasecmp(ent->d_name, kMainScriptName, strlen(kMainScriptName)) ||
               !strncasecmp(ent->d_name, kManAtariScriptName, strlen(kManAtariScriptName)) ||
               !strncasecmp(ent->d_name, kManAmigaScriptName, strlen(kManAmigaScriptName)) ||
               !strncasecmp(ent->d_name, kManMSDOSScriptName, strlen(kManMSDOSScriptName)))
            {
                strcpy(main_path, data_path);
                strcat(main_path, ent->d_name);

                if ((file = fopen(main_path, "r")) != NULL) {
                   fclose(file);
                }
                else {
                   debug(EDebugError,"Cannot open %s for reading: No such file or directory.\n", main_path);
                   closedir(dir);
                   return platform;
                }

                platform = pl_get(ent->d_name);
                if(platform->kind != EPlatformUnknown)
                {
                    strcpy(platform->main, main_path);
                    strcpy(platform->path, data_path);
                    break;
                }
            }
        }
    }
    
    closedir(dir);
    
    return platform;
}
