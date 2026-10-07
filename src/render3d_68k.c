//
//  render3d_68k.c
//  alis
//
//  m68k-specific terrain rendering functions extracted from render3d.c
//

#include <string.h>
#include "alis.h"
#include "image.h"
#include "mem.h"
#include "render3d.h"
#include "render3d_68k.h"

#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
extern SDL_Rect dirty_rects[256];
extern u8 dirty_len;
#endif

// Shared globals defined in render3d.c
extern u8 fprectop;
extern u8 fprectopa;
extern u8 fbottom;
extern u8 notopa;
extern u16 prectopa;
extern u16 precbota;
extern u16 prectopb;
extern u16 precbotb;
extern u16 prectopc;
extern u16 precbotc;
extern u16 prectopi;
extern u16 precboti;
extern u16 botalt;
extern u16 bothigh;
extern u16 solha;
extern u32 adresa;
extern u8 terrain_fill_color;
extern s16 bartra_saved_si;
extern s16 bottom_type_index;

#if defined(ALIS_MEASURE_OVERDRAW)
// Overdraw stats: stamp each bar's rect with its id, report how much survives per frame.
#define OD_W 512
#define OD_H 256
#define OD_MAXBARS 65535
static u16 od_id[OD_H][OD_W];
static u16 od_row_of[OD_MAXBARS + 1];
static u32 od_bars, od_px, od_cells, od_rows, od_sprites, od_frames;
static u32 od_cells_in_row[1024];
static u64 od_sum_cells, od_sum_bars, od_sum_px, od_sum_vis_px, od_sum_vis_bars, od_sum_dead_cells, od_sum_sprites;

u32 g_zoom_px, g_zoom_opaque;

static void od_begin(void)
{
    memset(od_id, 0, sizeof(od_id));
    od_bars = od_px = od_cells = od_rows = od_sprites = 0;
}

static void od_bar(s16 x, s16 y, s16 w, s16 h)
{
    if (w <= 0 || h <= 0 || od_bars >= OD_MAXBARS) return;
    u16 id = (u16)++od_bars;
    od_row_of[id] = (u16)od_rows;
    od_px += (u32)w * (u32)h;
    for (s16 r = y; r < y + h; r++)
        for (s16 c = x; c < x + w; c++)
            if (r >= 0 && r < OD_H && c >= 0 && c < OD_W) od_id[r][c] = id;
}

static void od_end(void)
{
    static u8 seen[OD_MAXBARS + 1];
    memset(seen, 0, od_bars + 1);
    u32 vis_px = 0, vis_bars = 0, first_vis_row = od_rows;
    for (int r = 0; r < OD_H; r++)
        for (int c = 0; c < OD_W; c++)
        {
            u16 id = od_id[r][c];
            if (!id) continue;
            vis_px++;
            if (!seen[id]) { seen[id] = 1; vis_bars++; if (od_row_of[id] < first_vis_row) first_vis_row = od_row_of[id]; }
        }
    // Rows painted before the farthest visible one are fully hidden: front-to-back never walks them.
    u32 dead_cells = 0;
    for (u32 r = 0; r < first_vis_row && r < 1024; r++) dead_cells += od_cells_in_row[r];

    od_frames++;
    od_sum_cells += od_cells; od_sum_bars += od_bars; od_sum_px += od_px;
    od_sum_vis_px += vis_px; od_sum_vis_bars += vis_bars; od_sum_dead_cells += dead_cells; od_sum_sprites += od_sprites;
    if (od_frames % 50 == 0)
        printf("[overdraw] %u frames avg: rows %u cells %llu bars %llu (visible %llu = %.0f%%) px %llu visible %llu overdraw %.2fx sprites %llu; cells in fully hidden far rows %.0f%%\n",
               od_frames, od_rows, od_sum_cells / od_frames, od_sum_bars / od_frames, od_sum_vis_bars / od_frames,
               100.0 * od_sum_vis_bars / (od_sum_bars ? od_sum_bars : 1), od_sum_px / od_frames, od_sum_vis_px / od_frames,
               (double)od_sum_px / (od_sum_vis_px ? od_sum_vis_px : 1), od_sum_sprites / od_frames,
               100.0 * od_sum_dead_cells / (od_sum_cells ? od_sum_cells : 1));
    if (od_frames % 50 == 0)
        printf("[overdraw] billboard pixels %u, opaque %u (%.0f%%)\n", g_zoom_px, g_zoom_opaque,
               100.0 * g_zoom_opaque / (g_zoom_px ? g_zoom_px : 1));
}
#define OD(x) x
#else
#define OD(x)
#endif

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
// Harness pixel watch: after each bar, log when the watched byte changed and which C paths ran.
u32 rrq_watch;            // alis.mem offset of the watched byte, 0 = off
static s16 rrq_cell_x, rrq_cell_y;
// Row trace for the harness: 1 = C run, 2 = asm run (asm calls rrq_row_note from doland1).
u8 rrq_rowlog;
void rrq_bar_note(s32 kind, s32 row, s32 x, s32 top, s32 height, s32 precx)
{
    if (!rrq_rowlog)
        return;
    extern void dbglog(const char *fmt, ...);
    dbglog("[rrqbar] %c %c %d x=%d top=%d h=%d precx=%d\n", rrq_rowlog == 1 ? 'C' : 'A', kind == 1 ? 'g' : 't',
           (int)row, (int)(s16)x, (int)(s16)top, (int)(s16)height, (int)(s16)precx);
}

void rrq_row_note(s32 row, u32 colx, u32 step, u32 tx, u32 ty, u32 sx, u32 sy)
{
    if (!rrq_rowlog)
        return;
    extern void dbglog(const char *fmt, ...);
    dbglog("[rrqrow] %c %d x=%08x st=%08x t=%08x,%08x s=%08x,%08x\n", rrq_rowlog == 1 ? 'C' : 'A',
           (int)row, colx, step, tx, ty, sx, sy);
}
u8  rrq_watch_val;
static char rrq_path[24];
static u8 rrq_path_n;
#define RRQ_PATH(c) do { if (rrq_path_n < sizeof(rrq_path) - 1) rrq_path[rrq_path_n++] = (c); } while (0)
static void rrq_watch_bar(const char *kind, s32 rc, s16 a, s16 b, s16 c, s16 d)
{
    rrq_path[rrq_path_n] = 0;
    rrq_path_n = 0;
    if (!rrq_watch || alis.mem[rrq_watch] == rrq_watch_val)
        return;
    extern void dbglog(const char *fmt, ...);
    dbglog("[rrqw] %s %d,%d,%d,%d path=%s %02x->%02x precx=%d vbarx=%d vbarlarg=%d r246=%d r25c=%d r24e=%d vbarbot=%d bothigh=%d botalt=%d precboti=%d cell=%d,%d solh=%d solpixy=%d r276=%d r25a=%d\n",
           kind, a, b, c, d, rrq_path, rrq_watch_val, alis.mem[rrq_watch], image.precx, image.vbarx, image.vbarlarg,
           xread16(rc - 0x246), xread16(rc - 0x25c), xread16(rc - 0x24e), image.vbarbot, bothigh, botalt, precboti,
           rrq_cell_x, rrq_cell_y, image.solh, image.solpixy, xread16(rc - 0x276), xread16(rc - 0x25a));
    rrq_watch_val = alis.mem[rrq_watch];
}
#define RRQ_WATCH(...) rrq_watch_bar(__VA_ARGS__)
#else
#define RRQ_PATH(c)
#define RRQ_WATCH(...)
#endif

void vgatofen_68k(void)
{
    image.switchgo = 1;
    u8 *src = image.wlogic + (image.clipx1 + (image.clipy1 - image.wlogy1) * image.wloglarg);

    image.vgamodulo = image.wloglarg - ((image.clipx2 - image.clipx1) + 1);
    image.bitmodulo = image.loglarg * 2 - ((image.clipx2 - image.clipx1) + 1);

    u8 limit = alis.platform.bpp == 4 ? 0xf : 0xff;

#if defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
    // Native planar: c2p the viewport rows into the planar logic buffer. X is snapped out to
    // 16-px chunks; the extra columns are inside wloglarg and redrawn by sprites afterward.
    {
        extern void chunky8_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst);
        s32 pitch = alis.platform.width;                    // planar pitch (bytes/row) == width
        s32 x0 = image.clipx1 & ~15;                        // align start down to a chunk
        s32 x1 = image.clipx2 | 15;                         // align end up (inclusive)
        s32 w  = x1 - x0 + 1;                               // multiple of 16
        // 4-bit games: mask to planes 0-3 (clrvga's 0x10 fill and shade bytes set high bits).
        static u8 vf_row[352];
#if defined(__m68k__) && !defined(ALIS_NO_C2P_ASM)
        if (limit == 0xff && image.clipy2 >= image.clipy1 && w > 0) {
            extern void c2p8_rect(const u8 *src, s32 src_pitch, u8 *dst, s32 dst_pitch, s32 chunks, s32 rows);
            const u8 *src0 = image.wlogic + x0 + (image.clipy1 - image.wlogy1) * image.wloglarg;
            u8 *dst0 = image.logic + (u32)image.clipy1 * pitch + x0;
            s32 rows = image.clipy2 - image.clipy1 + 1;
            c2p8_rect(src0, image.wloglarg, dst0, pitch, w >> 4, rows);
#if defined(ALIS_C2P_VERIFY)
            {
                // Convert again into a private buffer (the cursor ISR may touch the screen) and
                // compare with the C converter.
                extern void dbglog(const char *fmt, ...);
                static u8 *tmp, ref[352];
                static u32 calls, bad;
                if (!tmp) tmp = malloc(352 * 256);
                if (tmp && rows <= 256 && w <= 352) {
                    c2p8_rect(src0, image.wloglarg, tmp, w, w >> 4, rows);
                    calls++;
                    for (s32 y = 0; y < rows; y++) {
                        chunky8_to_planar(src0 + y * image.wloglarg, w, 1, 0, ref);
                        if (memcmp(ref, tmp + y * w, w)) { bad++; dbglog("[c2p] row %d differs\n", (int)y); break; }
                    }
                    if ((calls & 63) == 0) dbglog("[c2p] %u calls, %u bad\n", calls, bad);
                }
            }
#endif
        } else
#endif
        for (int y = image.clipy1; y <= image.clipy2; y++) {
            u8 *src_row = image.wlogic + x0 + (y - image.wlogy1) * image.wloglarg;
            u8 *dst_row = image.logic + (u32)y * pitch + x0; // x0 16-aligned → planar byte offset == x0
            if (limit != 0xff) { for (s32 i = 0; i < w; i++) vf_row[i] = src_row[i] & limit; src_row = vf_row; }
            chunky8_to_planar(src_row, w, 1, 0, dst_row);
        }
#if ALIS_SDL_VER < 2
        if (dirty_len > 0xfd) { dirty_rects[0] = (SDL_Rect){0,0,host.pixelbuf.w,host.pixelbuf.h}; dirty_len = 0xff; }
        else { dirty_rects[dirty_len] = (SDL_Rect){ x0, image.clipy1, w, image.clipy2 - image.clipy1 + 1 }; dirty_len++; }
#endif
        return;
    }
#endif

    u8 *ptr = image.logic;
    u8 *tgt = ptr + image.clipx1 + image.clipy1 * image.loglarg * 2;
    s32 roww = (image.clipx2 - image.clipx1) + 1;
    // roww + bitmodulo == loglarg*2 (dst row stride); roww + vgamodulo == wloglarg (src stride).
    s32 dst_stride = (s32)image.loglarg * 2;
    s32 src_stride = (s32)image.wloglarg;
    if (limit == 0xff)
    {
        // 8-bit games: straight row copy (mint libc memcpy bursts via movem).
        for (int y = image.clipy1; y <= image.clipy2; y++, tgt += dst_stride, src += src_stride)
            memcpy(tgt, src, (size_t)roww);
    }
    else
    {
        // 4-bit games: mask to `limit` a long (4 px) at a time; high nibbles come from clrvga's
        // 0x10 fill and shade bytes written every frame.
        u32 m4 = 0x01010101u * (u32)limit;   // 0x0f0f0f0f
        for (int y = image.clipy1; y <= image.clipy2; y++, tgt += dst_stride, src += src_stride)
        {
            u8 *t = tgt, *s = src;
            s32 x = roww;
            for (; x >= 4; x -= 4, t += 4, s += 4) *(u32 *)t = *(u32 *)s & m4;
            for (; x > 0; x--, t++, s++) *t = *s & limit;
        }
    }

