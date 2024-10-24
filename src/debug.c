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

// Before Windows 10, there was no native support for ANSI colors in Windows.
// But in Windows 10, it requires a special enablement with SetConsoleMode().
// We did not implement it yet. ANSI colors work well with CYGWIN.
#if defined (_WIN32) || defined (__MINGW32__) && !defined(__CYGWIN__)
#define DISABLE_ANSI_COLORS
// Enable color support even if not supported by the system natively
#define FORCE_ENABLE_ANSI_COLORS 0
#endif

// Disabling ANSI colors support by user
#define USER_DISABLE_ANSI_COLORS   0

#if (defined (DISABLE_ANSI_COLORS) && FORCE_ENABLE_ANSI_COLORS == 0) || USER_DISABLE_ANSI_COLORS == 1

#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_BLUE    ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN    ""
#define ANSI_COLOR_BRIGHT  ""
#define ANSI_COLOR_RESET   ""
#define NO_ANSI_COLOR      ""

#else

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_BRIGHT  "\x1b[1m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define NO_ANSI_COLOR      ""

#endif

static char* debug_colors[] = {
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_RED,
    ANSI_COLOR_RED,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_CYAN
};

static char* debug_prefix[] = {
    ANSI_COLOR_BRIGHT "[SYSTEM]",
    ANSI_COLOR_BRIGHT "[FATAL]",
    ANSI_COLOR_BRIGHT "[ERROR]",
    "[WARNING]",
    "[INFO]",
    "[VERBOSE]"
};

#ifndef DISABLE_DEBUG
void debug(EDebugLevel level, char * format, ...) {
    if((disalis) && (level < EDebugVerbose)) {
       va_list arg;
       va_start(arg, format);
       fflush(stdout);
       vprintf(format, arg);
       fflush(stdout);
    }
    else {
    if(level <= DEBUG_LEVEL) {
       va_list arg;
       va_start(arg, format);
       vdebug(level, format, arg);
       va_end(arg);
    }
  }
}

void vdebug(EDebugLevel level, char * format, va_list args) {
  if(level <= DEBUG_LEVEL) {
     printf("%s%s ", debug_colors[level], debug_prefix[level]);
     vprintf(format, args);
     printf("%s", ANSI_COLOR_RESET);
     fflush(stdout);
  }
}
#else
void debug(EDebugLevel level, char * format, ...) {
  if((disalis) && (level < EDebugVerbose)) {
     va_list arg;
     va_start(arg, format);
     fflush(stdout);
     vprintf(format, arg);
     fflush(stdout);
  }
}
#endif
