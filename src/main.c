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

#include "alis.h"
#include "config.h"
#include "image.h"
#include "unpack.h"
#include "sys/sys.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(__TOS__) || defined(__atarist__)
#include <mint/osbind.h>
#endif

// Wait for a keypress so a startup error stays readable (GEM repaints the desktop on exit).
// Only while GEMDOS owns the keyboard: before sys_init or after sys_deinit. No-op off Atari.
static void hold_screen(void)
{
#if defined(__TOS__) || defined(__atarist__)
    printf("\nPress any key to quit.\n");
    fflush(stdout);
    Cconin();
#endif
}

// Desktop dbglog (native build: sys_atari.c): stderr, so the planar/conversion diagnostics link.
#if !defined(ALIS_USE_NATIVE_ATARI)
void __attribute__((format(printf,1,2))) dbglog(const char *fmt, ...)
{
    // TEMP diagnostic: also mirror to macdbg.log, diff-able against the native alis_dbg.log.
    static FILE *lf = NULL; static int tried = 0;
    if (!tried) { tried = 1; lf = fopen("/Users/gildor/work/macdbg.log", "w"); }
    va_list ap;
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    if (lf) { va_start(ap, fmt); vfprintf(lf, fmt, ap); va_end(ap); fflush(lf); }
}
#endif

#if ALIS_SDL_VER >= 2
# if defined(_MSC_VER)
#  include "SDL.h"
# elif __has_include(<SDL.h>)
#  include <SDL.h>
# else
#  include <SDL2/SDL.h>
# endif
#endif

// To avoid the issue of undefined reference to 'SDL_main' in some compilers,
// we need two lines in main.c:
// #include "SDL2/SDL.h"
// int main(int argc, char *argv[]) (exactly like this)

int disalis = DEBUG_SCRIPT;
int authormode = 0;

void usage(void) {

    printf("%s ver. %s\n\n"
            "Usage:\n"
            "\tGame mode (Windowed):     %s <data_path>\n"
            "\tGame mode (Fullscreen):   %s -f <data_path>\n"
            "\tGame mode (No Sound):     %s -m <data_path>\n"
#ifndef NDEBUG
            "\tEnable runtime disalis:   %s -d <data_path>\n"
#endif
            "\tUnpack mode:              %s -u <data_path>\n"
            "\tDeveloper (author) mode:  %s -a <data_path>\n"
            "\n"
            "Timing:\n"
            "\t--native-timing           Run DOS/Mac data at their port's rates (DOS 70 Hz frames,\n"
            "\t                          60 Hz music; Mac 60 Hz) instead of the ST's 50 Hz\n"
            "\n"
            "Display scaling (1.2 vertical aspect, like CRT):\n"
#if ALIS_SDL_VER > 1
            "\t--1x                      Native 1x window, no aspect correction\n"
            "\t--2x                      2x window (default)\n"
            "\t--2x / --3x / --4x        Scale display 2x/3x/4x\n"
#else
            "\t--1x                      Native 1x window (default)\n"
            "\t--2x / --3x / --4x        Scale display 2x/3x/4x\n"
#endif
            "\n"
#if ALIS_SDL_VER == 1
            "Atari audio backend (Falcon):\n"
            "\t--audio=dma               CPU-mixed DMA sound (default, proven)\n"
            "\t--audio=dsp               DSP mixer (opt-in, not yet HW-verified)\n"
            "\t--audio-rate=N            Override DMA output rate (Falcon: 8195/9834/12292/\n"
            "\t                          16390/24585/49170; STE/TT: 6258/12517/25033/50066)\n"
            "\t--audio-stereo            Falcon: 8-bit stereo DMA instead of mono\n"
            "\n"
#endif
#if defined(ALIS_USE_NATIVE_ATARI)
            "Memory:\n"
            "\t--no-mem-check            Skip the free-memory check and run anyway\n"
            "\n"
#endif
            "Enhanced 3D (currently Robinson's Requiem only):\n"
            "\t--far                     Double terrain view distance\n"
#if ALIS_SDL_VER > 1
            "\t--fog                     Blend distant terrain toward sky horizon color\n"
            "\t--fog-start=N             Depth where fog begins (0-255, default 80)\n"
            "\t--fog-end=N               Depth where fog is fully opaque (0-255, default 255)\n"
#endif
           ,
            kProgName, kProgVersion,
#ifndef NDEBUG
            kProgName,
#endif
            kProgName, kProgName, kProgName, kProgName, kProgName);
    printf( "\nHotkeys:\n"\
            "\tPause                     Quit\n"\
            "\tShift+F10                 Quit (Atari: no Pause key on ST/Falcon keyboards)\n"\
            "\tUndo                      Quit (Atari native build)\n"\
            "\tPrScr                     Capture screenshot (not implemented yet)\n"
            "\tF11 / Shift+F1            Save state (experimental; Shift+F1 for Atari keyboards)\n"
            "\tF12 / Shift+F2            Load state (experimental; Shift+F2 for Atari keyboards)\n"
#ifndef NDEBUG
            "\tLeft Alt                  Set user debug label (in disalis listing)\n"
#endif
           );
    fflush(stdout);
}