#if ALIS_SDL_VER < 2
    if (dirty_len > 0xfd)
    {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 0xff;
    }
    else
    {
        dirty_rects[dirty_len] = (SDL_Rect){ .x = image.clipx1, .y = image.clipy1, .w = image.clipx2 - image.clipx1 + 1, .h = image.clipy2 - image.clipy1 + 1 };
        dirty_len++;
    }
#endif
}

void clrvga_68k(void)
{
    // m68k: fill with 0x1010 using 2-byte writes, width = fenx2-fenx1
    u16 width = image.fenx2 - image.fenx1;
    s16 rows = image.feny2 - image.feny1;
    u16 w2 = (width >> 1) + 1;
    s16 modulo = (image.wloglarg - 1) - width;
    u16 *ptr = (u16 *)(image.wlogic + image.fenx1 + (u32)(u16)(image.feny1 - image.wlogy1) * (u32)image.wloglarg);
    for (s16 y = rows; y >= 0; y--)
    {
        for (u16 x = 0; x < w2; x++)
        {
            *ptr++ = 0x1010;
        }
        ptr = (u16 *)((u8 *)ptr + modulo);
    }

#if ALIS_SDL_VER > 1
    // Clear enhanced RGBA + depth buffers
    if (image.terrgba && image.terrgbarows) {
        memset(image.terrgba, 0, image.terrgbaw * image.terrgbah * sizeof(u32));
        if (image.depthbuf)
            memset(image.depthbuf, 0, image.terrgbaw * image.terrgbah);
    }
#endif
}

void calclan0_68k(s32 scene_addr, s32 render_context)
{
    u32 grid_shift = (u32)(u16)xread16(render_context - 0x3c0);

    // Compute signed axis deltas and their absolute values
    image.signedx = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
    s16 abs_dx = image.signedx;
    if (image.signedx < 0)
    {
        abs_dx = -image.signedx;
    }

    image.signedy = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
    s16 abs_dy = image.signedy;
    if (image.signedy < 0)
    {
        abs_dy = -image.signedy;
    }

    // Determine dominant axis and compute fixed-point stepping
    if (abs_dx < abs_dy)
    {
        // Y-dominant: step along Y, interpolate X
        xwrite16(render_context - 0x2b0, abs_dy);
        u16 minor_span = xread16(render_context - 0x36a) - xread16(render_context - 0x370);
        if ((int)((u32)minor_span << 0x10) < 0) {
            minor_span = -minor_span;
        }

        xwrite16(render_context - 0x2ac, minor_span);
        s16 step_num = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
        s16 frac = (s16)(((int)((u32)(u16)(step_num % abs_dy) << 0x10) >> 1) / (int)abs_dy);
        xwrite32(render_context - 0x2c4, (u32)(u16)(step_num / abs_dy) * 0x10000 + frac * 2);
        s32 dir_sign = 0x10000;
        if (image.signedy < 0)
        {
            dir_sign = 0xffff0000;
        }

        xwrite32(render_context - 0x2c0, dir_sign);
        s16 cross_corr = (s16)((int)(s16)(xread16(render_context - 0x37c) - xread16(render_context - 0x370)) * (int)(s16)(xread32(render_context - 0x2c4) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c0) < 0)
        {
            cross_corr = -cross_corr;
        }

        u16 strip_span = (xread16(render_context - 0x372) + cross_corr) - xread16(render_context - 0x37e);
        if ((int)((u32)strip_span << 0x10) < 0)
        {
            strip_span = -strip_span;
        }
        xwrite16(render_context - 0x24e, (s16)strip_span >> (grid_shift & 0x3f));
    }
    else
    {
        // X-dominant: step along X, interpolate Y
        xwrite16(render_context - 0x2b0, abs_dx);
        u16 minor_span = xread16(render_context - 0x36c) - xread16(render_context - 0x372);
        if ((int)((u32)minor_span << 0x10) < 0)
        {
            minor_span = -minor_span;
        }

        xwrite16(render_context - 0x2ac, minor_span);
        s16 step_num = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
        s16 frac = (s16)(((int)((u32)(u16)(step_num % abs_dx) << 0x10) >> 1) / (int)abs_dx);
        xwrite32(render_context - 0x2c0, (u32)(u16)(step_num / abs_dx) * 0x10000 + frac * 2);
        s32 dir_sign = 0x10000;
        if (image.signedx < 0)
        {
            dir_sign = 0xffff0000;
        }

        xwrite32(render_context - 0x2c4, dir_sign);
        s16 cross_corr = (s16)((int)(s16)(xread16(render_context - 0x37e) - xread16(render_context - 0x372)) * (int)(s16)(xread32(render_context - 0x2c0) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c4) < 0)
        {
            cross_corr = -cross_corr;
        }

        u16 strip_span = (xread16(render_context - 0x370) + cross_corr) - xread16(render_context - 0x37c);
        if ((int)((u32)strip_span << 0x10) < 0)
        {
            strip_span = -strip_span;
        }

        xwrite16(render_context - 0x24e, (s16)strip_span >> (grid_shift & 0x3f));
    }

    // Compute per-strip grid stepping (16.16 fixed-point)
    s16 strip_count = xread16(render_context - 0x24e);
    s16 delta_x = xread16(render_context - 0x372) - xread16(render_context - 0x37e);
    s16 frac_x = (s16)(((int)((u32)(u16)(delta_x % strip_count) << 0x10) >> 1) / (int)strip_count);
    xwrite32(render_context - 0x2bc, (int)((u32)(u16)(delta_x / strip_count) * 0x10000 + frac_x * 2) >> (grid_shift & 0x3f));
    s16 delta_y = xread16(render_context - 0x370) - xread16(render_context - 0x37c);
    s16 frac_y = (s16)(((int)((u32)(u16)(delta_y % strip_count) << 0x10) >> 1) / (int)strip_count);
    xwrite32(render_context - 0x2b8, (int)((u32)(u16)(delta_y / strip_count) * 0x10000 + frac_y * 2) >> (grid_shift & 0x3f));

    // Direction signs for grid stepping
    s32 dir_sign_x = 0x10000;
    if (xread32(render_context - 700) < 0)
    {
        dir_sign_x = 0xffff0000;
    }

    xwrite32(render_context - 0x2a0, dir_sign_x);

    s32 dir_sign_y = 0x10000;
    if (xread32(render_context - 0x2b8) < 0)
    {
        dir_sign_y = 0xffff0000;
    }

    xwrite32(render_context - 0x29c, dir_sign_y);

    // Dimension scaling delta per strip
    xwrite32(render_context - 0x2b4, ((int)(s16)(((int)(s16)(xread16(render_context - 0x2ac) - xread16(render_context - 0x2b0)) << 8) / (int)xread16(render_context - 0x24e)) << 8) >> (grid_shift & 0x3f));

    // Fog/palette interpolation setup
    xwrite32(render_context - 0x252, 0);
    xwrite32(render_context - 0x256, (u16)xread16(scene_addr + 0x9e) << 0x10);

    if (xread16(scene_addr + 0x98) != 0)
    {
        xwrite32(render_context - 0x256, 0x60000);
        u16 fog_level = xread16(scene_addr + 0x9e);
        if ((s16)fog_level < 7)
        {
            xwrite32(render_context - 0x256, 0);
        }

        if (-1 < xread16(scene_addr + 0x98))
        {
            xwrite32(render_context - 0x256, (u32)(u16)xread16(scene_addr + 0xa0) * 0x10000 - 1);
        }

        xwrite32(render_context - 0x252, (int)(s16)(((int)((u32)fog_level * 0x10000 - xread32(render_context - 0x256)) >> 8) / (int)(s16)(xread16(render_context - 0x24e) + 1)) << 8);
    }

    // Per-strip interpolation deltas and initial values
    strip_count = xread16(render_context - 0x24e);

    xwrite32(render_context - 0x2e4, (int)(s16)((xread32(render_context - 0x318) - xread32(render_context - 0x330)) / (int)strip_count));
    xwrite32(render_context - 0x2dc, (int)(s16)(((xread32(render_context - 0x2f0) - xread32(render_context - 0x2fc)) * 0x100) / (int)strip_count) << 8);
    xwrite32(render_context - 0x2d4, (int)(s16)((xread32(render_context - 0x310) - xread32(render_context - 0x328)) / (int)strip_count));
    xwrite32(render_context - 0x2cc, (int)(s16)(((xread32(render_context - 0x2ec) - xread32(render_context - 0x2f8)) * 0x100) / (int)strip_count) << 8);
    xwrite32(render_context - 0x2e8, xread32(render_context - 0x330));
    xwrite32(render_context - 0x2e0, xread32(render_context - 0x32c) << 0x10);
    xwrite32(render_context - 0x2d8, xread32(render_context - 0x328));
    xwrite32(render_context - 0x2d0, xread32(render_context - 0x2f8));
}

static inline u32 rot8(u32 x) { return (x << 8) | (x >> 24); }

// =========================================================================
// Flat bars: a transliteration of the original asm (tools/asm_lift/blob_a_bars.inc.S,
// bartra/bartrag/bartrar/bartram/bartramin/bartrab*), which is the reference. Register names
// are kept where they carry the logic: d4/d1 (or d2) are the two dither phases, swapped per row.
// =========================================================================

