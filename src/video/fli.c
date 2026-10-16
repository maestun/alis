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

// FLI/FLC: Autodesk Animator films of the DOS/Falcon versions, incl. audio chunks
// and streaming of films that do not fit in RAM.

#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "../sys/sys.h"
#include "utils.h"
#include "video.h"


#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
#elif defined(_MSC_VER)
# include "SDL.h"
#elif __has_include(<SDL.h>)
# include <SDL.h>
#else
# include <SDL2/SDL.h>
#endif

// Dirty-rect state: SDL1 + native Atari only (SDL2 renders full-frame).
#if ALIS_SDL_VER < 2
extern volatile u8 dirty_pal;
extern SDL_Rect dirty_rects[256];
extern u8 dirty_len;
#endif

extern void dbglog(const char *fmt, ...);

void fli_palette(u8 *addr)
{
    if (!dos_film_palette(addr))
    {
        u16 packets = read16le(addr), index = 0;
        addr+=2;
    
        do
        {
            index += *(addr); addr++;
            u16 len = *(addr); addr++;
            if (len == 0)
            {
                len = 256;
            }
        
            if (index + len > 256)
            {
                len = 256 - index;
                packets = 1;
            }

            PAL_WRITE(image.mpalet, index, len, RGB18(addr));
            index += len;
        }
        while (--packets);
    }

    image.ftopal = 0xff;

#if defined(ALIS_FLI_FALCON16)
    // 16-bit film path: refresh the 8->16 LUT now so this frame's own pixel chunks
    // (which follow in the same fli_next) decode with the new colours.
    extern u16 *g_fli16_screen;
    extern void fli16_build_lut(void);
    if (g_fli16_screen)
        fli16_build_lut();
#endif
}

void fli_blackdata(void)
{
    memset(vgalogic, 0, bfilm.width * bfilm.height);
#if ALIS_SDL_VER < 2
    dirty_rects[0] = (SDL_Rect){ .x = image.fenx1, .y = image.feny1, .w = bfilm.width, .h = bfilm.height };
    dirty_len = 0xff;
#endif
}

void fli_data(u8 *addr)
{
    u8 *ptr = vgalogic;
    for (int i = 0; i < bfilm.height; i++, ptr += bfilm.width, addr += bfilm.width)
    {
        memcpy(ptr, addr, bfilm.width);
    }
#if ALIS_SDL_VER < 2
    dirty_rects[0] = (SDL_Rect){ .x = image.fenx1, .y = image.feny1, .w = bfilm.width, .h = bfilm.height };
    dirty_len = 0xff;
#endif
}

// Optimised FLC decoder (ALIS_FLI_OPT); fli_decomp_ref is the bit-exact reference.
// No divide, no libc call, invariants hoisted so the loop fits the registers left by a4/a5.

// Reference decoder, parameterised on target buffer + dirty state (flag-off path).
#if !defined(ALIS_FLI_OPT)
static void fli_decomp_ref(u8 *addr, u8 partial, u8 *vga, SDL_Rect *dr, u8 *dlp)
{
    u8 dirty_len = *dlp;
    u32 index = 0;
    u16 len = bfilm.height;

    if (partial)
    {
        index = read16le(addr) * bfilm.width; addr+=2;
        len = read16le(addr); addr+=2;
    }

    // guard: don't write past the vgalogic frame buffer
    u32 vgamax = (u32)(vgalogic_sz - (vgalogic - pvgalogic));

    while (len--)
    {
        if (index + bfilm.width > vgamax)
            break;
        
        u8 packets = *addr; addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
            {
                col += *(addr); addr++;
            }

            short int count = (signed char) *(addr); addr++;
            if (partial)
                count = -count;
            
            if (count >= 0)
            {
                if (count == 0)
                    count = 256;

                if (col + count > bfilm.width)
                {
                    count = bfilm.width - col;
                    len = packets = 0;
                }

                memset(vga + index + col, *(addr), count); addr++;
#if ALIS_SDL_VER < 2
            if (dirty_len > 0xfd)
            {
                dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                dirty_len = 0xff;
            }
            else
            {
                dr[dirty_len] = (SDL_Rect){.x = image.fenx1 + (index + col) % bfilm.width, .y = image.feny1 + (index + col) / bfilm.width, .w = count, .h = 1 };
                dirty_len++;
            }
#endif
            }
            else
            {
                count = -count;
                if (col + count > bfilm.width)
                {
                    count = bfilm.width - col;
                    len = packets = 0;
                }

                memcpy(vga + index + col, addr, count); addr+= count;
#if ALIS_SDL_VER < 2
                if (dirty_len > 0xfd)
                {
                    dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                    dirty_len = 0xff;
                }
                else
                {
                    dr[dirty_len] = (SDL_Rect){.x = image.fenx1 + (index + col) % bfilm.width, .y = image.feny1 + (index + col) / bfilm.width, .w = count, .h = 1 };
                    dirty_len++;
                }
#endif
            }

            col += count;
        }

        index += bfilm.width;
    }

    *dlp = dirty_len;
}
#endif /* reference compiled only when used */

