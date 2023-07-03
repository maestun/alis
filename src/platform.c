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

#include "platform.h"
#include "utils.h"

static sPlatform platforms[] = {
    { EPlatformAtari,       "Atari ST/STe", "AO", 0x100000, 0x8000, 320, 200, 5, 0, "" },
    { EPlatformFalcon,      "Atari Falcon", "FO", 0x400000, 0xfa00, 320, 200, 8, 0, "" },
    { EPlatformAmiga,       "Amiga",        "CO", 0x100000, 0x8000, 320, 200, 5, 0, "" },
    { EPlatformAmigaAGA,    "Amiga AGA",    "DO", 0x400000, 0xfa00, 320, 200, 8, 0, "" },
    { EPlatformMac,         "Macintosh",    "MO",        0,      0, 320, 200, 5, 0, "" },
    { EPlatformPC,          "MS/DOS",       "IO",        0,      0, 320, 200, 8, 1, "" },
    { EPlatformUnknown,     "Unknown",      "??",        0,      0,   0,   0, 0, 0, "" },
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
        // this might be a script file path
        FILE* fp = fopen(path, "r");
        if(fp != NULL) {
            fclose(fp);
            sPlatform* pf = pl_get(path);
            if(pf->kind != EPlatformUnknown) {
                platform = pf;
                // strcpy(platform->main, path);
                // char * ptr = strrchr(path, kPathSeparator[0]);
                // *++ptr = 0;
                // strcpy(platform->path, path);
            }
        }
        else {
            printf("ERROR: %s is not a valid script path.", path);
        }
    }
    return platform;
}