static inline void put_l(u8 *p, u32 v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static inline void put_w(u8 *p, u32 v) { p[0] = v >> 8; p[1] = v; }

// The unrolled line bodies: count index i writes i+1 bytes as i>>2 longs of d4 (one more when
// i&3 == 3), then a d4 word (i&3 == 1, 2) and a byte of `alt` (i&3 == 0, 2). Negative i lands
// in the table guard and writes nothing.
static u8 *line_fwd(u8 *p, s16 i, u32 d4, u32 alt)
{
    if (i < 0)
        return p;
    s16 j = i & 3;
    for (s16 n = (i >> 2) + (j == 3); n > 0; n--, p += 4)
        put_l(p, d4);
    if (j == 1 || j == 2) { put_w(p, d4); p += 2; }
    if (j == 0 || j == 2) *p++ = (u8)alt;
    return p;
}

// tg bodies: the same, written right to left with pre-decrement.
static u8 *line_bwd(u8 *p, s16 i, u32 d4, u32 alt)
{
    if (i < 0)
        return p;
    s16 j = i & 3;
    for (s16 n = (i >> 2) + (j == 3); n > 0; n--) { p -= 4; put_l(p, d4); }
    if (j == 1 || j == 2) { p -= 2; put_w(p, d4); }
    if (j == 0 || j == 2) *--p = (u8)alt;
    return p;
}

static inline void swap_l(u32 *a, u32 *b) { u32 t = *a; *a = *b; *b = t; }
static inline int odd_addr(const u8 *p) { return (int)((p - alis.mem) & 1); }
static inline u8 *pixrow(s16 y) { return alis.mem + xread32(image.atlpix + (s16)((y - image.wlogy1) * 4)); }

// bartra/bartrab head: dark level from the cell, then the shade lookup -> 2-periodic long.
static u32 bar_shade(s32 cell, s32 rc, s16 index)
{
    s16 dark = (s16)((((xread16(xread16(rc - 0x3c4) + cell) & 0xc0) + ((xread16(cell + 2) >> 8) & 0xc0)
                       + ((xread16(cell) >> 8) & 0xc0) * 2) * -2) + image.vdarkw);
    if (dark < 0)
        dark = 0;
    u16 c;
    if (alis.platform.bpp == 4)
        c = xread16(alis.ptrdark + (s16)((dark & 0xff00) | (u8)(xread8(rc + 8 + index) * 2)));
    else
        c = (u16)(xread8(alis.ptrdark + (s16)((dark & 0xff00) | xread8(rc + 8 + index))) << 8)
          | xread8(alis.ptrdark + (s16)((dark & 0xff00) | xread8(rc + 9 + index)));
    return ((u32)c << 16) | c;
}

// bartra32/bartrab head: slope step from atalias.
static u16 alias_step(u16 d2, u16 larg)
{
    if (d2 <= 0x40 && larg <= 0x3f)
        return xread16(image.atalias + (s16)((((d2 - 1) << 6) + larg) * 2));
    d2 >>= 2;
    if (d2 > 0x40)
        d2 = 0x40;
    return (u16)((s16)xread16(image.atalias + (s16)((((d2 - 1) << 7) + larg) & 0xfffe)) >> 1);
}

static inline u32 dither_phase(u32 d4, s16 y, s32 rc)
{
    if (y & 1) d4 = rot8(d4);
    if (xread16(rc - 0x24e) & 1) d4 = rot8(d4);
    return d4;
}

// 16.16 start of a slope/taper: (y - top) << 8, times the step (mulu.w on the low word).
static inline u32 slope_start(s16 d0, s16 d1, u16 d2)
{
    return d0 > d1 ? (u32)(u16)((u16)(d0 - d1) << 8) * d2 : 0;
}

static inline u32 add_hi(u32 d3, s16 v) { return ((u32)(u16)((d3 >> 16) + v) << 16) | (d3 & 0xffff); }

static void bartrab_flat(s32 rc, s16 d0, s16 d1, s16 d5, s16 d6, u32 d4);

// bartramin (+ bartrams): d5+1 full rows of d6+1 bytes from a0; then the bottom taper if pending.
static void bartramin_flat(s32 rc, u8 *a0, s16 d5, s16 d6, u32 d4)
{
    RRQ_PATH('m');
    u32 d2 = rot8(d4);
    s16 mod = image.wloglarg - d6 - 1;
    if (odd_addr(a0)) {
        *a0++ = (u8)d4;
        for (;;) {
            a0 = line_fwd(a0, d6 - 1, d4, d2) + mod;
            swap_l(&d2, &d4);
            if (--d5 < 0) break;
            *a0++ = (u8)d4;
        }
    } else {
        for (;;) {
            a0 = line_fwd(a0, d6, d4, d2) + mod;
            swap_l(&d2, &d4);
            if (--d5 < 0) break;
        }
    }
    if (image.vbarbot)
        bartrab_flat(rc, bothigh, bothigh, image.vbarbot, d6, d4);   // bartrams -> bartrabin
}

// bartram: d5 rows from the bar top.
static void bartram_flat(s32 rc, s16 d0, s16 d5, s16 d6, u32 d4)
{
    if (--d5 < 0)
        return;
    d4 = dither_phase(d4, d0, rc);
    bartramin_flat(rc, pixrow(d0) + image.precx, d5, d6, d4);
}

// bartrabin: bottom taper, d5 rows from d0 narrowing by the botalt step. d1 = taper top.
static void bartrab_flat(s32 rc, s16 d0, s16 d1, s16 d5, s16 d6, u32 d4)
{
    image.vbarbot = 0;
    if (--d5 < 0)
        return;
    u16 d2 = alias_step((u16)(botalt - bothigh), image.vbarlarg);
    u32 d3 = (u32)-(s32)slope_start(d0, d1, d2);
    d3 = add_hi(d3, image.vbarlarg);
    s32 step = -((s32)(s16)d2 << 8);
    u32 d1r;
    s16 wlog = image.wloglarg;

    if (botalt != precboti) {
        // bartrabr: anchored left
        RRQ_PATH('B');
        if ((s16)(image.vbarlarg - d6) > 0)
            d3 = add_hi(d3, image.vbarx - image.precx);
        d4 = dither_phase(d4, d0, rc);
        u8 *a2 = pixrow(d0) + image.precx, *a0 = a2;
        d1r = rot8(d4);
        int odd = odd_addr(a0);
        for (;;) {
            s16 d7 = (s16)(d3 >> 16);
            if (d7 > d6) d7 = d6;
            if (odd) { *a0++ = (u8)d4; line_fwd(a0, d7 - 1, d4, d1r); }
            else line_fwd(a0, d7, d4, d1r);
            swap_l(&d1r, &d4);
            d3 += step;
            if ((s32)d3 < 0) break;
            a2 += wlog; a0 = a2;
            if (--d5 == -1) break;
        }
    } else {
        // bartrabg: anchored right, written right to left
        RRQ_PATH('b');
        if ((s16)(image.vbarlarg - d6) > 0)
            d3 = add_hi(d3, image.precx + d6 - image.vbarx - image.vbarlarg);
        d4 = dither_phase(d4, d0, rc);
        u8 *a2 = pixrow(d0) + image.precx + 1 + d6, *a0 = a2;
        d1r = rot8(d4);
        int odd = odd_addr(a0);
        for (;;) {
            s16 d7 = (s16)(d3 >> 16);
            if (odd) { *--a0 = (u8)d4; line_bwd(a0, d7 - 1, d4, d1r); }   // no clamp on this path
            else { if (d7 > d6) d7 = d6; line_bwd(a0, d7, d4, d1r); }
            swap_l(&d1r, &d4);
            d3 += step;
            if ((s32)d3 < 0) break;
            a2 += wlog; a0 = a2;
            if (--d5 == -1) break;
        }
    }
}

// bartrag/bartrar: top slope, d5 rows from d0 widening by the step. Returns the next row start
// (left edge) and the phase for it.
static u8 *bartra_slope(s32 rc, s16 d0, s16 d1, s16 d5, s16 d6, u16 d2, u32 *d4p)
{
    u32 d4 = *d4p, d3 = slope_start(d0, d1, d2);
    s32 step = (s32)(s16)d2 << 8;
    s16 wlog = image.wloglarg;
    int rtl = xread16(rc - 0x246) != xread16(rc - 0x25c);
    RRQ_PATH(rtl ? 'R' : 'L');
    if ((s16)(image.vbarlarg - d6) > 0)
        d3 = add_hi(d3, rtl ? image.precx + d6 - image.vbarx - image.vbarlarg : image.vbarx - image.precx);
    d4 = dither_phase(d4, d0, rc);
    u8 *a2 = pixrow(d0) + image.precx + (rtl ? 1 + d6 : 0), *a0 = a2;
    u32 d1r = rot8(d4);
    int odd = odd_addr(a0);
    d5++;
    s16 d7 = (s16)(d3 >> 16);
    for (;;) {
        int over = odd ? (u16)d7 > (u16)d6 : d7 > d6;   // dbhi on odd rows, dbgt on even
        if (!over) {
            if (--d5 == -1) break;
            if (odd) {
                if (rtl) { *--a0 = (u8)d4; line_bwd(a0, d7 - 1, d4, d1r); }
                else { *a0++ = (u8)d4; line_fwd(a0, d7 - 1, d4, d1r); }
            } else {
                if (rtl) line_bwd(a0, d7, d4, d1r);
                else line_fwd(a0, d7, d4, d1r);
            }
        } else {
            if (d5 < 0) break;
            if (odd && d7 < 0) {
                if (--d5 == -1) break;      // bartragi11/bartrari11: skip the row
            } else {
                d7 = d6;
                continue;
            }
        }
        swap_l(&d1r, &d4);
        d3 += step;
        a2 += wlog; a0 = a2;
        d7 = (s16)(d3 >> 16);
    }
    *d4p = d4;
    return rtl ? a0 - 1 - d6 : a0;
}

// bartra after the shade lookup: ground or overhang bar. d0 = clipped top, bary = unclipped top.
static void bartra_shaded(s32 rc, s16 d0, s16 d5, s16 d6, s16 bary, u32 d4)
{
    RRQ_PATH('F');
    s16 r246 = xread16(rc - 0x246);

    s16 d7 = r246 - d0;
    if (d7 <= 1) { bartram_flat(rc, d0, d5, d6, d4); return; }
    d5 -= d7;
    if (d5 < 0) d7 += d5;
    if (d7 <= 0) { bartram_flat(rc, d0, d5, d6, d4); return; }
    image.vbarmid = d5;
    d5 = d7 - 1;
    d7 = r246 - bary;
    if (d7 <= 1) { bartram_flat(rc, d0, image.vbarmid, d6, d4); return; }

    u8 *a0 = bartra_slope(rc, d0, bary, d5, d6, alias_step((u16)d7, image.vbarlarg), &d4);
    d5 = image.vbarmid - 1;
    if (d5 < 0)
        return;
    bartramin_flat(rc, a0, d5, d6, d4);
}

static void bartra_68k(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight, s16 bary)
{
    bartra_shaded(render_context, (s16)drawy, barheight, barwidth - 1, bary,
                  bar_shade(terrain_cell, render_context, index));
}

// tbarland bottom-only entry (bartrab): the bar lies wholly below bothigh.
static void bartrab_68k(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight)
{
    bartrab_flat(render_context, (s16)drawy, bothigh, barheight, barwidth - 1,
                 bar_shade(terrain_cell, render_context, index));
}


// =========================================================================
// Falcon CD textured bottom narrowing renderer
// Ghidra barland CD lines 23774-23901, tbarland CD lines 24249-24389
// =========================================================================
static void bartrab_textured_68k(s32 render_context, s16 index, s16 max_cols,
    s16 bot_lines, u32 dark, u32 persp, u32 v_step, u32 h_step_per_line,
    u32 V, u16 width_mask, s16 height_mask, s32 tex_base)
{
    RRQ_PATH('t');
    // Atalias lookup for bottom slope
    u16 bot_dist = botalt - bothigh;
    u16 slope_step;
    if ((bot_dist < 0x41) && (image.vbarlarg < 0x40))
    {
        slope_step = (u16)xread16(image.atalias + (s16)((image.vbarlarg + (bot_dist - 1) * 0x40) * 2));
    }
    else
    {
        bot_dist >>= 2;
        if (0x40 < bot_dist)
            bot_dist = 0x40;
        slope_step = xread16(image.atalias + (s16)(image.vbarlarg + (bot_dist - 1) * 0x80 & 0xfffe)) >> 1;
    }

    u16 clip_cols = (u16)(image.vbarlarg - max_cols);

    if (botalt == precboti)
    {
        // RTL bottom narrowing (Ghidra lines 23789-23844)
        s32 botf = 0;
        if (bothigh < (s16)bothigh) // NOTE: Ghidra uses sVar7 = drawy for bottom-only, bothigh for after-main
            botf = 0; // simplified: bothigh < bothigh is always false
        // The initial offset is always 0 for the standard after-main-body path

        u16 col_start = image.vbarlarg;
        if (clip_cols != 0 && (s16)max_cols <= (s16)image.vbarlarg)
            col_start = ((max_cols + image.precx + col_start) - image.vbarx) - image.vbarlarg;

        u32 pix_frac = (u32)col_start << 0x10;
        u32 fb = (s32)(s16)max_cols + (s32)image.precx + xread32(image.atlpix + (s16)((bothigh - image.wlogy1) * 4)) + 1;
        u32 row_fb = fb;

        do
        {
            u16 col_count = pix_frac >> 0x10;
            if ((s16)max_cols < (s16)col_count)
                col_count = max_cols;

            u32 H_init = concat22((s16)(h_step_per_line >> 16), width_mask & (u16)h_step_per_line);
            u16 v_lo = height_mask & (u16)V;
            u32 Vsaved = concat22((s16)(V >> 16), v_lo);

            s16 cnt = (s16)col_count;
            if (cnt >= 0)
            {
                do {
                    u16 uv = v_lo | (u16)H_init;
                    u8 texel = xread8(tex_base + (s16)uv);
                    u32 H_recov = concat22((s16)(H_init >> 16), v_lo ^ uv);
                    fb--;
                    xwrite8(fb, xread8(alis.ptrdark + (s16)((dark & 0xFF00) | texel)));
                    u32 H_sum = persp + H_recov;
                    H_init = concat22((s16)(H_sum >> 16),
                                      width_mask & ((s16)H_sum + (u16)carry4(persp, H_recov)));
                    cnt--;
                } while (cnt >= 0);
            }

            V = v_step + Vsaved;
            if (carry4(v_step, Vsaved))
                V = concat22((s16)(V >> 16), width_mask + (s16)V + 1);

            pix_frac += (s16)slope_step * -0x100;
            fb = row_fb + image.wloglarg;
            row_fb = fb;
        }
        while ((s32)pix_frac >= 0 && (--bot_lines) != -1);
    }
    else
    {
        // LTR bottom narrowing (Ghidra lines 23846-23900)
        u16 col_start = image.vbarlarg;
        if (clip_cols != 0 && (s16)max_cols <= (s16)image.vbarlarg)
            col_start = (image.vbarx + col_start) - image.precx;

        u32 pix_frac = (u32)col_start << 0x10;
        u32 fb = (s32)image.precx + xread32(image.atlpix + (s16)((bothigh - image.wlogy1) * 4));
        u32 row_fb = fb;

        do
        {
            u16 col_count = pix_frac >> 0x10;
            if ((s16)max_cols < (s16)col_count)
                col_count = max_cols;

            u32 H_init = concat22((s16)(h_step_per_line >> 16), width_mask & (u16)h_step_per_line);
            u16 v_lo = height_mask & (u16)V;
            u32 Vsaved = concat22((s16)(V >> 16), v_lo);

            s16 cnt = (s16)col_count;
            if (cnt >= 0)
            {
                do {
                    u16 uv = v_lo | (u16)H_init;
                    u8 texel = xread8(tex_base + (s16)uv);
                    u32 H_recov = concat22((s16)(H_init >> 16), v_lo ^ uv);
                    xwrite8(fb, xread8(alis.ptrdark + (s16)((dark & 0xFF00) | texel)));
                    u32 H_sum = persp + H_recov;
                    H_init = concat22((s16)(H_sum >> 16),
                                      width_mask & ((s16)H_sum + (u16)carry4(persp, H_recov)));
                    fb++;
                    cnt--;
                } while (cnt >= 0);
            }

            V = v_step + Vsaved;
            if (carry4(v_step, Vsaved))
                V = concat22((s16)(V >> 16), width_mask + (s16)V + 1);

            pix_frac += (s16)slope_step * -0x100;
            fb = row_fb + image.wloglarg;
            row_fb = fb;
        }
        while ((s32)pix_frac >= 0 && (--bot_lines) != -1);
    }
}

// =========================================================================
// Falcon CD textured bar renderer — slope + main body + bottom dispatch
// Ghidra barland CD lines 23575-23907
// =========================================================================
static void bartra_textured_68k(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight, s16 bary)
{
    RRQ_PATH('T');
    s16 max_cols = barwidth - 1;
    s16 row_offset = (s16)drawy - bary;

    // Darkness: same formula as flat bartra
    u16 dark_val = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0)
                 + ((xread16(terrain_cell + 2) >> 8) & 0xc0)
                 + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
    u32 dark = (u32)dark_val;
    if ((s32)(dark << 0x10) < 0)
        dark = 0;

    // Resolve texture from type entry (need width_mask before shift derivation)
    u32 tex_table_ptr = xread32(render_context + index + 4);
    s32 tex_offset = (s32)xread32(tex_table_ptr);
    u16 width_mask = xread16(tex_table_ptr + tex_offset + 2);

    // Shift: read from type entry; if 0, derive from width_mask (Falcon CD
    // game scripts don't initialise this field — floppy never needed it)
    u8 shift = xread8(render_context + index) & 0x3f;
    if (shift == 0 && width_mask != 0) {
        u16 w = width_mask + 1;
        while (w > 1) { shift++; w >>= 1; }
    }

    u32 persp = image.tex_persp_step;
    u32 persp_unrolled = (persp << 8) | (persp >> 24);

    // V step per scanline
    u32 v_step = concat22((s16)(persp >> 16), (s16)persp << shift);

    // H step per scanline (accounts for left-clipped columns)
    u16 clip_cols = (u16)(image.vbarlarg - max_cols);
    u32 h_step_raw = persp_unrolled * (u32)clip_cols * 0x10000;
    u32 h_step_per_line = (h_step_raw << 8) | (h_step_raw >> 24);

    // V initial from row offset
    u32 v_init_raw = persp_unrolled * (u32)(u16)row_offset * 0x10000;
    u32 V = concat22((s16)((v_init_raw << 8) >> 16),
                     (u16)(u8)(v_init_raw >> 24) << shift);

    // Height mask and texture base
    s16 type_v_base = xread16(render_context + index + 2);
    s16 hmask_raw = xread16(tex_table_ptr + tex_offset + 4) - type_v_base;
    // Snap to nearest power-of-2 minus 1 (required for AND masking).
    // Original Falcon CD resources may have non-power-of-2 heights (e.g. 36);
    // DOS init code snaps these in opcodes.c but the 68k path doesn't.
    if (hmask_raw > 0 && (hmask_raw & (hmask_raw + 1)) != 0) {
        u16 h = (u16)hmask_raw;
        h |= h >> 1; h |= h >> 2; h |= h >> 4; h |= h >> 8;
        hmask_raw = (s16)(h >> 1);
    }
    s16 height_mask = hmask_raw << shift;
    // Image header: +0 format, +2 width_mask, +4 height, +6 extra, +8 pixel data
    // Original Falcon CD asm uses +6 (native resource format), but loaded resources
    // use the platform image format which has an 8-byte header (format 0x1C/0x1E)
    s32 tex_base = tex_table_ptr + ((s32)type_v_base << shift) + tex_offset + 8;

#if defined(ALIS_PROFILE_TEX)
    { extern void texprof_add(u32, s32, s32);
      texprof_add((u32)(tex_table_ptr + tex_offset), (s32)width_mask + 1,
                  (s32)xread16(tex_table_ptr + tex_offset + 4)); }
#endif

    // Framebuffer pointer — flows through slope into main body
    u32 fb = (s32)image.precx + xread32(image.atlpix + (s16)(((s16)drawy - image.wlogy1) * 4));

    // Slope section
    s16 top_lines = xread16(render_context - 0x246) - (s16)drawy;
    s16 mid_height = barheight;

    if (top_lines != 0 && (s16)drawy < xread16(render_context - 0x246)
        && (s16)(xread16(render_context - 0x246) - bary) >= 2)
    {
        mid_height = barheight - top_lines;
        if (barheight < top_lines)
            top_lines = mid_height + top_lines; // clamp to barheight

        if (top_lines >= 1)
        {
            u16 tex_step_dist = xread16(render_context - 0x246) - bary;
            image.vbarmid = mid_height;

            if ((s16)tex_step_dist >= 2)
            {
                u16 slope_step;
                if ((tex_step_dist < 0x41) && (image.vbarlarg < 0x40))
                    slope_step = (u16)xread16(image.atalias + (s16)((image.vbarlarg + (tex_step_dist - 1) * 0x40) * 2));
                else
                {
                    tex_step_dist >>= 2;
                    if (0x40 < tex_step_dist)
                        tex_step_dist = 0x40;
                    slope_step = xread16(image.atalias + (s16)(image.vbarlarg + (tex_step_dist - 1) * 0x80 & 0xfffe)) >> 1;
                }

                // LTR slope (rc[-0x246] == rc[-0x25c])
                if (xread16(render_context - 0x246) == xread16(render_context - 0x25c))
                {
                    u32 tex_frac = 0;
                    if (bary < (s16)drawy)
                        tex_frac = ((u16)((s16)drawy - bary) & 0xff) * 0x100 * (u32)slope_step;
                    if (clip_cols != 0 && (s16)max_cols <= (s16)image.vbarlarg)
                        tex_frac = (u32)(u16)((image.vbarx + (s16)(tex_frac >> 0x10)) - image.precx) << 0x10 | (tex_frac & 0xffff);

                    u16 col_count = tex_frac >> 0x10;
                    u32 row_start = fb;

                    while (true)
                    {
                        while ((s16)col_count <= (s16)max_cols && (--top_lines) != -1)
                        {
                            u32 H = concat22((s16)(h_step_per_line >> 16), width_mask & (u16)h_step_per_line);
                            u32 Vm = concat22((s16)(V >> 16), height_mask & (u16)V);
                            s16 cnt = (s16)col_count;
                            while (cnt >= 0)
                            {
                                u16 uv = (u16)H | (u16)Vm;
                                u8 texel = xread8(tex_base + (s16)uv);
                                Vm = concat22((s16)(Vm >> 16), (u16)H ^ uv);
                                xwrite8(fb, xread8(alis.ptrdark + (s16)((dark & 0xFF00) | texel)));
                                u32 old_H = H;
                                H = concat22((s16)((persp + old_H) >> 16),
                                             width_mask & ((s16)(persp + old_H) + (u16)carry4(persp, old_H)));
                                fb++;
                                cnt--;
                            }
                            V = v_step + Vm;
                            if (carry4(v_step, Vm))
                                V = concat22((s16)(V >> 16), width_mask + (s16)V + 1);
                            tex_frac += (s16)slope_step * 0x100;
                            fb = row_start + image.wloglarg;
                            col_count = tex_frac >> 0x10;
                            row_start = fb;
                        }
                        if (top_lines < 0) break;
                        col_count = max_cols;
                    }
                }
                // RTL slope
                else
                {
                    u32 tex_frac = 0;
                    if (bary < (s16)drawy)
                        tex_frac = ((u16)((s16)drawy - bary) & 0xff) * 0x100 * (u32)slope_step;
                    if (clip_cols != 0 && (s16)max_cols <= (s16)image.vbarlarg)
                        tex_frac = (u32)(u16)(((max_cols + image.precx + (s16)(tex_frac >> 0x10)) - image.vbarx) - image.vbarlarg) << 0x10 | (tex_frac & 0xffff);

                    fb = (s32)(s16)max_cols + (s32)image.precx + xread32(image.atlpix + (s16)(((s16)drawy - image.wlogy1) * 4)) + 1;
                    u16 col_count = tex_frac >> 0x10;
                    u32 row_start = fb;

                    while (true)
                    {
                        while ((s16)col_count <= (s16)max_cols && (--top_lines) != -1)
                        {
                            u32 H = 0; // RTL: H starts at 0 each row
                            u32 Vm = concat22((s16)(V >> 16), height_mask & (u16)V);
                            s16 cnt = (s16)col_count;
                            while (cnt >= 0)
                            {
                                u16 uv = (u16)H | (u16)Vm;
                                u8 texel = xread8(tex_base + (s16)uv);
                                Vm = concat22((s16)(Vm >> 16), (u16)H ^ uv);
                                fb--;
                                xwrite8(fb, xread8(alis.ptrdark + (s16)((dark & 0xFF00) | texel)));
                                u32 old_H = H;
                                H = concat22((s16)((persp + old_H) >> 16),
                                             width_mask & ((s16)(persp + old_H) + (u16)carry4(persp, old_H)));
                                cnt--;
                            }
                            V = v_step + Vm;
                            if (carry4(v_step, Vm))
                                V = concat22((s16)(V >> 16), width_mask + (s16)V + 1);
                            tex_frac += (s16)slope_step * 0x100;
                            fb = row_start + image.wloglarg;
                            col_count = tex_frac >> 0x10;
                            row_start = fb;
                        }
                        if (top_lines < 0) break;
                        col_count = max_cols;
                    }
                    fb += (-1 - (s16)max_cols); // adjust back to left edge for main body
                }

                mid_height = image.vbarmid;
            }
        }
    }

    // Main body: full-width LTR textured scanlines
    s16 mid_lines = mid_height - 1;
    if (mid_lines >= 0)
    {
        s16 stride_advance = image.wloglarg - max_cols;
        do
        {
            u32 H = concat22((s16)(h_step_per_line >> 16), width_mask & (u16)h_step_per_line);
            u32 Vm = concat22((s16)(V >> 16), height_mask & (u16)V);
            s16 cnt = max_cols;
            while (cnt >= 0)
            {
                u16 uv = (u16)H | (u16)Vm;
                u8 texel = xread8(tex_base + (s16)uv);
                Vm = concat22((s16)(Vm >> 16), (u16)H ^ uv);
                xwrite8(fb, xread8(alis.ptrdark + (s16)((dark & 0xFF00) | texel)));
                fb++;
                u32 old_H = H;
                H = concat22((s16)((persp + old_H) >> 16),
                             width_mask & ((s16)(persp + old_H) + (u16)carry4(persp, old_H)));
                cnt--;
            }
            V = v_step + Vm;
            if (carry4(v_step, Vm))
                V = concat22((s16)(V >> 16), width_mask + (s16)V + 1);
            fb += (s16)(stride_advance - 1);
            mid_lines--;
        }
        while (mid_lines != -1);
    }

    // Bottom section: textured narrowing (from tbarland with vbarbot > 0)
    s16 saved_vbarbot = image.vbarbot;
    if (saved_vbarbot != 0)
    {
        image.vbarbot = 0;
        s16 bot_lines = saved_vbarbot - 1;
        if (bot_lines >= 0)
        {
            bartrab_textured_68k(render_context, index, max_cols, bot_lines + 1,
                dark, persp, v_step, h_step_per_line, V, width_mask, height_mask, tex_base);
        }
    }
}

