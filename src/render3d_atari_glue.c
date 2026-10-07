//
// render3d_atari_glue.c — per-call sync between the engine state (image.*/alis.*)
// and the lifted original doland asm's mirror block (doland_mirror in render3d_atari.S).
// Contract: tools/asm_lift/entry_contract.md; layout lock: asserts below.
//
#include "config.h"

#if defined(ALIS_RRQ_ASM_DOLAND) && ALIS_RRQ_ASM_DOLAND

#include <stddef.h>
#include <string.h>
#include "alis.h"
#include "image.h"
#include "render3d.h"
#include "render3d_68k.h"
#include "render3d_atari_glue.h"

/* Freeze the asm-visible offsets. If one of these fires, the .equ block in
   render3d_atari.S no longer matches — fix BOTH, never just the struct. */
#define RRQ_OFF(f, o) _Static_assert(offsetof(doland_mirror_t, f) == (o), "doland_mirror." #f)
RRQ_OFF(atlland, 0);   RRQ_OFF(atlpix, 4);    RRQ_OFF(atalti, 8);   RRQ_OFF(atalias, 12);
RRQ_OFF(ptrdark, 16);  RRQ_OFF(basesprite, 20); RRQ_OFF(atexsprite, 24); RRQ_OFF(mapscreen, 28);
RRQ_OFF(memfix, 32);
RRQ_OFF(clipx1, 36);   RRQ_OFF(clipy2, 38);   RRQ_OFF(landclipy1, 40); RRQ_OFF(landclipx2, 42);
RRQ_OFF(landcliph, 44); RRQ_OFF(vbarclipx2, 46); RRQ_OFF(wlogy1, 48); RRQ_OFF(wloglarg, 50);
RRQ_OFF(vdarkw, 52);   RRQ_OFF(precx, 54);    RRQ_OFF(spritprof, 56); RRQ_OFF(spritnext, 58);
RRQ_OFF(solh, 60);     RRQ_OFF(solpixy, 62);  RRQ_OFF(toph, 64);    RRQ_OFF(toppixy, 66);
RRQ_OFF(vbarlarg, 68); RRQ_OFF(vbarx, 70);    RRQ_OFF(vbarmid, 72); RRQ_OFF(vbarbot, 74);
RRQ_OFF(ftstpix, 76);  RRQ_OFF(xtstpix, 78);  RRQ_OFF(ytstpix, 80);
RRQ_OFF(cxtstpix, 82); RRQ_OFF(cytstpix, 84); RRQ_OFF(cztstpix, 86);
RRQ_OFF(dtstpix, 88);  RRQ_OFF(ntstpix, 90);  RRQ_OFF(etstpix, 92);
RRQ_OFF(is4bit, 94);
RRQ_OFF(tex_saved_persp, 96); RRQ_OFF(tex_persp, 100); RRQ_OFF(tex_vstep, 104);
RRQ_OFF(tex_hstep, 108);      RRQ_OFF(tex_v, 112);      RRQ_OFF(tex_base, 116);
RRQ_OFF(tex_wmask, 120);      RRQ_OFF(tex_hmask, 122);  RRQ_OFF(tex_dark, 124);
_Static_assert(sizeof(doland_mirror_t) == 128, "doland_mirror size");
#undef RRQ_OFF

/* The lifted asm walks OUR sprite records directly (depth dispatch reads
   state/link/newd/... at raw original offsets). sSprite is packed to the
   original layout — lock the fields the asm touches. */
_Static_assert(sizeof(sSprite) == 0x30, "sSprite stride");
#define SPR_OFF(f, o) _Static_assert(offsetof(sSprite, f) == (o), "sSprite." #f)
SPR_OFF(state, 0x00); SPR_OFF(screen_id, 0x02); SPR_OFF(link, 0x06);
SPR_OFF(newx, 0x0c);  SPR_OFF(newy, 0x0e);     SPR_OFF(newd, 0x10);
SPR_OFF(depx, 0x16);  SPR_OFF(clinking, 0x1e); SPR_OFF(width, 0x28);
SPR_OFF(height, 0x2a); SPR_OFF(newzoomx, 0x2c);
#undef SPR_OFF

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
/* ---------------------------------------------------------------------------
 * Byte-compare harness: run the C reference renderer, snapshot its output,
 * restore ALL inputs, run the lifted asm, and compare. The asm result stays
 * in the buffer (so divergences are also visible on screen). Compares the
 * VGA work buffer plus every out-field the glue writes back.
 * ------------------------------------------------------------------------- */
