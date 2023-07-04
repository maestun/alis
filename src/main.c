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

void usage(void) {
    printf("%s v%s\nUsage:\n\t%s <data_path>\n\n\t%s <script_path>\n",
           kProgName, kProgVersion, kProgName, kProgName);
}

int main(int argc, const char* argv[]) {

    if (argc < 2) {
        usage();
    }
    else {
        sPlatform *pl = pl_guess(argv[1]);
        if(pl_supported(pl)) {
            sys_init();
            alis_init(*pl);
            alis_start();
            alis_deinit();
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