static bool clip_bar_y(s16 bary, s16 *barheight, u16 *drawy)
{
    s16 barclipy = (bary - image.landclipy1) - image.landcliph;
    *drawy = bary;
    if (((u16)image.landcliph <= (u16)(bary - image.landclipy1) && barclipy != 0) || ((s16)(-*barheight - barclipy) < 0))
    {
        if (image.clipy2 < bary)
            return false;

        s16 barbot = bary + *barheight - 1;
        if (barbot < (s16)image.landclipy1)
            return false;

        if (bary < (s16)image.landclipy1)
        {
            *barheight = bary + (*barheight - image.landclipy1);
            *drawy = image.landclipy1;
        }

        if (image.clipy2 < barbot)
        {
            *barheight = image.clipy2 + (*barheight - barbot);
        }
    }
    return true;
}

static void hittest_bar(s32 terrain_cell, s32 render_context, u16 drawy, s16 barheight, s16 barwidth, s32 packed_coord, s16 step_x, s16 step_y)
{
    if ((s16)drawy <= image.ytstpix && image.ytstpix < (s16)(barheight + drawy) && image.precx <= image.xtstpix && image.xtstpix < (s16)(barwidth + image.precx))
    {
        u16 shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
        image.ntstpix = (u16)(xread16(terrain_cell) >> 8);
        image.cztstpix = (u16)(xread16(terrain_cell) & 0xff);
        image.cxtstpix = (u16)(((s16)((u32)packed_coord >> 0x10) * 2 - step_x) << shift) >> 1;
        image.cytstpix = (u16)(((s16)packed_coord * 2 - step_y) << shift) >> 1;
        image.etstpix = 0xfffe;
        image.dtstpix = xread32hi16(render_context - 0x2e0);
    }
    barlands(drawy, barheight, barwidth);
}

