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

#include "debug.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static char* debug_colors[] = {
    ANSI_COLOR_RED,
    ANSI_COLOR_RED,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_CYAN
};

static char* debug_prefix[] = {
    "\x1b[1m[FATAL]",
    "[ERROR]",
    "[WARNING]",
    "[INFO]",
    "[VERBOSE]"
};

void debug(EDebugLevel level, char * format, ...) {
    if(level <= DEBUG_LEVEL) {
        va_list arg;
        va_start(arg, format);
        printf("%s%s ", debug_colors[level], debug_prefix[level]);
        vprintf(format, arg);
        printf("%s", ANSI_COLOR_RESET);
        fflush(stdout);
        va_end(arg);
    }
}
