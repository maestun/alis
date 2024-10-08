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
#include "sys/sys.h"
#include "SDL2/SDL.h"

void usage(void) {
    printf("%s v%s\nUsage:\n\t%s [-f] <data_path>\n\n\t%s <data_path>\n",
           kProgName, kProgVersion, kProgName, kProgName);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        usage();
    }
    else {
        printf("%s v%s\n", kProgName, kProgVersion);
        
        int fullscreen = 0;
        const char *path = NULL;
        
        for (int c = 1; c < argc; c++)
        {
            const char * cmd = argv[c];
            if (strcmp(cmd, "-f") == 0)
            {
                fullscreen = 1;
            }
            else
            {
                path = argv[c];
            }
        }
        
        sPlatform *pl = pl_guess(path);
        if(pl_supported(pl)) {
        if(pl_supported(pl)) {
            printf("System initialization...\n");
            sys_init(pl, fullscreen);
            printf("ALIS VM initialization...\n");
            alis_init(*pl);
            printf("Starting ALIS VM...\n");
            alis_start();
            // Quit
            printf("Releasing ALIS VM memory...\n");
            alis_deinit();
            printf("System deinitialization...\n");
            sys_deinit();
        }
        else {
            debug(EDebugFatal,
                  "Platform '%s' is not supported.\n",
                  pl->desc);
        }
    }
    return 0;
}