static __attribute__((noinline)) void barland_68k(s32 terrain_cell, s32 render_context, s16 step_x, s16 step_y, s16 bary, s16 barheight, s16 index, s16 barx, s32 packed_coord, s32 d6)
{
    s16 prev_screen_x = image.precx;

    // Clip bar vertically
    u16 drawy;
    if (!clip_bar_y(bary, &barheight, &drawy))
        return;

    if (image.clipx1 <= barx)
    {
        // Compute horizontal span from previous cursor to current bar edge
        image.vbarlarg = (barx - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }

        // Clip bar width against right boundary
        s16 barwidth;
        if (image.vbarclipx2 < barx)
        {
            barwidth = -(image.precx - image.vbarclipx2);
            if (barwidth == 0 || 0 < (s16)(image.precx - image.vbarclipx2))
            {
                barlands(drawy, barheight, barwidth);
                return;
            }
        }
        else
        {
            barwidth = -(image.precx - barx);
            if (barwidth == 0 || 0 < (s16)(image.precx - barx))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }

        // Render or hit-test the bar
        if (-1 < xread16(render_context - 0x24e))
        {
            OD(if (image.ftstpix == 0) od_bar(image.precx, (s16)drawy - image.wlogy1, barwidth, barheight);)
#if ALIS_SDL_VER > 1
            // Stamp per-pixel depth for the bar's screen region
            if (image.depthrows) {
                s16 ry = (s16)drawy - image.wlogy1;
                s16 rx = image.precx;
                for (s16 r = 0; r < barheight; r++) {
                    s16 y = ry + r;
                    if (y < 0) continue;
                    if (y >= image.terrgbah) break;
                    u8 *drow = image.depthrows[y];
                    for (s16 c = 0; c < barwidth && (rx + c) < image.terrgbaw; c++)
                        if ((rx + c) >= 0) drow[rx + c] = image.stripdepth;
                }
            }
#endif
            if (image.ftstpix == 0)
            {
                if ((s8)xread8(render_context - 0x400) == 0x0B)
                    bartra_textured_68k(terrain_cell, render_context, drawy, index, barwidth, barheight, bary);
                else
                    bartra_68k(terrain_cell, render_context, drawy, index, barwidth, barheight, bary);
            }
            else
            {
                hittest_bar(terrain_cell, render_context, drawy, barheight, barwidth, packed_coord, step_x, step_y);
            }
        }
    }
}


static __attribute__((noinline)) void tbarland_68k(s32 terrain_cell, s32 render_context, s16 step_x, s32 step_y, u16 bary, s16 barheight, s16 index, s32 screen_x, s32 packed_coord, s32 d6)
{
    s16 prev_screen_x = image.precx;

    // Clip bar vertically
    u16 drawy;
    if (!clip_bar_y((s16)bary, &barheight, &drawy))
        return;

    s16 barx = (s16)screen_x;
    if (image.clipx1 <= barx)
    {
        // Compute horizontal span from previous cursor to current bar edge
        image.vbarlarg = (barx - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }

        // Clip bar width against right boundary
        s16 barwidth;
        if (image.vbarclipx2 < barx)
        {
            barwidth = -(image.precx - image.vbarclipx2);
            if (barwidth == 0 || 0 < (s16)(image.precx - image.vbarclipx2))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }
        else
        {
            barwidth = -(image.precx - barx);
            if (barwidth == 0 || 0 < (s16)(image.precx - barx))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }

        if (-1 < xread16(render_context - 0x24e))
        {
            // Clamp bottom high-water mark
            if ((s16)bothigh < xread16(render_context - 0x246))   // signed, as the asm cmp.w
            {
                bothigh = xread16(render_context - 0x246);
            }

            image.vbarbot = 0;
            if (fbottom != 0)
            {
                // Bar entirely below bottom clip — bottom-only rendering
                if ((s16)bothigh <= (s16)drawy)
                {
                    if ((s8)xread8(render_context - 0x400) == 0x0B)
                    {
                        // Textured bottom-only path (Ghidra tbarland CD lines 24014-24027)
                        barwidth--;
                        u16 dark_val = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + ((xread16(terrain_cell + 2) >> 8) & 0xc0) + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
                        u32 dark = (u32)dark_val;
                        if ((s32)(dark << 0x10) < 0)
                            dark = 0;
                        u32 persp = image.tex_persp_step;
                        u32 persp_unrolled = (persp << 8) | (persp >> 24);
                        u32 ttp = xread32(render_context + index + 4);
                        s32 toff = (s32)xread32(ttp);
                        u16 wmask = xread16(ttp + toff + 2);
                        u8 shift = xread8(render_context + index) & 0x3f;
                        if (shift == 0 && wmask != 0) {
                            u16 w = wmask + 1;
                            while (w > 1) { shift++; w >>= 1; }
                        }
                        u32 v_step = concat22((s16)(persp >> 16), (s16)persp << shift);
                        u16 clip_cols = (u16)(image.vbarlarg - barwidth);
                        u32 h_raw = persp_unrolled * (u32)clip_cols * 0x10000;
                        u32 h_step = (h_raw << 8) | (h_raw >> 24);
                        s16 row_offset = (s16)drawy - (s16)bary;
                        u32 v_raw = persp_unrolled * (u32)(u16)row_offset * 0x10000;
                        u32 V = concat22((s16)((v_raw << 8) >> 16), (u16)(u8)(v_raw >> 24) << shift);
                        s16 vbase = xread16(render_context + index + 2);
                        s16 hmask_raw = xread16(ttp + toff + 4) - vbase;
                        if (hmask_raw > 0 && (hmask_raw & (hmask_raw + 1)) != 0) {
                            u16 h = (u16)hmask_raw;
                            h |= h >> 1; h |= h >> 2; h |= h >> 4; h |= h >> 8;
                            hmask_raw = (s16)(h >> 1);
                        }
                        s16 hmask = hmask_raw << shift;
                        s32 tbase = ttp + ((s32)vbase << shift) + toff + 8;
                        bartrab_textured_68k(render_context, index, barwidth, barheight, dark, persp, v_step, h_step, V, wmask, hmask, tbase);
                    }
                    else
                    {
                        bartrab_68k(terrain_cell, render_context, drawy, index, barwidth, barheight);
                    }
                    return;
                }

                // Clip bar height against bottom boundary
                s16 clip_calc = (barheight + drawy) - bothigh;
                if (clip_calc != 0 && (s16)bothigh <= (s16)(barheight + drawy))
                {
                    image.vbarbot = clip_calc;
                }
            }

            OD(if (image.ftstpix == 0) od_bar(image.precx, (s16)drawy - image.wlogy1, barwidth, barheight);)
#if ALIS_SDL_VER > 1
            // Stamp per-pixel depth for the bar's screen region
            if (image.depthrows)
            {
                s16 ry = (s16)drawy - image.wlogy1;
                s16 rx = image.precx;
                for (s16 r = 0; r < barheight; r++)
                {
                    s16 y = ry + r;
                    if (y < 0) continue;
                    if (y >= image.terrgbah) break;
                    u8 *drow = image.depthrows[y];
                    for (s16 c = 0; c < barwidth && (rx + c) < image.terrgbaw; c++)
                    {
                        if ((rx + c) >= 0) drow[rx + c] = image.stripdepth;
                    }
                }
            }
#endif
            
            // Render or hit-test the bar
            if (image.ftstpix == 0)
            {
                if ((s8)xread8(render_context - 0x400) == 0x0B)
                    bartra_textured_68k(terrain_cell, render_context, drawy, index, barwidth, barheight - image.vbarbot, bary);
                else
                    bartra_68k(terrain_cell, render_context, drawy, index, barwidth, barheight - image.vbarbot, bary);
            }
            else
            {
                hittest_bar(terrain_cell, render_context, drawy, barheight, barwidth, packed_coord, step_x, (s16)step_y);
            }
        }
    }
}


#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
u32 g_bar_ticks = 0, g_bar_calls = 0, g_bar_px = 0;   // barland time + fill-pixel count within one doland
u32 g_prescan_ticks = 0, g_prescan_iters = 0;         // occlusion prescan time + cell-sample count
#endif