#if defined(ALIS_FLI_OPT)
// Inline run fill/copy; FLI_OPT_ATTR keeps gcc from turning them back into memset/memcpy.
// Misaligned long access is legal on 020+.
static inline void fli_fill_run(u8 *d, u8 v, u32 n)
{
    u32 w = (u32)v; w |= w << 8; w |= w << 16;
    while (n >= 4)
    {
        *(u32 *)d = w;
        d += 4;
        n -= 4;
    }
    while (n)
    {
        *d++ = v;
        n--;
    }
}

static inline void fli_copy_run(u8 *d, const u8 *s, u32 n)
{
    while (n >= 4)
    {
        *(u32 *)d = *(const u32 *)s;
        d += 4;
        s += 4;
        n -= 4;
    }
    while (n)
    {
        *d++ = *s++;
        n--;
    }
}

// `partial` is a compile-time constant in each entry point.
static inline __attribute__((always_inline)) void
fli_decomp_impl(u8 *addr, u8 partial, u8 *vga, SDL_Rect *dr, u8 *dlp)
{
    const u16 W = bfilm.width;
#if ALIS_SDL_VER < 2
    const s16 fx = image.fenx1, fy = image.feny1;
#endif
    u8 *rowptr;
    u16 line, len;
    u8  dl = *dlp;

    if (partial)
    {
        line = read16le(addr);
        addr += 2;
        rowptr = vga + (u32)line * W;
        len = read16le(addr);
        addr += 2;
    }
    else
    {
        line = 0;
        rowptr = vga;
        len = bfilm.height;
    }

    while (len--)
    {
        u8  packets = *addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
                col += *addr++;

            short count = (signed char)*addr++;
            if (partial)
                count = -count;

            if (count >= 0)
            {
                if (count == 0)
                    count = 256;
                
                if (col + count > W)
                {
                    count = W - col;
                    len = packets = 0;
                }

                fli_fill_run(rowptr + col, *addr++, (u32)(u16)count);
            }
            else
            {
                count = -count;
                if (col + count > W)
                {
                    count = W - col;
                    len = packets = 0;
                }

                fli_copy_run(rowptr + col, addr, (u32)(u16)count); addr += count;
            }
#if ALIS_SDL_VER < 2
            if (dl > 0xfd)
            {
                dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                dl = 0xff;
            }
            else
            {
                dr[dl] = (SDL_Rect){ .x = fx + col, .y = fy + line, .w = count, .h = 1 };
                dl++;
            }
#endif
            col += count;
        }

        rowptr += W; line++;
    }

    *dlp = dl;
}

#define FLI_OPT_ATTR __attribute__((noinline, optimize("no-tree-loop-distribute-patterns,no-unroll-loops,no-peel-loops")))
FLI_OPT_ATTR static void fli_decomp_brun(u8 *a, u8 *vga, SDL_Rect *dr, u8 *dlp) { fli_decomp_impl(a, 0, vga, dr, dlp); }
FLI_OPT_ATTR static void fli_decomp_lc  (u8 *a, u8 *vga, SDL_Rect *dr, u8 *dlp) { fli_decomp_impl(a, 1, vga, dr, dlp); }
#endif /* ALIS_FLI_OPT */

