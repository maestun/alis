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
#include "unpack.h"
#include "sys/sys.h"
#include "SDL2/SDL.h"

void usage(void) {

    printf("%s ver. %s\n\n"\
            "Usage:\n"\
            "\tGame mode (Windowed):     %s <data_path>\n"\
            "\tGame mode (Fullscreen):   %s -f <data_path>\n"\
            "\tUnpack mode:              %s -u <data_path>\n",
            kProgName, kProgVersion, kProgName, kProgName, kProgName);
    printf( "\nHotkeys:\n"\
            "\tPause                     Quit\n"\
            "\tPrScr                     Capture screenshot (not implemented yet)\n"\
            "\tF11                       Save state (experimental)\n"\
            "\tF12                       Load state (experimental)\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        usage();
    }
    else {
        printf("%s ver. %s\n", kProgName, kProgVersion);
        
        int fullscreen = 0;
        int unpackmode = 0;

        const char *path = NULL;
        
        for (int c = 1; c < argc; c++)
        {
            const char * cmd = argv[c];
            if (strcmp(cmd, "-f") == 0)
            {
                fullscreen = 1;
            }
            else
            if (strcmp(cmd, "-u") == 0)
            {
                unpackmode = 1;
            }
            else
            {
                path = argv[c];
            }
        }

        sPlatform *pl = pl_guess(path, unpackmode);

        // Unpack mode
        if(pl_supported(pl) && unpackmode) {
            alis.platform = *pl;
            vram_init();
//          unpack_mode(alis.platform.path);
            return 0;
        }

        // Game mode
        if(pl_supported(pl)) {
            printf("#############################\n");
            printf("# System initialization...\n");
            printf("#############################\n");
            sys_init(pl, fullscreen);

            printf("#############################\n");
            printf("# ALIS VM initialization...\n");
            printf("#############################\n");
            alis_init(*pl);

            printf("#############################\n");
            printf("# Starting ALIS VM...\n");
            printf("#############################\n");
            alis_start();

            // Quit
            printf("\n");
            printf("#############################\n");
            printf("# System deinitialization...\n");
            printf("#############################\n");
            sys_deinit();

            printf("\n");
            printf("#############################\n");
            printf("# Releasing ALIS VM memory...\n");
            printf("#############################\n");
            alis_deinit();
        }
        else {
//          debug(EDebugFatal, "Platform '%s' is not supported.\n", pl->desc);
        }
    }
    return 0;
}