// Occlusion prescan (orig dolanc @0x22862): walk grid cells between the previous and current
// screen column tracking max occlusion height; writes rc-0x25e/-0x260/-0x280 and `adresa`.
// Keep inlinable: noinline costs a movem per call (~1 iteration per call).
static void doland_prescan(s32 render_context, s32 terrain_grid, s32 alt_table,
                           u32 col_step_x, u32 col_step_y, s16 bar_screen_x,
                           u32 *scan_xp, u32 *scan_yp, s16 *prev_max_yp)
{
    u32 scan_x = *scan_xp;
    u32 scan_y = *scan_yp;
    s16 max_y = xread16(render_context - 0x260);
    s16 prev_max_y = xread16(render_context - 0x25e);
    if (prev_max_y < max_y)
        prev_max_y = max_y;

    s16 col_target_x = xread32hi16(render_context - 0x280);

    const s16 rc_294 = xread16(render_context - 0x294);   // grid X-bound
    const s16 rc_292 = xread16(render_context - 0x292);   // grid Y-bound
    const u8  rc_3fe = xread8(render_context - 0x3fe);     // wrap-enable flag

    while (col_target_x < bar_screen_x)
    {
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
        g_prescan_iters++;
#endif
        xwrite16(render_context - 0x25e, max_y);
        u16 prescan_height = 0;

        // Advance scan position by one column step (with carry)
        u16 next_scan_x = (s16)(col_step_x + scan_x) + (u16)carry4(col_step_x, scan_x);
        scan_x = concat22((s16)((col_step_x + scan_x) >> 0x10), next_scan_x);

        u16 next_scan_y = (s16)(col_step_y + scan_y) + (u16)carry4(col_step_y, scan_y);
        scan_y = concat22((s16)((col_step_y + scan_y) >> 0x10), next_scan_y);

        xwrite32(render_context - 0x280, xread32(render_context - 0x27c) + xread32(render_context - 0x280));

        if ((u16)rc_294 < next_scan_x)
        {
            if (rc_3fe == 1)
            {
                s16 wrap_x = 0;
                if (rc_294 <= (s16)next_scan_x)
                {
                    wrap_x = rc_294 * 2;
                }

                s32 strip_ptr = xread32(terrain_grid + (s32)(s16)(wrap_x - next_scan_x) * 4);
                u16 grid_height = (u16)rc_292;
                if (next_scan_y <= grid_height)
                {
                    if ((s16)grid_height <= (s16)next_scan_y)
                    {
                        strip_ptr += (s32)rc_292 << 2;
                    }

                    adresa = ((strip_ptr - (s16)next_scan_y) - (s32)(s16)next_scan_y);
                }
                else
                {
                    adresa = (strip_ptr + (s16)next_scan_y + (s32)(s16)next_scan_y);
                }

                prescan_height = (u16)(xread16((s32)adresa) & 0xff);
            }
        }
        else
        {
            // X in bounds: as in rrq-falcon.asm, only the Y-out-of-bounds case is wrap-gated;
            // an in-bounds cell is always sampled.
            if ((u16)rc_292 < next_scan_y)
            {
                // Y out of bounds — only real terrain here when wrapping is enabled.
                if (rc_3fe == 1)
                {
                    s32 strip_ptr = xread32(terrain_grid + (s16)(next_scan_x * 4));
                    u16 grid_height = (u16)rc_292;
                    if ((s16)grid_height <= (s16)next_scan_y)
                    {
                        strip_ptr += (s32)rc_292 << 2;
                    }

                    adresa = ((strip_ptr - (s16)next_scan_y) - (s32)(s16)next_scan_y);
                    prescan_height = (u16)(xread16((s32)adresa) & 0xff);
                }
                // wrap off: no terrain out here — prescan_height stays 0
            }
            else
            {
                // Y in bounds — always sample the real terrain height (no wrap gate).
                s32 strip_ptr = xread32(terrain_grid + (s16)(next_scan_x * 4));
                adresa = (strip_ptr + (s16)next_scan_y + (s32)(s16)next_scan_y);
                prescan_height = (u16)(xread16((s32)adresa) & 0xff);
            }
        }

        // Convert height to screen Y and track maximum
        max_y = xread32lo16(render_context - 0x270) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + prescan_height * 2));
        xwrite16(render_context - 0x260, max_y);
        if (prev_max_y < max_y)
        {
            prev_max_y = max_y;
        }

        col_target_x = xread32hi16(render_context - 0x280);
    }

    *scan_xp = scan_x;
    *scan_yp = scan_y;
    *prev_max_yp = prev_max_y;
}

// 68k word-swapped 16.16 step, as the original add.l/addx.w (sub.l/subx.w): the carry folds
// back into the integer word, so at fraction boundaries the net step can be zero.
// divs.w: 32/16 signed, quotient in the low word; on overflow the 68k leaves the dividend.
static inline s16 divs_w(s32 n, s16 d)
{
    if (d == 0)
        return (s16)n;   // a zero-divide trap in the original
    s32 q = n / d;
    return (q < -32768 || q > 32767) ? (s16)n : (s16)q;
}

static inline u32 swapstep_add(u32 pos_raw, u32 step_swp)
{
    u32 p = pos_raw << 16 | pos_raw >> 16;
    u32 r = p + step_swp;
    u16 lo = (u16)r + (u16)(r < p);        /* addx.w: carry out of add.l */
    return (u32)lo << 16 | r >> 16;
}
static inline u32 swapstep_sub(u32 pos_raw, u32 step_swp)
{
    u32 p = pos_raw << 16 | pos_raw >> 16;
    u32 r = p - step_swp;
    u16 lo = (u16)r - (u16)(p < step_swp); /* subx.w: borrow out of sub.l */
    return (u32)lo << 16 | r >> 16;
}