#define RRQ_VER_MAX (60 * 1024)
static u8 rrq_ver_in[RRQ_VER_MAX];    /* input buffer snapshot   */
static u8 rrq_ver_c[RRQ_VER_MAX];     /* C reference output      */
static u8 rrq_ver_rc[0x420];          /* rc block  -0x400..+0x20 */
static u32 rrq_ver_calls, rrq_ver_bad;
static u32 rrq_hit_calls, rrq_hit_bad;    /* ftstpix hit-test harness */

static u32 rrq_ver_span(void)
{
    s32 rows = (image.landclipy1 - image.wlogy1) + image.landcliph + 2;
    u32 n = (u32)rows * (u16)image.wloglarg + 0x140;
    return n > RRQ_VER_MAX ? RRQ_VER_MAX : n;
}

typedef struct {                       /* out-field pack for comparison */
    s16 precx, solh, solpixy, toph, toppixy, vdarkw;
    s32 spritprof, spritnext;
    s16 vbarlarg, vbarx, vbarmid, vbarbot;
    s16 cx, cy, cz, d, n, e;
} rrq_ver_outs;

/* Call-trace comparator: C spritaff/barsprite/glandtopix/skytofen entries note
   themselves here (hooks in render3d.c under the same flag). The asm run hits
   the SAME C bodies via the thunks, so one hook set captures both phases. */
#define RRQ_TR_MAX 4096
typedef struct { u8 kind; s16 a, b, c, d; } rrq_tr_ent;   /* kind: 1=spritaff 2=barsprite 3=glandtopix 4=skytofen 5=barland 6=tbarland */
static rrq_tr_ent rrq_tr[2][RRQ_TR_MAX];
static u16 rrq_tr_n[2];
static u8  rrq_tr_phase;               /* 0 = C reference run, 1 = asm run */
static u8  rrq_tr_live;

void rrq_ver_note(u8 kind, s16 a, s16 b, s16 c, s16 d)
{
    if (!rrq_tr_live) return;
    u16 *n = &rrq_tr_n[rrq_tr_phase];
    if (*n >= RRQ_TR_MAX) return;
    rrq_tr[rrq_tr_phase][*n] = (rrq_tr_ent){ kind, a, b, c, d };
    (*n)++;
}

static void rrq_ver_grab_outs(rrq_ver_outs *o)
{
    o->precx = image.precx; o->solh = image.solh; o->solpixy = image.solpixy;
    o->toph = image.toph; o->toppixy = image.toppixy; o->vdarkw = image.vdarkw;
    o->spritprof = image.spritprof; o->spritnext = image.spritnext;
    o->vbarlarg = image.vbarlarg; o->vbarx = image.vbarx;
    o->vbarmid = image.vbarmid; o->vbarbot = image.vbarbot;
    o->cx = image.cxtstpix; o->cy = image.cytstpix; o->cz = image.cztstpix;
    o->d = image.dtstpix; o->n = image.ntstpix; o->e = image.etstpix;
}
#endif /* ALIS_RRQ_ASM_DOLAND_VERIFY */

/* Sprite suppression for the verify harness: barsprite/spritaff (thunked, so
   both C and asm phases hit these C bodies) early-return when set, isolating
   terrain-only output. */
int rrq_ver_suppress = 0;