void fli_decomp(u8 *addr, u8 partial)
{
#if defined(ALIS_FLI_OPT)
    if (partial)
    {
        fli_decomp_lc(addr, vgalogic, dirty_rects, &dirty_len);
    }
    else
    {
        fli_decomp_brun(addr, vgalogic, dirty_rects, &dirty_len);
    }
#elif ALIS_SDL_VER < 2
    fli_decomp_ref(addr, partial, vgalogic, dirty_rects, &dirty_len);
#else
    // SDL2 renders full-frame: no dirty tracking.
    u8 _dl = 0;
    fli_decomp_ref(addr, partial, vgalogic, NULL, &_dl);
#endif
}


// Per-slot copies of queued FLI speech chunks (bytes = freq * waitclock / 60; larger
// chunks are split). Allocated on first use, never freed: the mixer drains asynchronously.
#define FLI_AUDIO_CHUNK_MAX 4096
typedef s8 fli_audio_slot_t[FLI_AUDIO_CHUNK_MAX];
static fli_audio_slot_t *fli_audio_copybuf = NULL;

// Scratch for downconverting Deus's 16-bit (0x1300) FLI audio to 8-bit; the queue copies it out.
#define FLI_DC_BYTES (128 * 1024)
static s8 *fli_dc_buf = NULL;

// Push one per-frame audio chunk onto the FLI audio queue. The audio callback splices queued
// chunks back-to-back on channel 3 (no per-frame channel reset -> no clicking) and runfilm paces
// the video to the queued audio's real-time duration, keeping A/V in sync.
void fli_audio(u8 *nextaddr, u8 *addr, s16 type)
{
    u32 length = (u32)(nextaddr - addr);
    if (length == 0)
        return;

    // Chunks are word-aligned: drop the padding byte of odd-length data (else a click).
    if (length & 1u)
        length--;


    // Channel 3 set directly (playsample clamps freq too low for 22 kHz).
    // Rate = chunk bytes * 60 / waitclock: FLI files are sized for the DOS 60 Hz PIT
    // on every platform, so not sys_sfx_tick_hz.
    u32 src_fps = 60UL;

    s8 *sample_addr;
    u32 sample_len;

    if (type == 0x1300)
    {
        // Deus's single audio track is 16-bit signed LE PCM. The 8-bit channel mixer can't play it
        // directly, so downconvert into this slot's scratch buffer and queue that
        // sample_len is the 8-bit sample count
        if (fli_dc_buf == NULL && (fli_dc_buf = (s8 *)malloc(FLI_DC_BYTES)) == NULL)
            return;
        u32 nsamp = length / 2;
        if (nsamp > FLI_DC_BYTES)
            nsamp = FLI_DC_BYTES;
        for (u32 i = 0; i < nsamp; i++)
            fli_dc_buf[i] = (s8)addr[i * 2 + 1];   // high byte of little-endian s16
        sample_addr = fli_dc_buf;
        sample_len  = nsamp;
    }
    else
    {
        // Per-language 8-bit audio tracks, selected by the bfilm.id
        // bitmask. type 0x1000 -> bit 8 (French), 0x1001 -> bit 9 (English), 0x1002 -> bit 10 (German)
        u16 mask = (u16)(1 << (type - 0x1000)) << 8;
        if ((mask & (u16)bfilm.id) == 0)
            return;
        sample_addr = (s8 *)addr;
        sample_len  = length;
    }

    // Sample rate = (samples * original_FLI_tick_hz) / waitclock so the chunk plays exactly one
    // waitclock-frame at the FLI's authored tempo
    s16 freq;
    {
        u32 f = (bfilm.waitclock > 0) ? (src_fps * sample_len) / (u32)bfilm.waitclock : 22050UL;
        if (f > 32767UL)
            f = 32767UL;                  // sChannel.freq is s16
        
        freq = (s16)f;
    }

    fli_audio_push(sample_addr, sample_len, freq);
}

