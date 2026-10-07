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
#include "alis_private.h"
#include "audio.h"
#include "video.h"
#include <sys/stat.h>

// fileno is POSIX but may not be declared with strict C99
#if !defined(_WIN32)
extern int fileno(FILE *);
#endif

// ============================================================================
#pragma mark - Codesc1 routines
// ============================================================================

// Codesc1name no. 01 opcode 0x00 cnul
// Calls stub cnul

// Codesc1name no. 02 opcode 0x01 csoundon
// Known as consound in c-file
void csoundon(void) {
    audio.fsound = 1;
}

// Codesc1name no. 03 opcode 0x02 csoundoff
// Known as coffsound in c-file
void csoundoff(void) {
    audio.fsound = 0;
}

// Codesc1name no. 04 opcode 0x03 cmusicon
// Known as conmusic in c-file
void cmusicon(void) {
    audio.fmusic = 1;
}

// Codesc1name no. 05 opcode 0x04 cmusicoff
// Known as coffmusic in c-file
void cmusicoff(void) {
    audio.fmusic = 0;
}

// Codesc1name no. 06 opcode 0x05 cdelfilm
void cdelfilm(void) {
#if defined(ALIS_TRACE_KEYS)
    { extern void dbglog(const char *fmt, ...);
      dbglog("cdelfilm t=%u script=%s film=%d/%d\n", (unsigned)alis.timeclock, alis.script->name, (int)bfilm.frame, (int)bfilm.frames); }
#endif
    endfilm();

    fli_stream_close();   // close the streaming file handle (no-op in whole-file mode)

    if (bfilm.delptr != NULL)
        free(bfilm.delptr);

    memset(&bfilm, 0, sizeof(bfilm));
}

// Codesc1name no. 07 opcode 0x06 copenfilm
void copenfilm(void) {
#if defined(ALIS_TRACE_KEYS)
    { extern void dbglog(const char *fmt, ...);
      dbglog("copenfilm t=%u script=%s\n", (unsigned)alis.timeclock, alis.script->name); }
#endif
    fli_stream_close();   // stale streaming film (script skipped cdelfilm) — don't leak the handle

    memset(&bfilm, 0, sizeof(bfilm));

    readexec_opername();
    bfilm.id = alis.varD7;

    readexec_opername_swap();
    
    alis.fp = NULL;

    // The one CD carries both "150." (smaller, Falcon-sized) and "300." (fullscreen) video
    // variants; this remaps 150 -> 300 to force the fullscreen version. Native Atari (Falcon)
    // should use its own 150 videos: they play smoothly on the 16-bit fused film path and are
    // positioned correctly (fli_decomp16 honours fenx1/feny1). Keep the remap only off-target.
#if !defined(ALIS_USE_NATIVE_ATARI)
    char *at = strstr(alis.sd7, "150.");
    if (at != NULL)
    {
        memcpy(at, "300.", 4);
    }
#endif

    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, alis.sd7);

    afopen((char *)path, 1);

#if defined(ALIS_USE_NATIVE_ATARI)
    // Native Falcon prefers its own smaller "150." speech videos (they play on the
    // 16-bit fused film path and are positioned correctly), so the remap above is
    // skipped on-target. But not every data set ships the 150 variants — when the
    // 150 file is absent, afopen fails, addr stays NULL and the whole speech film
    // (and its audio) is silently skipped. Fall back to the "300." fullscreen
    // variant, the one the desktop build always opens successfully.
    if (alis.fp == NULL)
    {
        char *at = strstr(alis.sd7, "150.");
        if (at != NULL)
        {
            memcpy(at, "300.", 4);
            strcpy(path, alis.platform.path);
            strcat(path, alis.sd7);
            afopen((char *)path, 1);
        }
    }
#endif

    u8 *addr = NULL;

    if (alis.fp)
    {
        struct stat st;
        fstat(fileno(alis.fp), &st);
        off_t size = st.st_size;

#if defined(ALIS_FLI_STREAM_TEST)
        // Test build: force every film through the streaming path.
#else
        addr = malloc(size);
#endif
        if (addr)
        {
            fread(addr, size, 1, alis.fp);
        }
        else
        {
            // Not enough free RAM for the whole film — on a real Falcon the
            // game data leaves ~2.5 MB of heap while the large speech videos
            // run 3-7.5 MB, and this malloc failing silently skipped them.
            // Fall back to the original CD engine's scheme: stream the film
            // through a bounded sliding window (fli_stream_* in video.c).
            // fli_stream_open takes ownership of the file handle.
            if (fli_stream_open(alis.fp, (u32)size))
            {
                addr = bfilm.sbuf;
                alis.fp = NULL;
            }
            else
            {
                extern void dbglog(const char*,...);
                dbglog("[flistream] film '%s' (%ld bytes) SKIPPED — no RAM for it at all\n",
                       alis.sd7, (long)size);
            }
        }
    }

    bfilm.addr1 = addr;
    bfilm.addr2 = addr;
    bfilm.delptr = addr;
    // TODO: not realy sure what next value is used for
    readexec_opername();
    readexec_opername();
    bfilm.waitclock = alis.varD7;
    bfilm.basemain = alis.basemain;
    inifilm();
    alis.varD7 = bfilm.frames;
    cstore_continue();
}

// ============================================================================
#pragma mark - Codesc1 routines pointer table
// ============================================================================
const sAlisOpcode codesc1names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
    DECL_OPCODE(0x01, csoundon,     "sound on"),
    DECL_OPCODE(0x02, csoundoff,    "sound off"),
    DECL_OPCODE(0x03, cmusicon,     "music on"),
    DECL_OPCODE(0x04, cmusicoff,    "music off"),
    DECL_OPCODE(0x05, cdelfilm,     "close and delete video from memory"),
    DECL_OPCODE(0x06, copenfilm,    "open video")
};

// ============================================================================
#pragma mark - Codesc2 routines pointer table
// ============================================================================

const sAlisOpcode codesc2names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
};

// ============================================================================
#pragma mark - Codesc3 routines pointer table
// ============================================================================

const sAlisOpcode codesc3names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
};