typedef void (*doland_body_fn)(u8 *scene_ptr, u8 *rc_ptr);
static void doland_asm_run_body(doland_body_fn body, s32 scene_addr, s32 render_context);
#define doland_asm_run(s, r) doland_asm_run_body(doland_body_asm, (s), (r))

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY == 2
/* Reference = the pre-optimisation asm (render3d_atari_ref.S), not the drifted C. */
extern void doland_body_asm_ref(u8 *scene_ptr, u8 *rc_ptr);
extern void initltra_asm_ref(void);
static void doland_ref(s32 scene_addr, s32 render_context)
{
    static u8 built;
    if (!built) { initltra_asm_ref(); built = 1; }
    doland_asm_run_body(doland_body_asm_ref, scene_addr, render_context);
}
#else
#define doland_ref doland_68k
#endif

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
static void rrq_ver_restore(s32 render_context, u32 span, const rrq_ver_outs *pre)
{
    memcpy(image.wlogic, rrq_ver_in, span);
    memcpy(alis.mem + render_context - 0x400, rrq_ver_rc, 0x420);
    image.precx = pre->precx; image.solh = pre->solh;
    image.solpixy = pre->solpixy; image.toph = pre->toph;
    image.toppixy = pre->toppixy; image.vdarkw = pre->vdarkw;
    image.spritprof = pre->spritprof; image.spritnext = pre->spritnext;
    image.vbarlarg = pre->vbarlarg; image.vbarx = pre->vbarx;
    image.vbarmid = pre->vbarmid; image.vbarbot = pre->vbarbot;
}
static u32 rrq_ver_cmp(u32 span, const u8 *ref, u32 *first)
{
    u32 diff = 0; *first = 0xffffffff;
    for (u32 i = 0; i < span; i++)
        if (image.wlogic[i] != ref[i]) { if (!diff) *first = i; diff++; }
    return diff;
}
#endif

