//
//  platform.c
//  silm-depack
//

#include "platform.h"
#include "utils.h"

static sPlatform platforms[] = {
    { EPlatformAtari,       "Atari ST/STe", "ao", 320, 200, 5, "" },
    { EPlatformFalcon,      "Atari Falcon", "fo", 320, 200, 8, "" },
    { EPlatformAmiga,       "Amiga",        "co", 320, 200, 5, "" },
    { EPlatformAmigaAGA,    "Amiga AGA",    "do", 320, 200, 8, "" },
    { EPlatformMac,         "Macintosh",    "??", 320, 200, 5, "" },
    { EPlatformPC,          "MS/DOS",       "io", 320, 200, 8, "" },
    { EPlatformUnknown,     "Unknown",      "??",   0,   0, 0, "" },
};


static sPlatform get_platform(const char * file_path) {
    FILE * file = NULL;
    char * ext = NULL;
    sPlatform platform = platforms[EPlatformUnknown];

    // get file extension
    if((file = fopen(file_path, "r")) != NULL) {
        fclose(file);
        ext = strlower(strrchr(file_path, '.') + 1);
    }
    
    // check platform by script file extension
    for (int i = 0; i <= EPlatformUnknown; i++) {
        platform = platforms[i];
        if(!strcmp(ext, platform.ext)) {
            break;
        }
    }
    
    return platform;
}


int is_supported(sPlatform platform) {
    return platform.kind != EPlatformUnknown && platform.kind != EPlatformPC;
}


sPlatform guess_platform(const char * path) {
    sPlatform platform = platforms[EPlatformUnknown];
    struct dirent * ent;
    DIR * dir = opendir(path);
    if (dir != NULL) {
        // open path ok, loop thru files until we find the main script
        // that matches one of the declared platforms
        char main_path[kPathMaxLen];
        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_type == DT_REG) {
                // look for main script
                if(!strncmp(strlower(ent->d_name), kMainScriptName, strlen(kMainScriptName))) {
                    memset(main_path, 0, kPathMaxLen);
                    sprintf(main_path, "%s%c%s", path, kPathSeparator, ent->d_name);
                    platform = get_platform(main_path);
                    if(platform.kind != EPlatformUnknown) {
                        strcpy(platform.main, main_path);
                        strcpy(platform.path, path);
                        break;
                    }
                }
            }
        }
        closedir(dir);
    }
    else {
        // this might be a script file path
        FILE * fp = fopen(path, "r");
        if(fp != NULL) {
            fclose(fp);
            sPlatform pf = get_platform(path);
            if(pf.kind != EPlatformUnknown) {
                platform = pf;
                strcpy(platform.main, path);
                char * ptr = strrchr(path, kPathSeparator);
                *++ptr = '\0';
                strcpy(platform.path, path);
            }
        }
        else {
            printf("ERROR: %s is not a valid script path.", path);
        }
    }
    return platform;
}