int main(int argc, char *argv[]) {

    u8 result = 0;

    {
        printf("%s ver. %s\n", kProgName, kProgVersion);

        int fullscreen = 0;
        int unpackmode = 0;
        int mutesound = 0;
        int show_usage = 0;

        int opt_far = 0;
#if defined(ALIS_DEBUG_NO_MEM_CHECK)
        int opt_no_mem_check = 1;   // debug builds launched by hrdb can't pass --no-mem-check
#else
        int opt_no_mem_check = 0;
#endif
#if ALIS_SDL_VER > 1
        int opt_fog = 0;
        int opt_fog_start = -1;
        int opt_fog_end = -1;
#endif

        const char *path = NULL;

        for (int c = 1; c < argc; c++)
        {
            const char * cmd = argv[c];
            if (strcmp(cmd, "-f") == 0)
            {
                fullscreen = 1;
            }
            else if (strcmp(cmd, "-m") == 0)
            {
                mutesound = 1;
            }
#ifndef NDEBUG
            else if (strcmp(cmd, "-d") == 0)
            {
                disalis ^= 1;
            }
#endif
            else if (strcmp(cmd, "-u") == 0)
            {
                unpackmode = 1;
            }
            else if (strcmp(cmd, "-a") == 0)
            {
                authormode = 1;
            }
            else if (strncmp(cmd, "--audio-rate=", 13) == 0)
            {
                // Override the Atari DMA output rate. The auto pick keys off the _CPU cookie only
                // (family, not clock), so a fast 030 or a CT60 is judged as a stock one.
                extern int atari_audio_rate;
                atari_audio_rate = atoi(cmd + 13);
            }
            else if (strcmp(cmd, "--audio-stereo") == 0)
            {
                extern int atari_audio_stereo;
                atari_audio_stereo = 1;
            }
            else if (strcmp(cmd, "--native-timing") == 0)
            {
                opt_native_timing = 1;
            }
            else if (strcmp(cmd, "--far") == 0)
            {
                opt_far = 1;
            }
            else if (strcmp(cmd, "--no-mem-check") == 0)
            {
                opt_no_mem_check = 1;
            }
            else if (strcmp(cmd, "--opl-capture") == 0) {
                extern int opl_capture_flag;
                opl_capture_flag = 1;   // dump OPL register stream to opl_capture.bin
            }
            else if (strcmp(cmd, "--1x") == 0) { extern int opt_scale; opt_scale = 1; }
            else if (strcmp(cmd, "--2x") == 0) { extern int opt_scale; opt_scale = 2; }
            else if (strcmp(cmd, "--3x") == 0) { extern int opt_scale; opt_scale = 3; }
            else if (strcmp(cmd, "--4x") == 0) { extern int opt_scale; opt_scale = 4; }
#if ALIS_SDL_VER == 1
            else if (strcmp(cmd, "--no-timerc") == 0) {
                extern int atari_no_timerc;
                atari_no_timerc = 1;
            }
            // --audio=dsp|dma (Falcon): DSP mixer or CPU-mixed DMA (default). STE/TT: DMA only.
            // 0=auto(→DMA), 1=force DSP, 2=force DMA.
            else if (strcmp(cmd, "--audio=dsp") == 0) {
                extern int atari_audio_backend;
                atari_audio_backend = 1;
            }
            else if (strcmp(cmd, "--audio=dma") == 0) {
                extern int atari_audio_backend;
                atari_audio_backend = 2;
            }
#endif
#if ALIS_SDL_VER > 1
            else if (strcmp(cmd, "--fog") == 0)
            {
                opt_fog = 1;
            }
            else if (strncmp(cmd, "--fog-start=", 12) == 0)
            {
                int v = atoi(cmd + 12);
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                opt_fog_start = v;
            }
            else if (strncmp(cmd, "--fog-end=", 10) == 0)
            {
                int v = atoi(cmd + 10);
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                opt_fog_end = v;
            }
#endif
            else if (strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0)
            {
                show_usage = 1;
            }
            else
            {
                path = argv[c];
            }
        }

        if (show_usage)
        {
            usage();
            return 0;
        }

        // No path on the command line — fall back to the directory the
        // binary was launched from. Lets the user drop alis into a game
        // folder and double-click to play.
        static char base_path[1024];
        if (path == NULL)
        {
#if defined(ALIS_DEBUG_DATA_PATH) && (defined(__atarist__) || defined(__TOS__))
            // Debug builds launched by hrdb (no TTP args): 1 = Colorado, 2 = RRQ Falcon, 3 = RRQ DOS CD,
            // 4 = Ishar 1 ST, 5 = Ishar 2 ST, 6 = Arborea, 7 = Boston Bomb Club, 8 = Bunny Bricks,
            // 9 = Targhan, 10 = Metal Mutant (ST data).
            path = ALIS_DEBUG_DATA_PATH == 10 ? "C:\\ALIS\\DATA\\METALMUTANT\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 9 ? "C:\\ALIS\\DATA\\TARGHAN\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 8 ? "C:\\ALIS\\DATA\\BUNNYBRICKS\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 7 ? "C:\\ALIS\\DATA\\BOSTON\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 6 ? "C:\\ALIS\\DATA\\ARBOREA\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 5 ? "C:\\ALIS\\DATA\\ISHAR2\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 4 ? "C:\\ALIS\\DATA\\ISHAR\\ATARI\\"
                 : ALIS_DEBUG_DATA_PATH == 3 ? "C:\\ALIS\\DATA\\REQUIEM\\DOS_CD\\"
                 : ALIS_DEBUG_DATA_PATH == 2 ? "C:\\ALIS\\DATA\\REQUIEM\\FALCON\\"
                                             : "C:\\ALIS\\DATA\\COLORADO\\ATARI\\";
#elif ALIS_SDL_VER >= 2
            char *bp = SDL_GetBasePath();
            if (bp)
            {
                strncpy(base_path, bp, sizeof(base_path) - 1);
                base_path[sizeof(base_path) - 1] = 0;
                SDL_free(bp);
                path = base_path;
            }
#else
            const char *exe = argc > 0 ? argv[0] : "";
            const char *sep = strrchr(exe, '/');
#if defined(_WIN32) || defined(__atarist__) || defined(__TOS__)
            const char *bs = strrchr(exe, '\\');
            if (bs && (!sep || bs > sep)) sep = bs;
#endif
            if (sep)
            {
                size_t len = (size_t)(sep - exe) + 1;
                if (len >= sizeof(base_path)) len = sizeof(base_path) - 1;
                memcpy(base_path, exe, len);
                base_path[len] = 0;
                path = base_path;
            }
            else
            {
                path = "./";
            }
#endif
            printf("No data path given - using '%s'.\n", path);
        }

        sys_errors_init();

        sPlatform *pl = pl_guess(path, unpackmode);

#if defined(ALIS_MEM_SWAP_ALWAYS) && ALIS_MEM_SWAP_ALWAYS
        // Mirror image of the guard below: this build hardcodes "game and host endianness differ"
        // to delete the per-access test (mem.h _XSWAP_NEEDED). Matching-endian data would be
        // byte-swapped into garbage, so refuse it.
        if (pl_supported(pl) && pl->is_little_endian == is_host_le())
        {
            printf("This build is compiled for %s-endian games only (ALIS_MEM_SWAP_ALWAYS).\nUse the universal build to play this game.\n", is_host_le() ? "big" : "little");
            hold_screen();
            return 1;
        }
#endif

#if defined(ALIS_MEM_NATIVE_ENDIAN)
        // Native-byte-order memory access: refuse games whose endianness differs from the host.
        if (pl_supported(pl) && pl->is_little_endian != is_host_le())
        {
            printf("You are running %s-endian game on %s-endian machine.\nThis build support only games that match endians of host machine.\n\nRebuild without ALIS_MEM_NATIVE_ENDIAN to play it.\n", pl->is_little_endian ? "little" : "big", is_host_le() ? "little" : "big");
            hold_screen();
            return 1;
        }
#endif

#if defined(ALIS_USE_NATIVE_ATARI)
        // Refuse machines that can't fit the game BEFORE sys_init takes over video/GEM/vectors,
        // so a refusal needs no restore.
        if (pl_supported(pl) && !unpackmode && !opt_no_mem_check)
        {
            // The tier needs script_guess_game (header read only; alis_init redoes it). vram_init
            // must run first: it installs the _convert16/_convert32 pointers fread16/32 go through.
            alis.platform = *pl;                  // vram_init/script_guess_game read it
            vram_init();
            script_guess_game(pl->main);
            if (alis.platform.uid > 0)
            {
                u32 floor, pref;
                pl_arena_range(&alis.platform, &floor, &pref);
                if (!(alis_arena_size = sys_size_arena(&alis.platform, floor, pref)))
                {
                    hold_screen();
                    return 1;
                }
            }
        }
#endif

        // Unpack mode
        if (pl_supported(pl) && unpackmode)
        {
            alis.platform = *pl;
            vram_init();
            return 0;
        }

        // Game mode
        if (pl_supported(pl))
        {
            printf("#############################\n");
            printf("# System initialization...\n");
            printf("#############################\n");
            sys_init(pl, fullscreen, mutesound);

            printf("#############################\n");
            printf("# ALIS VM initialization...\n");
            printf("#############################\n");
            result = alis_init(*pl);
            if (result == 1)
            {
                // sys_init already took over video/GEM/vectors (incl. the Timer A ISR): unwind first.
                sys_deinit();
                hold_screen();
                sys_restore_desktop();   // last: repaints the desktop over our messages
                return result;
            }

            if (opt_far)          image.ddrawdist = 1;
#if ALIS_SDL_VER > 1
            if (opt_fog)          image.emode = 1;
            if (opt_fog_start >= 0) image.fogbeg = (u8)opt_fog_start;
            if (opt_fog_end   >= 0) image.fogend   = (u8)opt_fog_end;
#endif
            
            printf("#############################\n");
            printf("# Starting ALIS VM...\n");
            printf("#############################\n");
            result = sys_start();

            printf("\n");
            ALIS_DEBUG(EDebugSystem, "The ALIS VM has been stopped.\n");

            // Quit
            printf("\n");
            printf("#############################\n");
            printf("# System deinitialization...\n");
            printf("#############################\n");
            sys_deinit();

            if (alis_fatal) {
                printf("\n%s\n", alis_fatal);
                hold_screen();
            }

            printf("\n");
            printf("#############################\n");
            printf("# Releasing ALIS VM memory...\n");
            printf("#############################\n");
            alis_deinit();
            sys_errors_deinit();

            // LAST action: hands the screen back to GEM; anything printed after would paint over it.
            sys_restore_desktop();

        }
        else {
            ALIS_DEBUG(EDebugFatal, "Platform '%s' is not supported.\n", pl->desc);
        }
    }
    return result;
}