void doland_asm(s32 scene_addr, s32 render_context)
{
    /* The asm covers 8- and 4-bit buffers, flat and textured (0x0B) terrain,
       and the ftstpix hit-test (barlapix probe, orig 0x2077c; sprite hits stay
       in C spritaff via spritaff_thunk). ddrawdist needs no gate: openland
       doubles scene 0x6a/0x6c/0x6e, which reach the asm through the rc. */
    if ((alis.platform.bpp != 8 && alis.platform.bpp != 4)
        || (u16)xread16(render_context - 0x294) >= DOLAND_LANDTAB_MAX)
    {
        doland_68k(scene_addr, render_context);
        return;
    }

    /* one-time: build the four line jump tables (idempotent guard inside asm) */
    static u8 tables_built = 0;
    if (!tables_built) { initltra_asm(); tables_built = 1; }

#if defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY
    extern void dbglog(const char *fmt, ...);

    /* ---- ftstpix hit-test harness: compare the hit-output fields C vs asm,
       then leave the TRUSTED C result live (a wrong hit-test would break
       gameplay and, unlike the render harness, is not visible on screen — so we
       keep C authoritative while the asm is only validated in the log). ---- */
    if (image.ftstpix) {
        rrq_ver_outs pre, cref, aref;
        s32 ant_pre, ant_c, ant_a;
        rrq_ver_suppress = 0;   /* sprites ON — sprite hits must register in both runs */
        memcpy(rrq_ver_rc, alis.mem + render_context - 0x400, 0x420);
        rrq_ver_grab_outs(&pre); ant_pre = image.antstpix;

        /* run C reference */
        doland_ref(scene_addr, render_context);
        rrq_ver_grab_outs(&cref); ant_c = image.antstpix;

        /* restore every input the traversal touched, then run the asm */
        memcpy(alis.mem + render_context - 0x400, rrq_ver_rc, 0x420);
        image.precx = pre.precx; image.solh = pre.solh; image.solpixy = pre.solpixy;
        image.toph = pre.toph; image.toppixy = pre.toppixy; image.vdarkw = pre.vdarkw;
        image.spritprof = pre.spritprof; image.spritnext = pre.spritnext;
        image.vbarlarg = pre.vbarlarg; image.vbarx = pre.vbarx;
        image.vbarmid = pre.vbarmid; image.vbarbot = pre.vbarbot;
        image.cxtstpix = pre.cx; image.cytstpix = pre.cy; image.cztstpix = pre.cz;
        image.dtstpix = pre.d; image.ntstpix = pre.n; image.etstpix = pre.e;
        image.antstpix = ant_pre;
        doland_asm_run(scene_addr, render_context);
        rrq_ver_grab_outs(&aref); ant_a = image.antstpix;

        rrq_hit_calls++;
        /* Compare only the fields that survive into the VM-visible result
           (cpointpix): etstpix + dtstpix always; the ntstpix
           SOURCE per hit type — antstpix for a sprite hit (e>=0, pointpix
           recomputes ntstpix from it) or the raw ntstpix for a terrain hit
           (e==-2). Raw cx/cy/cz are OVERWRITTEN by pixtoland (from dtstpix), so
           they're logged for diagnosis but never trigger BAD on their own. */
        int e_bad  = (cref.e != aref.e);
        int d_bad  = (cref.d != aref.d);
        int id_bad = (cref.e >= 0)  ? (ant_c != ant_a)
                   : (cref.e == -2) ? (cref.n != aref.n)
                   : 0;
        if (e_bad || d_bad || id_bad) {
            rrq_hit_bad++;
            dbglog("[rrqhit] #%u BAD%s%s%s e C=%d A=%d d C=%d A=%d n C=%d A=%d an C=%ld A=%ld | rawcx C=%d A=%d cy C=%d A=%d cz C=%d A=%d xy=%d,%d\n",
                   rrq_hit_calls, e_bad?" E":"", d_bad?" D":"", id_bad?" ID":"",
                   cref.e, aref.e, cref.d, aref.d, cref.n, aref.n,
                   (long)ant_c, (long)ant_a, cref.cx, aref.cx, cref.cy, aref.cy,
                   cref.cz, aref.cz, image.xtstpix, image.ytstpix);
        } else if (rrq_hit_calls <= 20 || (rrq_hit_calls & 0x3f) == 0) {
            dbglog("[rrqhit] #%u OK (%u bad) e=%d d=%d n=%d\n",
                   rrq_hit_calls, rrq_hit_bad, cref.e, cref.d, cref.n);
        }

        /* leave the trusted C result live for gameplay */
        image.cxtstpix = cref.cx; image.cytstpix = cref.cy; image.cztstpix = cref.cz;
        image.dtstpix = cref.d; image.ntstpix = cref.n; image.etstpix = cref.e;
        image.antstpix = ant_c;
        return;
    }

    static u8 rrq_ver_terr[RRQ_VER_MAX];   /* C terrain-only output */
    u32 span = rrq_ver_span();
    rrq_ver_outs outs_pre;
    u32 first_f, first_t, tdiff, fdiff;
    /* snapshot inputs */
    memcpy(rrq_ver_in, image.wlogic, span);
    memcpy(rrq_ver_rc, alis.mem + render_context - 0x400, 0x420);
    rrq_ver_grab_outs(&outs_pre);

    /* run 1: C reference, sprites ON, traced -> C_full */
    rrq_ver_suppress = 0;
    rrq_tr_live = 1; rrq_tr_phase = 0; rrq_tr_n[0] = rrq_tr_n[1] = 0;
    doland_ref(scene_addr, render_context);
    rrq_tr_live = 0;
    memcpy(rrq_ver_c, image.wlogic, span);

    /* run 2: C reference, sprites OFF -> C_terrain */
    rrq_ver_restore(render_context, span, &outs_pre);
    rrq_ver_suppress = 1;
    doland_ref(scene_addr, render_context);
    memcpy(rrq_ver_terr, image.wlogic, span);

    /* run 3: asm, sprites OFF -> compare to C_terrain (isolates the BARS) */
    rrq_ver_restore(render_context, span, &outs_pre);
    rrq_ver_suppress = 1;
    doland_asm_run(scene_addr, render_context);
    tdiff = rrq_ver_cmp(span, rrq_ver_terr, &first_t);

    /* run 4: asm, sprites ON, traced -> compare to C_full; stays on screen */
    rrq_ver_restore(render_context, span, &outs_pre);
    rrq_ver_suppress = 0;
    rrq_tr_live = 1; rrq_tr_phase = 1;
    doland_asm_run(scene_addr, render_context);
    rrq_tr_live = 0;
    fdiff = rrq_ver_cmp(span, rrq_ver_c, &first_f);

    /* Pixel watch: re-run the C terrain with the first mismatching byte watched; [rrqw] lines
       name each bar that changed it. Then redo the asm run so its result stays live. */
    if (tdiff && rrq_ver_bad < 4 && first_t != 0xffffffff) {
        extern u32 rrq_watch; extern u8 rrq_watch_val;
        u8 asm_px = image.wlogic[first_t];
        rrq_ver_restore(render_context, span, &outs_pre);
        rrq_ver_suppress = 1;
        rrq_watch = (u32)(image.wlogic + first_t - alis.mem);
        rrq_watch_val = alis.mem[rrq_watch];
        dbglog("[rrqw] #%u watch r%u c%u: C=%02x asm(full)=%02x start=%02x addr=%08lx calls@%08lx\n", rrq_ver_calls + 1,
               first_t / (u16)image.wloglarg, first_t % (u16)image.wloglarg, rrq_ver_terr[first_t], asm_px, rrq_watch_val,
               (unsigned long)(image.wlogic + first_t), (unsigned long)&rrq_ver_calls);
        doland_ref(scene_addr, render_context);
        rrq_watch = 0;
        /* row-by-row walk state of both renderers ([rrqrow] C / A) */
        extern u8 rrq_rowlog;
        rrq_ver_restore(render_context, span, &outs_pre);
        rrq_rowlog = 1;
        doland_ref(scene_addr, render_context);
        rrq_ver_restore(render_context, span, &outs_pre);
        rrq_rowlog = 2;
        doland_asm_run(scene_addr, render_context);
        rrq_rowlog = 0;
        rrq_ver_restore(render_context, span, &outs_pre);
        rrq_ver_suppress = 0;
        doland_asm_run(scene_addr, render_context);
    }
    rrq_ver_calls++;
#if defined(ALIS_DEBUG_VERIFY_FRAMES)
    if (rrq_ver_calls >= ALIS_DEBUG_VERIFY_FRAMES) {
        dbglog("[rrqver] done: %u frames, %u bad\n", rrq_ver_calls, rrq_ver_bad + (tdiff || fdiff));
        alis.state = eAlisStateStopped;
    }
#endif
    if (tdiff || fdiff) {
        rrq_ver_bad++;
        dbglog("[rrqver] #%u full=%u (row %u) TERRAIN=%u (row %u) sprite=%u win=[%d..%d]x[%d..%d]\n",
               rrq_ver_calls, fdiff,
               first_f == 0xffffffff ? 0 : first_f / (u16)image.wloglarg,
               tdiff,
               first_t == 0xffffffff ? 0 : first_t / (u16)image.wloglarg,
               (fdiff >= tdiff) ? fdiff - tdiff : 0,
               image.clipx1, image.clipx2, image.clipy1, image.clipy2);
        /* First mismatching frame: dump byte pattern (asm live in wlogic vs C_full)
           so the shape tells the bug type — stray col / dither phase / missing bar. */
        if (rrq_ver_bad == 1) {
            u16 pitch = (u16)image.wloglarg;
            u32 shown = 0;
            for (u32 i = 0; i < span && shown < 40; i++) {
                if (image.wlogic[i] != rrq_ver_c[i]) {
                    dbglog("[rrqpx] +%u r%u c%u asm=%02x C=%02x\n",
                           i, i / pitch, i % pitch, image.wlogic[i], rrq_ver_c[i]);
                    shown++;
                }
            }
        }
        /* call-trace diff on the full runs */
        u16 nc = rrq_tr_n[0], na = rrq_tr_n[1], i, lim = nc < na ? nc : na;
        for (i = 0; i < lim; i++)
            if (memcmp(&rrq_tr[0][i], &rrq_tr[1][i], sizeof(rrq_tr_ent)) != 0) break;
        if (nc != na || i < lim) {
            dbglog("[rrqtr] #%u calls C=%u asm=%u first-div=%u\n", rrq_ver_calls, nc, na, i);
            for (u16 j = (i > 1 ? i - 1 : 0); j < i + 2 && (j < nc || j < na); j++) {
                if (j < nc) dbglog("[rrqtr]   C  [%u] k=%d %d,%d,%d,%d\n", j, rrq_tr[0][j].kind,
                                   rrq_tr[0][j].a, rrq_tr[0][j].b, rrq_tr[0][j].c, rrq_tr[0][j].d);
                if (j < na) dbglog("[rrqtr]   ASM[%u] k=%d %d,%d,%d,%d\n", j, rrq_tr[1][j].kind,
                                   rrq_tr[1][j].a, rrq_tr[1][j].b, rrq_tr[1][j].c, rrq_tr[1][j].d);
            }
        }
    } else if (rrq_ver_calls <= 20 || (rrq_ver_calls & 0x3f) == 0) {
        dbglog("[rrqver] #%u OK (%u bad) span=%u\n", rrq_ver_calls, rrq_ver_bad, span);
    }
    return;
#else
    doland_asm_run(scene_addr, render_context);
#endif
}

