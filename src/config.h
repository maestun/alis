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

#ifdef _MSC_VER
# include "dirent.h"
#elif __GNUC__
# include <dirent.h>
# include <unistd.h>
# include <sys/time.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#define kProgName           "alis"
#define kProgVersion        "0.1.039" // temporal version numbering scheme: x.y.zzz; zzz - pull request number
#define kPathMaxLen         (256)
#define kNameMaxLen         (16)
#define kDescMaxLen         (1024)
#define kSaveStateVersion   "0003"

#if defined (_WIN32) || defined (__CYGWIN__)
#  define kPathSeparator    '\\'
#else
#  define kPathSeparator    '/'
#endif

typedef uint8_t             u8;
typedef uint16_t            u16;
typedef uint32_t            u32;
typedef uint64_t            u64;
typedef int8_t              s8;
typedef int16_t             s16;
typedef int32_t             s32;
typedef int64_t             s64;

// TODO: move this
#define BIT_CLR(v, b)       (v &= ~(1UL << b))
#define BIT_SET(v, b)       (v |= (1UL << b))
#define BIT_CHG(v, b)       (v ^= (1UL << b))
#define BIT_CHK(v, b)       ((v >> b) & 1U)

typedef enum {
    EDebugSystem = 0,
    EDebugFatal,
    EDebugError,
    EDebugWarning,
    EDebugInfo,
    EDebugVerbose
} EDebugLevel;

#define DEBUG_LEVEL         EDebugWarning
#define DEBUG_SCRIPT        0
#define VM_IGNORE_ERRORS    0

#ifndef ALIS_SDL_VER
# define ALIS_SDL_VER       2
#endif

#if ALIS_SDL_VER <= 1
# define ALIS_USE_THREADS   0
#else
# define ALIS_USE_THREADS   1
#endif

extern int disalis;