// Queue samples on channel 3 for gapless playback; returns how many fit
u32 fli_audio_push(const s8 *src, u32 len, s16 freq)
{
    // Chunks are copied, not referenced: a streamed film recycles its window before the mixer drains the queue
    if (fli_audio_copybuf == NULL)
    {
        fli_audio_copybuf = (fli_audio_slot_t *)
            malloc(sizeof(fli_audio_slot_t) * FLI_AUDIO_QUEUE_SIZE);
        
        if (fli_audio_copybuf == NULL)
            return len;                  // no room for speech buffers — film plays silent
    }

    u32 left = len;
    while (left != 0)
    {
        u32 piece = (left > FLI_AUDIO_CHUNK_MAX) ? FLI_AUDIO_CHUNK_MAX : left;
        u8 next = (u8)((fli_audio_q_head + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
        if (next == fli_audio_q_tail)
            break;                       // queue full (~2 s buffered)
        
        memcpy(fli_audio_copybuf[fli_audio_q_head], src, piece);
        fli_audio_queue[fli_audio_q_head].addr   = fli_audio_copybuf[fli_audio_q_head];
        fli_audio_queue[fli_audio_q_head].length = piece;
        fli_audio_queue[fli_audio_q_head].freq   = freq;
        fli_audio_q_head = next;
        src  += piece;
        left -= piece;
    }

    // Prime idle channel 3; the mixer then advances through the queue itself.
    sChannel *canal = &audio.channels[3];
    if (canal->type == eChannelTypeNone && fli_audio_q_head != fli_audio_q_tail)
    {
        u8 t = fli_audio_q_tail;
        canal->type = eChannelTypeNone;   // gate against IRQ mid-update
        canal->address = fli_audio_queue[t].addr;
        canal->length  = fli_audio_queue[t].length;
        canal->freq    = fli_audio_queue[t].freq;
        canal->volume  = 0x7f;
        canal->loop    = 0;
        canal->played  = 0;
        canal->curson  = 0x7f;
        fli_audio_q_tail = (u8)((t + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
        canal->type = eChannelTypeSample; // last write — re-arm
    }

    return len - left;
}

void fli_palette256(u8 *addr)
{
    u16 packets = read16le(addr), index = 0;
    addr += 2;

    do
    {
        index += *(addr); addr++;
        u16 len = *(addr); addr++;
        if (len == 0)
            len = 256;

        if (index + len > 256)
        {
            len = 256 - index;
            packets = 1;
        }

        PAL_WRITE(image.mpalet, index, len, RGB24(addr));

        index += len;
    }
    while (--packets);

    image.ftopal = 0xff;
}

void fli_ss2(u8 *addr)
{
    u16 lines = read16le(addr); addr += 2;
    u32 y = 0;
    u32 vgamax = (u32)(vgalogic_sz - (vgalogic - pvgalogic));

    while (lines > 0)
    {
        u16 opcode = read16le(addr); addr += 2;
        u16 sel = opcode & 0xc000;

        if (sel == 0xc000)
        {
            y += (u16)(-(s16)opcode);
            continue;
        }

        if (sel == 0x8000)
        {
            if ((u32)(y * bfilm.width + bfilm.width - 1) < vgamax)
                vgalogic[y * bfilm.width + bfilm.width - 1] = (u8)(opcode & 0xff);
            continue;
        }

        // packet count for this line
        u16 packets = opcode;
        u32 x = 0;
        u8 *line = vgalogic + y * bfilm.width;

        while (packets--)
        {
            x += *(addr); addr++;
            s8 count = (s8)*(addr);
            addr++;

            if (count >= 0)
            {
                u32 n = (u32)count * 2;
                if (x + n > bfilm.width)
                {
                    n = (x < bfilm.width) ? bfilm.width - x : 0;
                }
                
                if ((u32)(y * bfilm.width + x + n) <= vgamax)
                {
                    memcpy(line + x, addr, n);
                }

                addr += (u32)count * 2;
                x += n;
            }
            else
            {
                u32 words = (u32)(-count);
                u8 b0 = addr[0], b1 = addr[1]; addr += 2;
                for (u32 w = 0; w < words && x + 2 <= bfilm.width; w++)
                {
                    if ((u32)(y * bfilm.width + x + 1) < vgamax)
                    {
                        line[x] = b0;
                        line[x + 1] = b1;
                    }

                    x += 2;
                }
            }
        }

        y++;
        lines--;
    }
}

// Falcon 16-bit film path (ALIS_FLI_FALCON16), as the CD original: the FLC decoder
// LUT-expands each index to RGB565 straight into the framebuffer (no c2p).
// Selected when g_fli16_screen != NULL (sys_film16_begin).
#if defined(ALIS_FLI_FALCON16)
u16 *g_fli16_screen = NULL;   // 16-bit RGB565 film framebuffer (Videl BPS16); NULL = planar path
u32  g_fli16_stride = 320;    // screen width in PIXELS (row stride); set by sys_film16_begin
u16  fli_lut16[256];          // 8-bit index -> RGB565, rebuilt from the film palette

// Rebuild the 8->16 LUT from the film palette (same RGB565 fold as sys_pal16_refresh).
void fli16_build_lut(void)
{
    for (int i = 0; i < 256; i++)
    {
        sColorARGB *c = (sColorARGB *)&image.mpalet[i];
        fli_lut16[i] = (u16)(((c->r >> 3) << 11) | ((c->g >> 2) << 5) | (c->b >> 3));
    }
}

// Long-word run fill (2 RGB565 px/store) + word tail. dst = screen + pixel offset
// (word-aligned; 030 tolerates the odd case).
static inline void fli16_fill(u16 *d, u16 w, u32 n)
{
    u32 ww = ((u32)w << 16) | w;
    while (n >= 2)
    {
        *(u32 *)d = ww;
        d += 2;
        n -= 2;
    }

    if (n)
        *d = w;
}

static inline void fli16_expand(u16 *d, const u8 *s, u32 n)
{
    while (n--)
        *d++ = fli_lut16[*s++];
}

// LC (partial=1) / BRUN (partial=0) decoder, RGB565 straight to g_fli16_screen.
__attribute__((optimize("no-tree-loop-distribute-patterns,no-unroll-loops,no-peel-loops")))
static void fli_decomp16(const u8 *addr, u8 partial)
{
    const u16 W = bfilm.width;
    const u32 stride = g_fli16_stride;                          // screen row stride (pixels)
    u16 *const base = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    u16 *rowptr; u16 line, len;
    if (partial)
    {
        line = read16le(addr);
        addr += 2;
        rowptr = base + (u32)line * stride;
        len = read16le(addr);
        addr += 2;
    }
    else
    {
        line = 0;
        rowptr = base;
        len = bfilm.height;
    }
    while (len--)
    {
        u8 packets = *addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
                col += *addr++;
            short count = (signed char)*addr++;
            if (partial)
                count = -count;
            if (count >= 0)
            {
                if (count == 0)
                    count = 256;
                if (col + count > W)
                {
                    count = W - col;
                    len = packets = 0;
                }

                fli16_fill(rowptr + col, fli_lut16[*addr++], (u32)(u16)count);
            }
            else
            {
                count = -count;
                if (col + count > W)
                {
                    count = W - col;
                    len = packets = 0;
                }

                fli16_expand(rowptr + col, addr, (u32)(u16)count); addr += count;
            }

            col += count;
        }

        rowptr += stride; line++;
    }
}

// Raw full-frame (chunk 0x10) and black (chunk 0xd) 16-bit variants — same screen
// stride + fenx1/feny1 positioning as the delta path.
static void fli_data16(const u8 *addr)
{
    const u32 stride = g_fli16_stride;
    u16 *row = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    for (int y = 0; y < bfilm.height; y++, row += stride, addr += bfilm.width)
        fli16_expand(row, addr, bfilm.width);
}

static void fli_blackdata16(void)
{
    const u32 stride = g_fli16_stride;
    u16 *row = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    for (int y = 0; y < bfilm.height; y++, row += stride)
        fli16_fill(row, fli_lut16[0], bfilm.width);
}
#endif /* ALIS_FLI_FALCON16 */

void fli_elements(u8 *addr)
{
#if defined(ALIS_FLI_FALCON16)
    extern void sys_film16_keyframe(void);   // aim the 16-bit decoder at the off-screen back buffer
    extern void sys_film16_present(void);    // flip to it on vblank (hides the keyframe wipe-in)
#endif
    while (addr < endframe)
    {
        u32 offset = read32le(addr);
        addr+=4;

        u16 type = read16le(addr);
        addr+=2;

        u8 *nextaddr = addr + offset - 6;

        switch (type)
        {
            case 0x4:                 // FLC 256-colour palette
                fli_palette256(addr);
                break;
            case 0x7:                 // FLC SS2 word delta
                fli_ss2(addr);
                break;
            case 0xb:
                fli_palette(addr);
                break;
            case 0xc:   // LC delta — modifies the previous frame -> in place on the displayed front
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen)
                {
                    fli_decomp16(addr, 1);
                    break;
                }
#endif
                fli_decomp(addr, 1);
                break;
            case 0xd:   // black — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen)
                {
                    sys_film16_keyframe();
                    fli_blackdata16();
                    sys_film16_present();
                    break;
                }
#endif
                fli_blackdata();
                break;
            case 0xf:   // BRUN — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen)
                {
                    sys_film16_keyframe();
                    fli_decomp16(addr, 0);
                    sys_film16_present();
                    break;
                }
#endif
                fli_decomp(addr, 0);
                break;
            case 0x10:  // raw — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen)
                {
                    sys_film16_keyframe();
                    fli_data16(addr);
                    sys_film16_present();
                    break;
                }
#endif
                fli_data(addr);
                break;

            default:
                // Silmarils audio extension: 0x1000..0x1007 are the 8 per-language track variants
                // (8-bit) and 0x1300 is Deus's single 16-bit track. Match only these so standard
                // FLC chunk types (0xe BRUN, 0x11 PSTAMP, 0x12 LZMA, ...) aren't fed to fli_audio.
                if ((type >= 0x1000 && type <= 0x1007) || type == 0x1300)
                    fli_audio(nextaddr, addr, type);
        }

        addr = nextaddr;
    }
}

// ============================================================================
#pragma mark - Film streaming (large films that do not fit in RAM)
// ============================================================================
// Fallback when the film doesn't fit: bounded sliding window, as the CD original.
// Each frame is made resident before parsing; the tail is compacted to the base
// only when a frame would run past the window end.

#define FLI_STREAM_TOPUP 32768   // minimum read size — keeps freads ~1/frame

int fli_stream_open(FILE *fp, u32 file_size)
{
    static const u32 sizes[] = { 512 * 1024, 256 * 1024, 128 * 1024 };
    u8 *buf = NULL;
    u32 cap = 0;
    for (int i = 0; i < 3 && buf == NULL; i++)
    {
        cap = sizes[i];
        buf = malloc(cap);
    }

    if (buf == NULL)
    {
        dbglog("[flistream] open FAILED: no RAM even for a %u-byte window (film %u bytes)\n",
               sizes[2], file_size);
        return 0;
    }

    fseek(fp, 0, SEEK_SET);
    bfilm.sfp        = fp;
    bfilm.sbuf       = buf;
    bfilm.sbuf_size  = cap;
    bfilm.sfill      = buf;          // window empty — fli_stream_ensure primes on demand
    bfilm.sremain    = file_size;
    bfilm.sfile_size = file_size;
    dbglog("[flistream] streaming film: size=%u window=%u\n", file_size, cap);
    return 1;
}

void fli_stream_close(void)
{
    if (bfilm.sfp != NULL)
    {
        fclose(bfilm.sfp);
        bfilm.sfp = NULL;
    }

    // The window buffer is owned by bfilm.delptr (cdelfilm frees it).
    bfilm.sbuf = NULL;
    bfilm.sfill = NULL;
    bfilm.sbuf_size = 0;
    bfilm.sremain = 0;
    bfilm.sfile_size = 0;
}

// Make `need` bytes starting at bfilm.addr1 resident in the window.
// Returns 0 when that can never happen (EOF / I/O error / frame > window).
// Keeps bfilm.endptr == bfilm.sfill so flitofen's bounds check stays valid.
static int fli_stream_ensure(u32 need)
{
    u32 have = (u32)(bfilm.sfill - bfilm.addr1);
    if (have >= need)
    {
        bfilm.endptr = bfilm.sfill;
        return 1;
    }

    if (need > bfilm.sbuf_size)
    {
        dbglog("[flistream] frame of %u bytes exceeds the %u window — aborting film\n",
               need, bfilm.sbuf_size);
        return 0;
    }

    // Compact only when the frame would extend past the window end.
    if ((u32)(bfilm.addr1 - bfilm.sbuf) + need > bfilm.sbuf_size)
    {
        memmove(bfilm.sbuf, bfilm.addr1, have);
        bfilm.addr1 = bfilm.sbuf;
        bfilm.sfill = bfilm.sbuf + have;
    }

    u32 space = bfilm.sbuf_size - (u32)(bfilm.sfill - bfilm.sbuf);
    u32 want = need - have;
    if (want < FLI_STREAM_TOPUP)
        want = FLI_STREAM_TOPUP;
    
    if (want > space)
        want = space;
    
    if (want > bfilm.sremain)
        want = bfilm.sremain;
    
    if (want != 0)
    {
        u32 got = (u32)fread(bfilm.sfill, 1, want, bfilm.sfp);
        bfilm.sfill += got;
        bfilm.sremain -= got;
        if (got < want)
            bfilm.sremain = 0;   // short read = EOF or I/O error — end the film cleanly
    }

    bfilm.endptr = bfilm.sfill;
    return (u32)(bfilm.sfill - bfilm.addr1) >= need;
}

// Make the whole chunk at bfilm.addr1 resident: the 0x80-byte FLC file
// header, or one complete 0xf1fa frame (its 32-bit length prefix tells us
// how much). Returns 0 at end of film.
static int fli_stream_frame(void)
{
    if (!fli_stream_ensure(6))
        return 0;
    u16 type = read16le(bfilm.addr1 + 4);
    u32 need = 6;
    if (type == 0xaf11)
    {
        need = 0x80;                      // FLC header; first frame starts at +0x80
    }
    else if (type == 0xf1fa)
    {
        need = read32le(bfilm.addr1);     // complete frame chunk
    }

    return fli_stream_ensure(need);
}

// Loop replay: rewind the file and empty the window; fli_stream_ensure
// re-primes on the next flitofen. Cheap when the film is about to be
// deleted instead of replayed (no read happens here).
void fli_stream_rewind(void)
{
    fseek(bfilm.sfp, 0, SEEK_SET);
    bfilm.sremain = bfilm.sfile_size;
    bfilm.sfill = bfilm.sbuf;
    bfilm.addr1 = bfilm.sbuf;
    bfilm.addr2 = bfilm.sbuf;
    bfilm.endptr = bfilm.sbuf;
}

void fli_init(u8 *flcaddr)
{
    u32 length = read32le(flcaddr);
    bfilm.endptr = flcaddr + length;
    bfilm.addr1 = flcaddr + 0x80;
    bfilm.frames = read16le(flcaddr + 6);
    bfilm.width = read16le(flcaddr + 8);
    bfilm.height = read16le(flcaddr + 10);
    bfilm.frame = 0;
    if (!vgalogic_fit((u32)(vgalogic - pvgalogic) + (u32)bfilm.width * bfilm.height))
        bfilm.width = bfilm.height = 0;

    // Streaming: the file-length-based endptr points far past the window;
    // the window fill boundary is the real bound.
    if (bfilm.sfp != NULL)
        bfilm.endptr = bfilm.sfill;
}

void fli_next(u8 *addr)
{
    u32 length = read32le(addr);
    endframe = addr + length;
    fli_elements(addr + 16);

#if ALIS_SDL_VER >= 2
    u16 fw = bfilm.width;
    u16 fh = bfilm.height;
    s16 dx = image.fenx1;
    s16 dy = image.feny1;
    u16 dw = alis.platform.width;

    for (int y = 0; y < fh; y++)
    {
        s32 row = dy + y;
        if (row < 0 || row >= alis.platform.height || dx < 0 || dx + fw > dw)
            continue;

        u8 *src = vgalogic + y * fw;
        u8 *dst = image.physic + row * dw + dx;
        memcpy(dst, src, fw);
    }

    memcpy(image.logic, image.physic, dw * alis.platform.height);
#endif

    bfilm.addr1 = endframe;
    bfilm.frame++;
}

s16 flitofen(void)
{
    while (bfilm.addr1 != NULL)
    {
        // Streaming film: make the chunk at addr1 fully resident first.
        if (bfilm.sfp != NULL && !fli_stream_frame())
            return 0;

        u16 type = read16le(bfilm.addr1 + 4);
        if (type == 0xaf11 || type == 0xaf12)
        {
            fli_init(bfilm.addr1);
            return 1;
        }
        else if (type == 0xf100 && bfilm.endptr > bfilm.addr1)
        {
            u32 length = read32le(bfilm.addr1);
            bfilm.addr1 += length ? length : 16;
            bfilm.frame++;
            continue;
        }
        else if (type == 0xf1fa && bfilm.endptr > bfilm.addr1)
        {
            fli_next(bfilm.addr1);
            return 1;
        }
        else
        {
            break;
        }
    }

    return 0;
}