static void doland_asm_run_body(doland_body_fn body, s32 scene_addr, s32 render_context)
{
    /* --- in-sync --- */
    doland_mirror.atlland    = alis.mem + image.atlland;
    doland_mirror.atlpix     = alis.mem + image.atlpix;
    doland_mirror.atalti     = alis.mem + image.atalti;
    doland_mirror.atalias    = alis.mem + image.atalias;
    doland_mirror.ptrdark    = alis.mem + alis.ptrdark;
    /* Sprites do NOT live in the VM arena in this port: record address =
       image.spritemem + image.basesprite + index (SPRITE_VAR). The original's
       basesprite global is exactly that base. */
    doland_mirror.basesprite = image.spritemem + image.basesprite;
    doland_mirror.atexsprite = image.spritemem + image.basesprite + image.atexsprite;
    doland_mirror.mapscreen  = alis.mem + scene_addr;
    doland_mirror.memfix     = (u32)alis.mem;

    /* Both source tables are base + i * stride, so (where, count, first, last) identifies them;
       rebuild only when that changes. */
    static u8 *pix_mem, *land_mem;
    static u32 pix_key[4], land_key[4];

    /* row y -> atlpix[y - wlogy1], the same entries the original indexed (even out of range) */
    u32 pk[4] = { image.atlpix, (u32)(s32)image.wlogy1, xread32(image.atlpix), xread32(image.atlpix + 199 * 4) };
    if (pix_mem != alis.mem || memcmp(pk, pix_key, sizeof(pk))) {
        for (s16 y = -64; y < 320; y++)
            doland_pixrows[y + 64] = alis.mem + xread32(image.atlpix + (s16)((y - image.wlogy1) * 4));
        memcpy(pix_key, pk, sizeof(pk));
        pix_mem = alis.mem;
    }

    /* in-bounds cells (x <= rc[-0x294]) read strips through this table */
    u16 n = (u16)xread16(render_context - 0x294);
    u32 lk[4] = { image.atlland, n, xread32(image.atlland), xread32(image.atlland + n * 4) };
    if (land_mem != alis.mem || memcmp(lk, land_key, sizeof(lk))) {
        for (u16 i = 0; i <= n; i++)
            doland_landtab[i] = alis.mem + xread32(image.atlland + i * 4);
        memcpy(land_key, lk, sizeof(lk));
        land_mem = alis.mem;
    }

    doland_mirror.clipx1     = image.clipx1;
    doland_mirror.clipy2     = image.clipy2;
    doland_mirror.landclipy1 = image.landclipy1;
    doland_mirror.landclipx2 = image.landclipx2;
    doland_mirror.landcliph  = image.landcliph;
    doland_mirror.vbarclipx2 = image.vbarclipx2;
    doland_mirror.wlogy1     = image.wlogy1;
    doland_mirror.wloglarg   = image.wloglarg;

    doland_mirror.vdarkw     = image.vdarkw;      /* verbatim; asm does its BE high-byte write */
    doland_mirror.precx      = image.precx;
    doland_mirror.spritprof  = (s16)image.spritprof;   /* original width is s16 */
    doland_mirror.spritnext  = (s16)image.spritnext;
    doland_mirror.ftstpix    = image.ftstpix;
    doland_mirror.is4bit     = (alis.platform.bpp == 4);   /* ST/Amiga shade lookup */
    doland_mirror.slowbar    = image.ftstpix || doland_mirror.is4bit || (s8)xread8(render_context - 0x400) == 0x0B;
    doland_mirror.xtstpix    = image.xtstpix;
    doland_mirror.ytstpix    = image.ytstpix;
    if (image.ftstpix) {
        /* Seed the hit-test result fields so a no-hit (or partial-hit) traversal
           emits the pointpix defaults (etstpix/ntstpix = -1, dtstpix = the input
           depth) rather than stale mirror values — the asm barlapix only WRITES
           these on an actual hit, exactly like C doland_68k. */
        doland_mirror.etstpix  = (s16)image.etstpix;
        doland_mirror.ntstpix  = (s16)image.ntstpix;
        doland_mirror.dtstpix  = (s16)image.dtstpix;
        doland_mirror.cxtstpix = (s16)image.cxtstpix;
        doland_mirror.cytstpix = (s16)image.cytstpix;
        doland_mirror.cztstpix = (s16)image.cztstpix;
    }

    /* --- run the lifted original --- */
    body(alis.mem + scene_addr, alis.mem + render_context);

    /* --- out-sync --- */
    image.precx     = doland_mirror.precx;
    image.solh      = doland_mirror.solh;
    image.solpixy   = doland_mirror.solpixy;
    image.toph      = doland_mirror.toph;
    image.toppixy   = doland_mirror.toppixy;
    image.vdarkw    = doland_mirror.vdarkw;
    image.spritprof = doland_mirror.spritprof;
    image.spritnext = doland_mirror.spritnext;
    image.vbarlarg  = doland_mirror.vbarlarg;
    image.vbarx     = doland_mirror.vbarx;
    image.vbarmid   = doland_mirror.vbarmid;
    image.vbarbot   = doland_mirror.vbarbot;
    if (image.ftstpix) {
        image.cxtstpix = doland_mirror.cxtstpix;
        image.cytstpix = doland_mirror.cytstpix;
        image.cztstpix = doland_mirror.cztstpix;
        image.dtstpix  = doland_mirror.dtstpix;
        image.ntstpix  = doland_mirror.ntstpix;
        image.etstpix  = doland_mirror.etstpix;
    }
}

