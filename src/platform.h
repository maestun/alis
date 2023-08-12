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

#define kMainScriptName         ("main")

// try to identify game using unique script name

#define kMadShowScriptName      ("roulette")
#define kTarghanScriptName      ("targhan")
#define kWindsurfScriptName     ("wwilly")
#define kMayaScriptName         ("itikal")
#define kColoradoScriptName     ("apaches")
#define kStarbladeScriptName    ("animcity")
#define kBostonScriptName       ("bobop")
#define kArboreaScriptName      ("shamnir")
#define kMutantScriptName       ("mutant")
#define kTransarticaScriptName  ("mamesc")
#define kStormMasterScriptName  ("sharkaan")
#define kBunnyBricksScriptName  ("balles")
#define kIshar_IScriptName      ("rampart")
#define kIshar_IIScriptName     ("elmtair5")
#define kIshar_IIIScriptName    ("moltus")

typedef enum {
    EPlatformAtari = 0,
    EPlatformFalcon,
    EPlatformAmiga,
    EPlatformAmigaAGA,
    EPlatformMac,
    EPlatformPC,
    EPlatformAmstradCPC,
    EPlatformUnknown
} EPlatform;

typedef enum {
    EGameManhattanDealers,      // Manhattan Dealers (most likely not gonna work)
    EGameMadShow,               // Mad Show
    EGameTarghan,               // Targhan
    EGameWindsurfWilly,         // Windsurf Willy
    EGameLeFéticheMaya,         // Le Fétiche Maya
    EGameColorado,              // Colorado
    EGameStarblade,             // Starblade
    EGameCrystalsOfArborea,     // Crystals of Arborea
    EGameMetalMutant,           // Metal Mutant
    EGameBostonBombClub,        // Boston Bomb Club
    EGameXyphoesFantasy,        // Xyphoes Fantasy
    EGameStormMaster,           // Storm Master
    EGameIshar_I,               // Ishar I
    EGameIshar_II,              // Ishar II
    EGameIshar_III,             // Ishar III
    EGameBunnyBricks,           // Bunny Bricks
    EGameTransartica,           // Transartica
    EGameRobinsonsRequiem,      // Robinson’s Requiem
    EGameAsghan,                // Asghan (anything past this likely use new engine)
    EGameDeus,                  // Deus
    EGameTournamentOfWarriors,  // Tournament of Warriors
    EGameTimeWarriors,          // Time Warriors
    EGameUnknown

} EGame;

typedef struct {
    EPlatform   kind;
    EGame       game;
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
