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

#pragma once

#include "config.h"

#define kMainScriptName         ("main.")

// NOTE: unlike newer games, Manhattan Dealers used different main script name for different platforms
#define kManAtariScriptName     ("man.sng")
#define kManAmigaScriptName     ("prog.oo")
#define kManMSDOSScriptName     ("prog.io")

// try to identify game using unique script name

typedef enum {
    EPlatformAtari = 0,
    EPlatformFalcon,
    EPlatformAmiga,
    EPlatformAmigaAGA,
    EPlatformMac,
    EPlatformPC,
    EPlatformAmstradCPC,
    EPlatform3DO,
    EPlatformJaguar,
    EPlatformPS1,
    EPlatformPS2,
    EPlatformOldAtari,
    EPlatformOldAmiga,
    EPlatformUnknown
} EPlatform;

// name         basemem     context     sprite      version
// dealers      0x?????     0x28        0x1c        1.0
// mad show     0x23e00     0x2c        0x22        1.1
// windsurf     0x25400     0x2e        0x24        1.2
// targhan      0x26000     0x30        0x24        1.2
// Maya         0x25e00     0x2e        0x24        1.3
// colorado     0x26000     0x2e        0x24        1.3

// starblade    0x21400     0x34        0x28        2.0
// storm        0x1ff00     0x34        0x28        2.0
// metal        0x20800     0x34        0x28        2.0
// boston       0x27d00     0x34        0x28        2.1
// trans        0x21d00     0x34        0x28        2.1
// arborea      0x1f300     0x34        0x28        2.0
// bunny        0x22200     0x34        0x28        2.1
// ishar 1      0x20000     0x34        0x28        2.0/2.1
// ishar 2      0x22400     0x34        0x28        2.1

// ishar 3      0x25a00     0x3e        0x30        3.0
// requiem      0x2b400     0x3e        0x30        3.0

// deus         0x?????     0x??        0x??        4.0

typedef enum {
    
    EGameManhattanDealers0       = 0x019bfcc0,
    EGameManhattanDealers1       = 0x02255100,
    EGameMadShow                 = 0x00244140,
    EGameWindsurfWilly           = 0x00325aa0,
    EGameTarghan0                = 0x0044aa20,
    EGameTarghan1                = 0x002dc6c0,
    EGameLeFeticheMaya           = 0x00180858,
    EGameColorado                = 0x003ece20,
    EGameStarblade               = 0x00594390,
    EGameStormMaster             = 0x00632ea0,
    EGameMetalMutant             = 0x001b7740,
    EGameCrystalsOfArborea0      = 0x006cb808,
    EGameCrystalsOfArborea1      = 0x006ddd00,
    EGameTransartica             = 0x01312d00,
    EGameBostonBombClub          = 0x00149970,
    EGameBunnyBricks             = 0x002625a0,
    EGameIshar_1                 = 0x00b71b00,
    EGameIshar_2                 = 0x00ec82e0,
    EGameIshar_3                 = 0x015b2330,
    EGameRobinsonsRequiem0       = 0x00f42400,
    EGameRobinsonsRequiem1       = 0x02160ec0,
    EGameDeus                    = -2,
    EGameTimeWarriors            = -3,
    EGameAsghan                  = -4,
    EGameTournamentOfWarriors    = -5,
    EGameArabianNights           = -6,
    EGameLesVisiteurs            = -7,
    EGameInspectorGadget         = -8,
    EGameXyphoesFantasy          = -9,
    EGameKillingFist             = -10,
    EGameUnknown                 = 0,

} EGame;

typedef enum {
    
    EPxFormatChunky             = 0,
    EPxFormatSTPlanar           = 1,
    EPxFormatAmPlanar           = 2,

} EPxFormat;

typedef struct {
    EPlatform   kind;
    char        name[32];
    u32         version;
    u32         uid;
    char        desc[kDescMaxLen];  // platform description
    char        ext[4];             // script file extension
    u32         ram_sz;             // size of ram, in bytes
    u32         video_ram_sz;       // size of video ram, in bytes
    u16         width;              // screen info
    u16         height;
    u8          bpp;
    EPxFormat   px_format;
    u8          dbl_buf;
    u8          is_little_endian;
    char        path[kPathMaxLen];  // path to scripts
    char        main[kPathMaxLen];  // path to main script
} sPlatform;

sPlatform*  pl_guess(const char* folder_path);
int         pl_supported(sPlatform* platform);