/* --- thunk backends: called from the asm veneers with alis.mem-relative offsets --- */

void spritaff_thunk(s16 depth)
{
    /* C spritaff reads/writes image.spritprof/spritnext — resync both directions */
    image.spritprof = doland_mirror.spritprof;
    image.spritnext = doland_mirror.spritnext;
    /* Hit-test coherence: asm terrain probe writes doland_mirror, C sprite probe
       (zoomtofen) writes image.*. The original had ONE global set, last writer
       (nearest, far->near traversal) wins — keep both in lockstep so the final
       out-sync yields the nearest hit. antstpix is C-only, survives in image.*. */
    if (image.ftstpix) {
        image.etstpix  = (s16)doland_mirror.etstpix;
        image.dtstpix  = (s16)doland_mirror.dtstpix;
        image.ntstpix  = (s16)doland_mirror.ntstpix;
        image.cxtstpix = (s16)doland_mirror.cxtstpix;
        image.cytstpix = (s16)doland_mirror.cytstpix;
        image.cztstpix = (s16)doland_mirror.cztstpix;
    }
    spritaff(depth);
    if (image.ftstpix) {
        doland_mirror.etstpix  = (s16)image.etstpix;
        doland_mirror.dtstpix  = (s16)image.dtstpix;
        doland_mirror.ntstpix  = (s16)image.ntstpix;
        doland_mirror.cxtstpix = (s16)image.cxtstpix;
        doland_mirror.cytstpix = (s16)image.cytstpix;
        doland_mirror.cztstpix = (s16)image.cztstpix;
    }
    doland_mirror.spritprof = (s16)image.spritprof;
    doland_mirror.spritnext = (s16)image.spritnext;
}

