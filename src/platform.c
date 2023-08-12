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

static sPlatform platforms[] = {
    { EPlatformAtari,       EGameUnknown, "Atari ST/STe", "AO", 0x100000, 0x8000, 320, 200, 5, 0, "" },
    { EPlatformFalcon,      EGameUnknown, "Atari Falcon", "FO", 0x400000, 0xfa00, 320, 200, 8, 0, "" },
    { EPlatformAmiga,       EGameUnknown, "Amiga",        "CO", 0x100000, 0x8000, 320, 200, 5, 0, "" },
    { EPlatformAmigaAGA,    EGameUnknown, "Amiga AGA",    "DO", 0x400000, 0xfa00, 320, 200, 8, 0, "" },
    { EPlatformMac,         EGameUnknown, "Macintosh",    "MO",        0,      0, 320, 200, 5, 0, "" },
    { EPlatformPC,          EGameUnknown, "MS/DOS",       "IO",        0,      0, 320, 200, 8, 1, "" },
    { EPlatformAmstradCPC,  EGameUnknown, "Amstrad CPC",  "??",        0,      0, 320, 200, 2, 1, "" },
    { EPlatformUnknown,     EGameUnknown, "Unknown",      "??",        0,      0,   0,   0, 0, 0, "" },
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
        dir = opendir(path);

        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_type == DT_REG) {
                // try to identify game
                if(!strncasecmp(ent->d_name, kMadShowScriptName, strlen(kMadShowScriptName))) {
                    
                    platform->game = EGameMadShow;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kTarghanScriptName, strlen(kTarghanScriptName))) {
                    
                    platform->game = EGameTarghan;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kWindsurfScriptName, strlen(kWindsurfScriptName))) {
                    
                    platform->game = EGameWindsurfWilly;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kMayaScriptName, strlen(kMayaScriptName))) {
                    
                    platform->game = EGameLeFéticheMaya;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kColoradoScriptName, strlen(kColoradoScriptName))) {
                    
                    platform->game = EGameColorado;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kStarbladeScriptName, strlen(kStarbladeScriptName))) {
                    
                    platform->game = EGameStarblade;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kBostonScriptName, strlen(kBostonScriptName))) {
                    
                    platform->game = EGameBostonBombClub;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kArboreaScriptName, strlen(kArboreaScriptName))) {
                    
                    platform->game = EGameCrystalsOfArborea;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kMutantScriptName, strlen(kMutantScriptName))) {
                    
                    platform->game = EGameMetalMutant;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kTransarticaScriptName, strlen(kTransarticaScriptName))) {
                    
                    platform->game = EGameTransartica;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kStormMasterScriptName, strlen(kStormMasterScriptName))) {
                    
                    platform->game = EGameStormMaster;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kBunnyBricksScriptName, strlen(kBunnyBricksScriptName))) {
                    
                    platform->game = EGameBunnyBricks;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kIshar_IScriptName, strlen(kIshar_IScriptName))) {
                    
                    platform->game = EGameIshar_I;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kIshar_IIScriptName, strlen(kIshar_IIScriptName))) {
                    
                    platform->game = EGameIshar_II;
                    break;
                }
                else if(!strncasecmp(ent->d_name, kIshar_IIIScriptName, strlen(kIshar_IIIScriptName))) {
                    
                    platform->game = EGameIshar_III;
                    break;
                }
            }
        }

        closedir(dir);
    }
    else {
        debug(EDebugError,"%s is not a valid data path.", path);
    }
//    else {
//        // this might be a script file path
//        FILE* fp = fopen(path, "r");
//        if(fp != NULL) {
//            fclose(fp);
//            sPlatform* pf = pl_get(path);
//            if(pf->kind != EPlatformUnknown) {
//                platform = pf;
//                // strcpy(platform->main, path);
//                // char * ptr = strrchr(path, kPathSeparator[0]);
//                // *++ptr = 0;
//                // strcpy(platform->path, path);
//            }
//        }
//        else {
//            debug(EDebugError,"%s is not a valid data path.", path);
//        }
//    }
    
    switch (platform->game) {
            
        case EGameMadShow:
        case EGameTarghan:
        case EGameLeFéticheMaya:
        case EGameColorado:
        case EGameStarblade:
        case EGameCrystalsOfArborea:
        case EGameMetalMutant:
        case EGameBostonBombClub:
        case EGameStormMaster:
        case EGameWindsurfWilly:
            platform->bpp = 4;
        case EGameTransartica:
        case EGameBunnyBricks:
        case EGameIshar_I:
        case EGameIshar_II:
        case EGameIshar_III:
            // NOP, depending on platform
            break;
            
        default:
            break;
    };
    
    return platform;
}