void doland_68k(s32 scene_addr, s32 render_context)
{
#if defined(ALIS_PROFILE_TEX)
    { extern void texprof_frame_begin(void); texprof_frame_begin(); }
#endif
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
    g_bar_ticks = 0; g_bar_calls = 0; g_bar_px = 0; g_prescan_ticks = 0; g_prescan_iters = 0;
#endif
    // =========================================================================
    // INITIALIZATION
    // =========================================================================

    OD(if (image.ftstpix == 0) od_begin();)

    // --- Sprite depth sorting setup ---
    image.spritprof = 0x8000;
    sSprite *sprite = SPRITE_VAR((u16)xread16(scene_addr +0x2));
    image.spritnext = sprite->link;
    if (image.spritnext != 0)
    {
        image.spritprof = SPRITE_VAR(image.spritnext)->newd;
    }

    // --- Render sky background ---
    if (xread16(scene_addr +0xa2) != 0)
    {
        skytofen(scene_addr, render_context);
    }

    // --- Terrain data pointers ---
    s32 alt_table = image.atalti;
    s32 terrain_grid = image.atlland;

    // --- Reset overhang interpolation state ---
    // These globals track overhang geometry across columns for smooth rendering.
    // prec = precedent (previous), top/bot = overhang top/bottom screen Y,
    // a/b/c = current/prev/prev-prev column, i = interpolated.
    fprectop = 0;
    fprectopa = 0;
    adresa = 0;
    prectopa = 1000;
    prectopb = 1000;
    prectopc = 1000;
    precbota = 1000;
    precbotb = 1000;
    precbotc = 1000;

    // --- Initialize rendering accumulators ---
    xwrite32(render_context - 0x25a, 0);
    xwrite32(render_context - 0x27c, 0);

    u16 cam_grid_x = (u16)xread16(render_context - 0x3a2);
    u16 cam_grid_y = (u16)xread16(render_context - 0x3a0);
    xwrite32(render_context - 0x2a8, (u32)cam_grid_x << 0x10);
    xwrite32(render_context - 0x2a4, (u32)cam_grid_y << 0x10);

    // Column step direction in terrain grid (16.16 fixed-point)
    // Word-swapped form used for 68k ADDX carry-chain emulation
    u32 col_step_x_raw = (u32)xread32(render_context - 0x2c4);
    u32 col_step_x = col_step_x_raw << 0x10 | col_step_x_raw >> 0x10;
    u32 col_step_y_raw = (u32)xread32(render_context - 0x2c0);
    u32 col_step_y = col_step_y_raw << 0x10 | col_step_y_raw >> 0x10;

    // Initial screen Y projection
    xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + xread16(render_context - 0x352)));

    // --- Initial perspective projection ---
    u16 cell_shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
    s16 proj_x = (cam_grid_x << cell_shift) - xread16(render_context - 0x360);
    s16 proj_y = (cam_grid_y << cell_shift) - xread16(render_context - 0x35e);

    glandtopix(render_context, &proj_x, &proj_y,
               (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
               (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
               xread32hi16(render_context - 0x2e0));

    xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
    // First row: << 8 / div << 8 (the per-row step below is << 6 / div << 10).
    xwrite32(render_context - 0x27c, (s32)divs_w((s16)(proj_y - proj_x) * 256, xread16(render_context - 0x3a4)) * 256);

    // Current traversal position in terrain grid (16.16 fixed-point)
    u32 trav_x = (u32)cam_grid_x << 16;
    u32 trav_y = (u32)cam_grid_y << 16;


    // =========================================================================
    // OUTER LOOP: Process terrain rows from far to near
    // =========================================================================
    do
    {
        // Falcon CD: compute horizontal texture base from current traversal position
        // Ghidra line 25063: DAT_0001fe82 = (trav_y_frac + trav_x_frac + carry) * 0x10
        image.tex_hbase = (s16)(((s16)trav_y + (s16)trav_x + (u16)carry4(trav_y, trav_x)) * 0x10);

        // ----- Advance row position accumulators -----
        xwrite32(render_context - 0x2a8, xread32(render_context - 0x2bc) + xread32(render_context - 0x2a8));
        xwrite32(render_context - 0x2a4, xread32(render_context - 0x2b8) + xread32(render_context - 0x2a4));

        u32 row_start_x = trav_x;
        u32 row_start_y = trav_y + (xread32hi16(render_context - 0x29c) << 16);

        // ----- Snap column position to terrain grid alignment -----
        // The column step is ±1 in either X or Y axis.
        // Snap the starting position so it aligns with the accumulated row position.
        // Safety bound: if s16 wrap-around causes divergence, break after 512 iterations.
        int _snap;
        if (col_step_x_raw == 0x00010000)
        {
            if (xread32hi16(render_context - 0x2bc) < 0)
            {
                for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a8) < (s16)(row_start_x >> 16); _snap--)
                {
                    row_start_y = swapstep_sub(row_start_y, col_step_y);
                    row_start_x = swapstep_sub(row_start_x, col_step_x);
                }
            }
            else
            {
                for (_snap = 512; _snap > 0 && (s16)(row_start_x >> 16) < xread32hi16(render_context - 0x2a8); _snap--)
                {
                    row_start_y = swapstep_add(row_start_y, col_step_y);
                    row_start_x = swapstep_add(row_start_x, col_step_x);
                }
            }
        }
        else if (col_step_x_raw == 0xffff0000)
        {
            if (xread32hi16(render_context - 0x2bc) < 0)
            {
                for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a8) < (s16)(row_start_x >> 16); _snap--)
                {
                    row_start_y = swapstep_add(row_start_y, col_step_y);
                    row_start_x = swapstep_add(row_start_x, col_step_x);
                }
            }
            else
            {
                for (_snap = 512; _snap > 0 && (s16)(row_start_x >> 16) < xread32hi16(render_context - 0x2a8); _snap--)
                {
                    row_start_y = swapstep_sub(row_start_y, col_step_y);
                    row_start_x = swapstep_sub(row_start_x, col_step_x);
                }
            }
        }
        else
        {
            // Column step is primarily in Y direction
            row_start_y = trav_y;
            row_start_x = trav_x + (xread32hi16(render_context - 0x2a0) << 16);
            if (col_step_y_raw == 0x00010000)
            {
                if (xread32hi16(render_context - 0x2b8) < 0)
                {
                    for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a4) < (s16)(row_start_y >> 16); _snap--)
                    {
                        row_start_y = swapstep_sub(row_start_y, col_step_y);
                        row_start_x = swapstep_sub(row_start_x, col_step_x);
                    }
                }
                else
                {
                    for (_snap = 512; _snap > 0 && (s16)(row_start_y >> 16) < xread32hi16(render_context - 0x2a4); _snap--)
                    {
                        row_start_y = swapstep_add(row_start_y, col_step_y);
                        row_start_x = swapstep_add(row_start_x, col_step_x);
                    }
                }
            }
            else
            {
                if (xread32hi16(render_context - 0x2b8) < 0)
                {
                    for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a4) < (s16)(row_start_y >> 16); _snap--)
                    {
                        row_start_y = swapstep_add(row_start_y, col_step_y);
                        row_start_x = swapstep_add(row_start_x, col_step_x);
                    }
                }
                else
                {
                    for (_snap = 512; _snap > 0 && (s16)(row_start_y >> 16) < xread32hi16(render_context - 0x2a4); _snap--)
                    {
                        row_start_y = swapstep_sub(row_start_y, col_step_y);
                        row_start_x = swapstep_sub(row_start_x, col_step_x);
                    }
                }
            }
        }

        // ----- Update fog/distance shading -----
        xwrite32(render_context - 0x256, xread32(render_context - 0x252) + xread32(render_context - 0x256));
        image.vdarkw = ((xread32(render_context - 0x256) >> 16) & 0xFF) << 8;

        // ----- Update depth (distance from camera) -----
        s32 depth = xread32(render_context - 0x2dc) + xread32(render_context - 0x2e0);
        xwrite32(render_context - 0x2e0, depth);
        if (depth < 0)
        {
            xwrite32(render_context - 0x2e0, xread32(render_context - 0x2e0) - xread32(render_context - 0x2dc));
        }

        // ----- Update vertical height projection -----
        xwrite32(render_context - 0x2d8, xread32(render_context - 0x2d4) + xread32(render_context - 0x2d8));

        // Prepare column rendering: word-swap for 68k fixed-point carry emulation
        // Save pre-glandtopix perspective value for Falcon CD texture step computation
        // (glandtopix overwrites rc-0x27c; Ghidra line 25156 saves before call)
        u32 saved_persp_27c = (u32)xread32(render_context - 0x27c);
        u32 col_x_step_swp = saved_persp_27c << 0x10 | saved_persp_27c >> 0x10;
        u32 col_x_pos_swp = (u32)xread32(render_context - 0x280) << 0x10 | (u32)xread32(render_context - 0x280) >> 0x10;

        xwrite32(render_context - 0x278, xread32(render_context - 0x270));

        // ----- Recalculate perspective if row is in front of camera -----
        if (-1 < xread16(render_context - 0x24e))
        {
            proj_x = ((s16)(row_start_x >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x360);
            proj_y = ((s16)(row_start_y >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x35e);
            glandtopix(render_context, &proj_x, &proj_y,
                       (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
                       (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
                       xread32hi16(render_context - 0x2e0));
            xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
            xwrite32(render_context - 0x27c, (s32)divs_w((s16)(proj_y - proj_x) * 64, xread16(render_context - 0x3a4)) * 1024);

            u16 proj_denom = (u16)(xread16(render_context - 0x3a8) + xread32hi16(render_context - 0x2e0));
            if (proj_denom == 0 || scarry2(xread16(render_context - 0x3a8), xread32hi16(render_context - 0x2e0)) != (s32)((u32)proj_denom << 0x10) < 0)
            {
                proj_denom = 1;
            }

            xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + divs_w((s32)xread32(render_context - 0x2d8), (s16)proj_denom)));
        }

        // ----- Render sprites at this depth layer -----
        if (xread32hi16(render_context - 0x2e0) <= image.spritprof)
        {
            spritaff(xread32hi16(render_context - 0x2e0));
        }

        // ----- Select altitude table segment for current distance -----
        alt_table += xread16(render_context - 0x25a);
        u16 alt_seg_idx = (u16)divs_w((s32)(xread32(render_context - 0x2e0) - xread32(render_context - 0x2c8)) >> 8, xread16(render_context - 0x262));

        {
            u16 max_seg = 0x31;

            if (xread16(render_context - 0x24e) == 1)
            {
                alt_seg_idx = max_seg;
            }

            if (max_seg < alt_seg_idx)
            {
                if ((s16)alt_seg_idx < 0)
                {
                    alt_seg_idx = 0;
                }
                else
                {
                    alt_seg_idx = max_seg;
                }
            }

            xwrite16(render_context - 0x25a, ((s16)image.atalti + alt_seg_idx * 0x200) - (s16)alt_table);
#if ALIS_SDL_VER > 1
            // Set strip depth for enhanced renderer fog: 0=near, 255=far
            // alt_seg_idx is low at far distance, high at near — invert
            image.stripdepth = (u8)(255 - (u32)alt_seg_idx * 255 / max_seg);
#endif
        }

        // Falcon CD: compute texture perspective step per altitude segment
        // Ghidra lines 25199-25207: DAT_0001fe7a = 0x200000 / perspective, ROL-encoded
        // Uses saved_persp_27c (pre-glandtopix value), NOT current rc-0x27c which was overwritten
        {
            u32 persp_val = saved_persp_27c;
            u32 divisor = (persp_val >> 8) & 0xFFFF;
            if (divisor > 0)
            {
                u16 quot = (u16)(0x200000 / divisor);
                image.tex_persp_step = ((u32)(u8)(quot) << 24) | (u8)(quot >> 8);
                if (0x200000 % divisor != 0)
                    image.tex_persp_step = ((u32)(u8)(quot) << 24) | (u16)((u8)(quot >> 8) + 1);
            }
            else
            {
                image.tex_persp_step = 0;
            }
        }

        // ----- Reset per-row rendering state -----
        image.precx = image.clipx1 - 0x10;
        xwrite16(render_context - 0x260, image.clipy2);
        xwrite16(render_context - 0x25c, image.clipy2);
        fprectop = 0;
        fprectopa = 0;

        // ----- Decrement row counter -----
        u16 rows_remaining = (u16)xread16(render_context - 0x24e) - 1;
        xwrite16(render_context - 0x24e, rows_remaining);
        if (((s32)((u32)rows_remaining << 0x10) < 0) && (xread16(render_context - 0x24e) < -3))
        {
            spritaff(-1);
            OD(if (image.ftstpix == 0) od_end();)
            return;
        }
        OD(od_rows++; if (od_rows < 1024) od_cells_in_row[od_rows] = 0;)

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
        rrq_row_note((s16)xread16(render_context - 0x24e), col_x_pos_swp, col_x_step_swp,
                     trav_x << 16 | trav_x >> 16, trav_y << 16 | trav_y >> 16,
                     row_start_x << 16 | row_start_x >> 16, row_start_y << 16 | row_start_y >> 16);
#endif
        // =====================================================================
        // INNER LOOP: Process columns left-to-right within this row
        // =====================================================================
        s32 saved_col_x = xread32(render_context - 0x280);
        u32 scan_x = concat22(row_start_x, row_start_x >> 16);
        u32 scan_y = concat22(row_start_y, row_start_y >> 16);

        do
        {
            s16 bar_screen_x = (s16)col_x_pos_swp;
            OD(od_cells++; if (od_rows < 1024) od_cells_in_row[od_rows]++;)

            // Hoisted terrain-map constants — also consumed by the main cell lookup below.
            const s16 rc_294 = xread16(render_context - 0x294);   // grid X-bound
            const s16 rc_292 = xread16(render_context - 0x292);   // grid Y-bound
            const u8  rc_3fe = xread8(render_context - 0x3fe);    // wrap-enable flag

            s16 prev_max_y;
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
            { extern u32 sys_profile_ticks_safe(void); u32 _pt = sys_profile_ticks_safe();
#endif
            doland_prescan(render_context, terrain_grid, alt_table, col_step_x, col_step_y,
                           bar_screen_x, &scan_x, &scan_y, &prev_max_y);
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
            g_prescan_ticks += sys_profile_ticks_safe() - _pt; }
#endif

            // =================================================================
            // Main terrain lookup at center column position
            // =================================================================
            u16 center_grid_x = (u16)(trav_x >> 16);
            u16 center_grid_y = (u16)(trav_y >> 16);
            u32 terrain_cell;
            s32 strip_ptr_main;
            u16 grid_h;
            u8 terrain_wrapped = 0;

            if ((u16)rc_294 < center_grid_x)
            {
                if (rc_3fe == 1)
                {
                    terrain_wrapped = 1;
                    s16 wrap_x = 0;
                    if (rc_294 <= (s16)center_grid_x)
                    {
                        wrap_x = rc_294 * 2;
                    }

                    strip_ptr_main = xread32(terrain_grid + (s16)((wrap_x - center_grid_x) * 4));
                    grid_h = (u16)rc_292;
                    if (center_grid_y <= grid_h)
                    {
                        goto cell_calc_subtract;
                    }
                    else
                    {
                        goto cell_calc_add;
                    }
                }
            }
            else
            {
                if ((u16)rc_292 < center_grid_y)
                {
                    if (rc_3fe != 1)
                        goto advance_column;

                    terrain_wrapped = 1;
                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));
                    grid_h = (u16)rc_292;

                cell_calc_subtract:

                    if ((s16)grid_h <= (s16)center_grid_y)
                    {
                        strip_ptr_main += (s32)rc_292 << 2;
                    }

                    terrain_cell = ((strip_ptr_main - (s16)center_grid_y) - (s32)(s16)center_grid_y);
                }
                else
                {
                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));

                cell_calc_add:

                    terrain_cell = (strip_ptr_main + (s16)center_grid_y + (s32)(s16)center_grid_y);
                }

                // --- Read terrain cell and compute ground screen Y ---
                s16 ground_clip_y = xread16(render_context - 0x25c);
                u16 cell_data = xread16(terrain_cell);
                image.solh = cell_data & 0xff;
                image.solpixy = xread32lo16(render_context - 0x278) + xread16(alt_table + (s16)(image.solh * 2));
                xwrite16(render_context - 0x246, image.solpixy);
                xwrite16(render_context - 0x25c, image.solpixy);
                if (image.solpixy < ground_clip_y)
                {
                    xwrite16(render_context - 0x246, ground_clip_y);
                    ground_clip_y = image.solpixy;
                }

                s16 col_dir_x = (s16)(col_step_x_raw >> 0x10);

                // Extract terrain type index: bits [13:8] encode type, shifted for table lookup
                s16 terrain_type_idx = ((cell_data & 0x3f00) >> 3) - 0xc00;
                u16 bar_height = prev_max_y - ground_clip_y;

                // --- Render ground terrain bar ---
                if (bar_height != 0 && sborrow2(prev_max_y, ground_clip_y) == (s32)((u32)bar_height << 0x10) < 0)
                {
                    u32 packed_pos = concat22((trav_x), (trav_x >> 16));
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
                    { extern u32 sys_profile_ticks_safe(void); u32 _bt = sys_profile_ticks_safe();
#endif
#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
                    rrq_bar_note(1, (s16)xread16(render_context - 0x24e), bar_screen_x, ground_clip_y, bar_height, image.precx);
#endif
                    barland_68k(terrain_cell, render_context, col_dir_x, (s16)(col_step_y_raw >> 0x10), ground_clip_y, bar_height, terrain_type_idx, bar_screen_x, packed_pos, scan_x);
#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
                    rrq_cell_x = (s16)center_grid_x; rrq_cell_y = (s16)center_grid_y;
#endif
                    RRQ_WATCH("bar", render_context, ground_clip_y, bar_height, bar_screen_x, terrain_type_idx);
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
                    g_bar_ticks += sys_profile_ticks_safe() - _bt; g_bar_calls++; }
#endif
                }

                // =============================================================
                // Check for terrain features: overhangs or billboard sprites
                // =============================================================
                // Feature flag at rc[0x14 + type]:
                //   < 0 (bit 7 set): overhang/bridge
                //   == 1: billboard sprite
                //   == 0: no feature
                if (xread8(render_context + terrain_type_idx + 0x14) != 0)
                {
                    if ((s8)xread8(render_context + terrain_type_idx + 0x14) < 0)
                    {
                        // =====================================================
                        // OVERHANG / BRIDGE RENDERING
                        // =====================================================
                        // dotop (orig 0x22ce8), transliterated from the asm: continuity with the
                        // previous column's overhang (adresa), then this cell's overhang bar.
                        s16 d0 = 0, d1 = 0, d2 = 0, d6 = (s16)prectopa, d7 = (s16)precbota;
                        notopa = 1;
                        if (adresa != 0)
                        {
                            u16 w = xread16(adresa);
                            solha = w & 0xff;
                            s16 m = ((w & 0x3f00) >> 3) - 0xc00;
                            int open = (s8)xread8(render_context + 0x14 + m) < 0;
                            if (open)
                            {
                                d2 = xread16(render_context + 0x16 + m);
                                d0 = xread8(adresa + xread32(render_context + 0x10 + m) + 1);
                                d1 = d0 - (s16)solha;
                                open = d1 > 0 && (d2 -= d1) > 0;
                            }
                            if (!open)
                                fprectopa = 0;   // dotopa7
                            else
                            {
                                d2 += solha;
                                d1 = d0;
                                if (d2 < d0)
                                    d1 = d2 - solha;
                                d1 = d0 - d1;
                                s16 seg = xread16(render_context - 0x25a), base = xread32lo16(render_context - 0x270);
                                d0 = xread16(alt_table + (s16)(d0 * 2 + seg)) + base;
                                prectopa = d0;
                                d1 = xread16(alt_table + (s16)(d1 * 2 + seg)) + base;
                                precbota = d1;
                                adresa = 0;
                                if (!fprectopa)
                                {
                                    fprectopa = 1;
                                    prectopc = 5000;
                                    precbotc = (u16)-5000;
                                }
                                else
                                {
                                    prectopb = prectopc;
                                    precbotb = precbotc;
                                    if (d0 > d6) d6 = d0;
                                    prectopc = d6;
                                    if (d6 < (s16)prectopb) d6 = prectopb;
                                    if (d1 < d7) d7 = d1;
                                    precbotc = d7;
                                    if (d7 > (s16)precbotb) d7 = precbotb;
                                    if (d6 < d7) notopa = 0;
                                }
                            }
                        }

                        // dotopa5: this cell's overhang layer
                        terrain_cell = xread32(render_context + 0x10 + terrain_type_idx) + (s32)(s16)center_grid_y * 2 + xread32(terrain_grid + (s16)(center_grid_x << 2));
                        d2 = xread16(render_context + 0x16 + terrain_type_idx);
                        fbottom = 0;
                        u16 oh_cell = xread16(terrain_cell);
                        image.toph = oh_cell & 0xff;
                        s16 oh_type_idx = ((oh_cell & 0x3f00) >> 3) - 0xc00;
                        d0 = image.toph;
                        d1 = d0 - (s16)image.solh;
                        if (d1 <= 0 || (d2 -= d1) <= 0)
                            goto advance_column;   // dotop6: no overhang, fprectop unchanged
                        d2 += image.solh;
                        if (d2 < d0)
                        {
                            d1 = d2 - image.solh;
                            fbottom = 1;
                        }
                        d1 = d0 - d1;
                        s16 row_base = xread32lo16(render_context - 0x278);
                        d0 = xread16(alt_table + (s16)(d0 * 2)) + row_base;
                        image.toppixy = d0;
                        s16 old246 = xread16(render_context - 0x246);
                        xwrite16(render_context - 0x246, d0);
                        s16 old_top = (s16)prectopi;
                        prectopi = d0;
                        if (old_top > d0)
                            xwrite16(render_context - 0x246, old_top);   // -0x246 = max, d0 = min
                        else
                            d0 = old_top;
                        d1 = fbottom ? xread16(alt_table + (s16)(d1 * 2)) + row_base : old246;
                        bothigh = d1;
                        s16 old_bot = (s16)precboti;
                        precboti = d1;
                        if (old_bot < d1)
                            bothigh = old_bot;                            // bothigh = min, botalt = max
                        else
                            d1 = old_bot;
                        botalt = d1;

                        if (fprectop && (d1 -= d0) > 0)
                        {
                            int draw = 1;
                            if (!notopa)
                            {
                                // dotop30/31: clip against the previous column's overhang span
                                if ((d6 -= d0) > 0)
                                {
                                    d7 -= d0;
                                    if (d1 <= d7)
                                    {
                                        fbottom = 0;
                                        d1 = d6;
                                    }
                                }
                                else
                                {
                                    d1 += d0;
                                    d0 = d7;
                                    d1 -= d7;
                                    draw = d1 > 0;
                                }
                            }
                            if (draw)
                            {
                                s16 saved_clip = xread16(render_context - 0x25c);
                                xwrite16(render_context - 0x25c, prectopi);
                                u32 packed_pos = concat22((trav_x), (trav_x >> 16));
#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
                                rrq_bar_note(2, (s16)xread16(render_context - 0x24e), (s16)col_x_pos_swp, d0, d1, image.precx);
#endif
                                tbarland_68k(terrain_cell, render_context, col_dir_x, col_step_y, d0, d1, oh_type_idx, col_x_pos_swp, packed_pos, 0);
                                RRQ_WATCH("tbar", render_context, d0, d1, (s16)col_x_pos_swp, oh_type_idx);
                                xwrite16(render_context - 0x25c, saved_clip);
                            }
                        }
                        fprectop = 1;   // dotop4

                        goto advance_column;
                    }

                    if ((s8)xread8(render_context + terrain_type_idx + 0x14) != 1)
                        goto advance_column;

                    // --- Billboard sprite at this terrain cell ---
                    barsprite(render_context, terrain_type_idx, center_grid_x, center_grid_y, 0);
                    RRQ_WATCH("spr", render_context, terrain_type_idx, center_grid_x, center_grid_y, 0);
                    OD(od_sprites++;)
                }

                fprectop = 0;
                fprectopa = 0;
            }

        advance_column:

            // Falcon CD: accumulate horizontal texture base per column (Ghidra line 25460)
            image.tex_hbase += 0x20;

            image.precx = bar_screen_x;
            if (image.landclipx2 < bar_screen_x)
                break;

            // Step to next column. Orig dobar3 @0x22a48: `add.l a4,d4 / addx.w d0,d4` on the
            // word-swapped position; same swapped carry as the snap and prescan.
            trav_x = swapstep_add(trav_x, col_step_x);
            trav_y = swapstep_add(trav_y, col_step_y);

            // Advance screen column X (fixed-point with carry emulation)
            s32 col_x_sum = col_x_step_swp + col_x_pos_swp;
            col_x_pos_swp = concat22((s16)((u32)col_x_sum >> 0x10), (s16)col_x_sum + (u16)carry4(col_x_step_swp, col_x_pos_swp));
        }
        while (true);

        // Restore row state for next iteration
        xwrite32(render_context - 0x280, saved_col_x);
        trav_x = row_start_x;
        trav_y = row_start_y;
    }
    while (true);
}