void barsprite_thunk(s32 render_context, s16 type_idx, s16 world_x, s16 world_y)
{
    /* asm doland just wrote solh (dobar, orig 0x229ca); C barsprite reads
       image.solh. Nothing to sync back. Original 5th arg (zoom scale in D6) is
       recomputed by the C body; pass 0. */
    image.solh = doland_mirror.solh;
    barsprite(render_context, type_idx, world_x, world_y, 0);
}

/* --- Falcon CD textured fill: per-bar constant setup --------------------------
 * Called once per textured bar from Lbartra_tex_setup (blob_c). Mirrors the
 * setup of bartra_textured_68k; kept in C because of the three port-only
 * fixups (shift from width_mask, height 2^n snap, +8 image header). persp comes
 * from the pre-glandtopix rc[-0x27c] the asm saved in tex_saved_persp. */
void bartra_tex_setup(s32 render_context, s32 terrain_cell, s16 index,
                   u16 drawy, s16 barwidth, s16 bary)
{
    s16 max_cols = barwidth - 1;
    s16 row_offset = (s16)drawy - bary;

    /* darkness */
    u16 dark_val = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0)
                 + ((xread16(terrain_cell + 2) >> 8) & 0xc0)
                 + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
    u32 dark = (u32)dark_val;
    if ((s32)(dark << 0x10) < 0)
        dark = 0;

    /* texture resolve; tex_table_ptr is an alis.mem offset */
    u32 tex_table_ptr = xread32(render_context + index + 4);
    s32 tex_offset = (s32)xread32(tex_table_ptr);
    u16 width_mask = xread16(tex_table_ptr + tex_offset + 2);

    /* FIXUP 1: shift, derived from width_mask when the field is 0 */
    u8 shift = xread8(render_context + index) & 0x3f;
    if (shift == 0 && width_mask != 0) {
        u16 w = width_mask + 1;
        while (w > 1) { shift++; w >>= 1; }
    }

    /* persp = 0x200000 / divisor, ROL-packed (as in doland_68k) */
    u32 persp;
    {
        u32 pv = doland_mirror.tex_saved_persp;
        u32 divisor = (pv >> 8) & 0xFFFF;
        if (divisor > 0) {
            u16 quot = (u16)(0x200000 / divisor);
            persp = ((u32)(u8)(quot) << 24) | (u8)(quot >> 8);
            if (0x200000 % divisor != 0)
                persp = ((u32)(u8)(quot) << 24) | (u16)((u8)(quot >> 8) + 1);
        } else {
            persp = 0;
        }
    }

    u32 persp_unrolled = (persp << 8) | (persp >> 24);
    u32 v_step = concat22((s16)(persp >> 16), (s16)persp << shift);

    u16 clip_cols = (u16)(image.vbarlarg - max_cols);
    u32 h_step_raw = persp_unrolled * (u32)clip_cols * 0x10000;
    u32 h_step_per_line = (h_step_raw << 8) | (h_step_raw >> 24);

    u32 v_init_raw = persp_unrolled * (u32)(u16)row_offset * 0x10000;
    u32 V = concat22((s16)((v_init_raw << 8) >> 16),
                     (u16)(u8)(v_init_raw >> 24) << shift);

    /* FIXUP 2: height mask, snapped to 2^n-1 for non-power-of-2 heights */
    s16 type_v_base = xread16(render_context + index + 2);
    s16 hmask_raw = xread16(tex_table_ptr + tex_offset + 4) - type_v_base;
    if (hmask_raw > 0 && (hmask_raw & (hmask_raw + 1)) != 0) {
        u16 h = (u16)hmask_raw;
        h |= h >> 1; h |= h >> 2; h |= h >> 4; h |= h >> 8;
        hmask_raw = (s16)(h >> 1);
    }
    s16 height_mask = hmask_raw << shift;

    /* FIXUP 3: loaded resources use the 8-byte header (+8, not CD-native +6) */
    s32 tex_base = tex_table_ptr + ((s32)type_v_base << shift) + tex_offset + 8;

    doland_mirror.tex_persp = persp;
    doland_mirror.tex_vstep = v_step;
    doland_mirror.tex_hstep = h_step_per_line;
    doland_mirror.tex_v     = V;
    doland_mirror.tex_base  = alis.mem + tex_base;
    doland_mirror.tex_wmask = (s16)width_mask;
    doland_mirror.tex_hmask = height_mask;
    doland_mirror.tex_dark  = (s16)dark;
}

#endif /* ALIS_RRQ_ASM_DOLAND */