void landtofi_68k(s16 unused, s16 scene_id)
{
    s32 scene_addr = alis.basemain + scene_id;

    // m68k path: original behavior with fdoland gate
    s16 save_cliph = image.cliph;
    s16 save_clipl = image.clipl;
    s16 save_clipy2 = image.clipy2;
    s16 save_clipx2 = image.clipx2;
    s16 save_clipy1 = image.clipy1;
    s16 save_clipx1 = image.clipx1;

    image.wloglarg = xread16(scene_addr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scene_addr + 0x30)));
    image.wlogx1 = xread16(scene_addr + 0xe);
    image.wlogy1 = xread16(scene_addr + 0x10);
    image.mapscreen = scene_addr;

#if defined(ALIS_TRACE_LAND)
    extern void dbglog(const char *fmt, ...);
#if defined(ALIS_PROFILE_DRAW)
    extern u32 sys_profile_ticks_safe(void);
    #define LT_MS() (sys_profile_ticks_safe() * 5u)   /* 200Hz ticker */
#else
    #define LT_MS() (alis.timeclock * 20u)             /* ~50Hz fallback */
#endif
    u32 _lt0 = LT_MS(), _lt_setup = 0, _lt_clr = 0, _lt_dol = 0;
    s16 _lt_x1 = image.clipx1, _lt_x2 = image.clipx2;
    s16 _lt_y1 = image.clipy1, _lt_y2 = image.clipy2;
    u8  _lt_fdo = image.fdoland;
#endif

    if (image.fdoland != 0)
    {
        image.landclipx2 = xread16(scene_addr + 0x12) + xread16(scene_addr + 0xe);
        // Don't widen clipx2 to the scene edge: only the CD build stores it (0x1ea26); ST
        // (0x1c918) and Falcon-floppy (0x1f460) keep partial re-renders bounded to the dirty
        // window. We follow ST/floppy.
        image.landcliph = (image.clipy2 - image.clipy1) + 1;
        image.clipl = (image.clipx2 - image.clipx1) + 1;
        image.vbarclipx2 = image.clipx2 + 1;
        image.landclipy1 = image.clipy1;

        s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
        image.cliph = image.landcliph;
        calctoy(scene_addr, render_context);
        spritland(scene_addr, xread16(scene_addr + 2));
        calclan0_68k(scene_addr, render_context);
#if defined(ALIS_TRACE_LAND)
        _lt_setup = LT_MS();
#endif

        if (((xread8(scene_addr + 1) & 0x40) == 0) && (xread16(scene_addr + 0xa2) == 0))
        {
            clrvga_68k();
        }
#if defined(ALIS_TRACE_LAND)
        _lt_clr = LT_MS();
#endif

        /* via doland fn-ptr so the asm renderer is used here too */
        doland(scene_addr, render_context);
#if defined(ALIS_TRACE_LAND)
        _lt_dol = LT_MS();
#endif

        image.fdoland = 0;
        image.landone = 1;
    }

    image.clipx1 = save_clipx1;
    image.clipy1 = save_clipy1;
    image.clipx2 = save_clipx2;
    image.clipy2 = save_clipy2;
    image.clipl = save_clipl;
    image.cliph = save_cliph;

#if ALIS_SDL_VER > 1
    // Enhanced: build terrain RGBA from 8-bit buffer with per-row depth fog
    if (image.emode)
    {
        vgatobuf();
    }
    else
#endif
    {
        vgatofen();
    }
    
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;

#if defined(ALIS_TRACE_LAND)
    {
        static int _lt_n = 0;
        if (_lt_n < 80) {
            _lt_n++;
            u32 _lt_end = LT_MS();
            if (_lt_fdo)
            {
                // terrain type: 0x0B = CD textured bars, else flat dither fills
                s32 _lt_rc = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
                dbglog("[land] landtofi #%d: fdoland=1 type=%02x win=[%d..%d]x[%d..%d] setup=%ums clrvga=%ums doland=%ums vgatofen=%ums total=%ums\n",
                       _lt_n, (unsigned)xread8(_lt_rc - 0x400), _lt_x1, _lt_x2, _lt_y1, _lt_y2,
                       (unsigned)(_lt_setup - _lt0), (unsigned)(_lt_clr - _lt_setup),
                       (unsigned)(_lt_dol - _lt_clr), (unsigned)(_lt_end - _lt_dol),
                       (unsigned)(_lt_end - _lt0));
            }
            else
                dbglog("[land] landtofi #%d: fdoland=0 win=[%d..%d]x[%d..%d] vgatofen=%ums\n",
                       _lt_n, _lt_x1, _lt_x2, _lt_y1, _lt_y2, (unsigned)(_lt_end - _lt0));
        }
    }
    #undef LT_MS
#endif
}
