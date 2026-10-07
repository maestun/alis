/*
 * render3d_atari.S — the ORIGINAL Robinson's Requiem Falcon renderer, lifted
 * from the Ghidra disassembly of the shipped interpreters. Contract with the C
 * side: render3d_atari_glue.h (mirror structs; offsets locked by _Static_asserts).
 *
 * zoomtofen — billboard sprite scaler (rrq-falcon-cd.asm): render body
 * 0x2373e-0x237f6 (normal) and 0x238a4-0x23920 (flipped), plus the setup/clip
 * path for 0x1c/0x1e (zoomtofen_full). The ftstpix hit-test stays in C.
 * Divergences from the original bytes:
 *   - absolute globals (wlogic/wlogy1/wloglarg/ztflowx/ztflowy) -> the
 *     zoomtofen_mirror mirror at (SYM(zoomtofen_mirror)+ZM_*).l  (same abs.l addressing mode).
 *   - the hardcoded `lea (0x8,a2,d1.l),a0` header offset is parameterized:
 *     `lea 0(a2,d1.l),a0` + `adda.w data_offset,a0`  (loaded resources use a
 *     6- or 8-byte header; the CD-native original always assumed 8).
 *
 * doland — terrain renderer core (rrq-falcon.asm): bars 0x2056c-0x225b7,
 * doland 0x225b8-0x22f9c. See tools/asm_lift/{entry_contract,externals,
 * conversion_notes}.md. Divergences from the original bytes:
 *   - absolute globals -> SYM(doland_mirror)+OFF (same abs.l addressing mode)
 *   - atlland/atlpix ENTRY dereferences get an `adda.l` memfix (our tables
 *     hold alis.mem offsets, the original held real pointers) — 12 sites
 *   - external callees (skytofen/glandtopix/spritaff/barsprite) go through
 *     register-preserving thunks to our C
 *   - initltra's 18 table immediates are symbolized (self-relocating tables)
 *
 * SECTION LAYOUT IS LOAD-BEARING: the four doland line tables + their unrolled
 * bodies must stay adjacent (16-bit self-relative offsets), and initltra WRITES
 * the tables at runtime (text-section write: fine on plain TOS/Falcon; would
 * fault under MiNT memory protection).
 *
 * Every original instruction keeps its 0x###### address as a line comment.
 */

#include "m68k_asm.h"

/* ========================================================================
 * zoomtofen
 * ======================================================================== */
#if defined(ALIS_ASM_PAD)
	.text
	.skip	ALIS_ASM_PAD		| placement experiments: shift all of this file's code
#endif

#if defined(ALIS_RRQ_ASM_ZOOM) && ALIS_RRQ_ASM_ZOOM

/* ---- zoomtofen_mirror mirror block (offsets = render3d_atari_glue.h, asserted in C) ---- */
	.equiv	ZM_WLOGIC,    0
	.equiv	ZM_ZTFLOWX,   4
	.equiv	ZM_ZTFLOWY,   8
	.equiv	ZM_BITMAP,   12
	.equiv	ZM_YSTEP,    16
	.equiv	ZM_XSTEP,    20
	.equiv	ZM_WLOGY1,   24
	.equiv	ZM_WLOGLARG, 26
	.equiv	ZM_SCREENX,  28
	.equiv	ZM_SCREENY,  30
	.equiv	ZM_NEWF,     32
	.equiv	ZM_CLIPW,    34
	.equiv	ZM_CLIPPEDH, 36
	.equiv	ZM_NEWY,     38
	.equiv	ZM_NEWX,     40
	.equiv	ZM_BMPW,     42
	.equiv	ZM_DATAOFF,  44
	/* --- zoomtofen_full_asm inputs --- */
	.equiv	ZM_MEM,      48
	.equiv	ZM_CLIPY1,   52
	.equiv	ZM_CLIPY2,   54
	.equiv	ZM_CLIPH,    56
	.equiv	ZM_CLIPX1,   58
	.equiv	ZM_CLIPX2,   60
	.equiv	ZM_CLIPL,    62
	.equiv	ZM_ROWS,     64
	.equiv	ZM_SIZE,     68

/* abs.l shorthands (same addressing mode as the original absolute globals) */
#define G_WLOGIC    (SYM(zoomtofen_mirror)+ZM_WLOGIC).l
#define G_ZTFLOWX   (SYM(zoomtofen_mirror)+ZM_ZTFLOWX).l
#define G_ZTFLOWY   (SYM(zoomtofen_mirror)+ZM_ZTFLOWY).l
#define G_WLOGY1    (SYM(zoomtofen_mirror)+ZM_WLOGY1).l
#define G_WLOGLARG  (SYM(zoomtofen_mirror)+ZM_WLOGLARG).l
#define G_DATAOFF   (SYM(zoomtofen_mirror)+ZM_DATAOFF).l
#define G_MEM       (SYM(zoomtofen_mirror)+ZM_MEM).l
#define G_CLIPY1    (SYM(zoomtofen_mirror)+ZM_CLIPY1).l
#define G_CLIPY2    (SYM(zoomtofen_mirror)+ZM_CLIPY2).l
#define G_CLIPH     (SYM(zoomtofen_mirror)+ZM_CLIPH).l
#define G_CLIPX1    (SYM(zoomtofen_mirror)+ZM_CLIPX1).l
#define G_CLIPX2    (SYM(zoomtofen_mirror)+ZM_CLIPX2).l
#define G_CLIPL     (SYM(zoomtofen_mirror)+ZM_CLIPL).l
#define G_ROWS      (SYM(zoomtofen_mirror)+ZM_ROWS).l
#define G_ZTFLOWX_LO (SYM(zoomtofen_mirror)+ZM_ZTFLOWX+2).l
#define G_ZTFLOWY_LO (SYM(zoomtofen_mirror)+ZM_ZTFLOWY+2).l

	.bss
	.even
	.globl	SYM(zoomtofen_mirror)
SYM(zoomtofen_mirror):
	.skip	ZM_SIZE

	.text
	.even

/* ---------------------------------------------------------------------------
 * C-ABI veneer: void zoomtofen_render_asm(void)
 * Loads the register contract from the zoomtofen_mirror mirror and runs the lifted
 * body. d0/d1/a0/a1 are caller-clobbered; d2-d7/a2-a6 are preserved.
 * ------------------------------------------------------------------------- */
	.globl	SYM(zoomtofen_render_asm)
SYM(zoomtofen_render_asm):
	movem.l	d2-d7/a2-a6,-(sp)	| C callee-saved
	move.w	(SYM(zoomtofen_mirror)+ZM_SCREENY).l,d0
	move.w	(SYM(zoomtofen_mirror)+ZM_SCREENX).l,d2
	move.w	(SYM(zoomtofen_mirror)+ZM_NEWF).l,d3
	move.l	(SYM(zoomtofen_mirror)+ZM_YSTEP).l,d4
	move.l	(SYM(zoomtofen_mirror)+ZM_XSTEP).l,d5
	move.w	(SYM(zoomtofen_mirror)+ZM_CLIPW).l,d6
	move.w	(SYM(zoomtofen_mirror)+ZM_CLIPPEDH).l,d7
	movea.l	(SYM(zoomtofen_mirror)+ZM_BITMAP).l,a2
	movea.w	(SYM(zoomtofen_mirror)+ZM_NEWY).l,a4
	movea.w	(SYM(zoomtofen_mirror)+ZM_BMPW).l,a5
	movea.w	(SYM(zoomtofen_mirror)+ZM_NEWX).l,a6
	bsr	zoomtofen_body
	movem.l	(sp)+,d2-d7/a2-a6
	rts

/* ===========================================================================
 * LIFTED SETUP + render.  void zoomtofen_full_asm(sSprite *sprite)
 *
 * Transcribes the ORIGINAL zoomtofen entry setup (0x236a8-0x23732) plus the
 * vertical-clip (0x23610-0x23654) and horizontal-clip (0x23658-0x2369c)
 * subblocks, then falls into the render body (zoomtofen_body == the
 * original 0x2373e code, entered with the exact register set live at 0x23734
 * when ftstpix == 0).  The ftstpix hit-test (0x23734 tst/beq) is NOT lifted —
 * the C wrapper only routes ftstpix==0 & format 0x1c/0x1e sprites here, so the
 * asm never sees the hit-test.
 *
 * Deliberate divergences from the original bytes (our sSprite layout differs):
 *   - newad @0x08 is a 24-bit bitfield PACKED with newf:8 @0x0b.  m68k BE puts
 *     newad in the high 24 bits, so `move.l (8,A0),D0; lsr.l #8,D0` yields
 *     newad (VERIFIED via m68k-atari-mint-gcc codegen — NOT and.l #0xffffff).
 *   - newad is an alis.mem OFFSET, so resolve via memfix: A2 = mem + newad,
 *     then A2 += (A2)  (== alis.mem + newad + xread32(newad), C render3d.c).
 *   - flip is our newf @0x0b (original read it from 0x30).
 *   - the clip globals clipy1/clipy2/cliph/clipx1/clipx2/clipl and alis.mem are
 *     threaded via the zoomtofen_mirror mirror (abs.l), same as the render body.
 * Every original instruction keeps its 0x###### address as a line comment.
 * ========================================================================= */
	.globl	SYM(zoomtofen_full_asm)
SYM(zoomtofen_full_asm):
	movem.l	d2-d7/a2-a6,-(sp)	| C callee-saved (11 regs, 44 bytes)
	movea.l	48(sp),a0		| a0 = sprite  (44 + 4 return addr)

| --- bitmap resolve + format check (orig 0x236ac-0x236c0) ---
	move.l	(0x8,a0),d0		| 0x236ac  (newad<<8)|newf   [BE bitfield]
	lsr.l	#8,d0			| [FIXUP]  d0 = newad (24-bit)
	movea.l	G_MEM,a2		| [FIXUP]  a2 = alis.mem
	adda.l	d0,a2			| [FIXUP]  a2 = mem + newad
	adda.l	(a2),a2			| 0x236b2  a2 = bitmap (mem+newad+xread32)
	cmpi.b	#0x1c,(a2)		| 0x236b4
	beq.w	Lzm_full_fmt_ok		| 0x236b8
	cmpi.b	#0x1e,(a2)		| 0x236bc
	bne.w	Lzm_full_ret		| 0x236c0  not 0x1c/0x1e -> draw nothing
Lzm_full_fmt_ok:				| 0x236c4
	move.w	(0x28,a0),d6		| 0x236c4  sprite->width
	addq.w	#1,d6			| 0x236c8  clipped_width = width+1
	move.w	(0x2a,a0),d7		| 0x236ca  sprite->height
	addq.w	#1,d7			| 0x236ce  clipped_height = height+1
	sub.l	d4,d4			| 0x236d0
	move.w	(0x4,a2),d4		| 0x236d2  bitmap height field
	addq.w	#1,d4			| 0x236d6  bitmap_height+1
	lsl.l	#8,d4			| 0x236d8  << 8
	divu.w	d7,d4			| 0x236da  / clipped_height -> zoom_y_scale
	ext.l	d4			| 0x236dc  (s16)quotient -> long
	lsl.l	#8,d4			| 0x236de  << 8
	swap	d4			| 0x236e0  d4 = zoom_y_step (8.8 packed)
	move.w	(0xe,a0),d0		| 0x236e2  sprite->newy
	movea.w	d0,a4			| 0x236e6  a4 = newy (unclipped)
	move.w	d0,d1			| 0x236e8
	sub.w	G_CLIPY1,d1		| 0x236ea  newy - clipy1
	sub.w	G_CLIPH,d1		| 0x236f0  - cliph  (= y_extent)
	bhi.w	Lzm_full_vclip		| 0x236f6  -> vertical clip
	neg.w	d1			| 0x236fa
	cmp.w	d7,d1			| 0x236fc
	bmi.w	Lzm_full_vclip		| 0x236fe  -> vertical clip
Lzm_full_hset:					| 0x23702
	move.w	(0xc,a0),d2		| 0x23702  sprite->newx (screen_x)
	movea.w	d2,a6			| 0x23706  a6 = newx (unclipped)
	move.w	d2,d1			| 0x23708
	sub.w	G_CLIPX1,d1		| 0x2370a  newx - clipx1
	sub.w	G_CLIPL,d1		| 0x23710  - clipl  (= x_extent)
	bhi.w	Lzm_full_hclip		| 0x23716  -> horizontal clip
	neg.w	d1			| 0x2371a
	cmp.w	d6,d1			| 0x2371c
	bmi.w	Lzm_full_hclip		| 0x2371e  -> horizontal clip
Lzm_full_xstep:					| 0x23722
	move.w	(0x2,a2),d1		| 0x23722  bitmap width field
	addq.w	#1,d1			| 0x23726  bitmap_width
	movea.w	d1,a5			| 0x23728  a5 = bitmap_width
	move.l	d4,d5			| 0x2372a
	mulu.w	d4,d1			| 0x2372c  bitmap_width * (zoom_y_step lo)
	move.w	d1,d5			| 0x2372e  d5 = zoom_x_step
	move.b	(0xb,a0),d3		| [FIXUP]  flip = newf (orig 0x30)
	| register set now == original 0x23734 (ftstpix==0): run render body
	bsr	zoomtofen_body		| 0x2373e..0x237f6 / 0x238a4..0x23920
Lzm_full_ret:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

| --- VERTICAL CLIP (orig 0x23610-0x23654), returns to Lzm_full_hset ---
Lzm_full_vclip:					| 0x23610
	cmp.w	G_CLIPY2,d0		| 0x23610
	bgt.w	Lzm_full_ret		| 0x23616  newy > clipy2 -> nothing
	move.w	d7,d1			| 0x2361a
	add.w	d0,d1			| 0x2361c
	subq.w	#1,d1			| 0x2361e  y_extent = newy+clipped_height-1
	cmp.w	G_CLIPY1,d1		| 0x23620
	blt.w	Lzm_full_ret		| 0x23626  bottom < clipy1 -> nothing
	cmp.w	G_CLIPY1,d0		| 0x2362a
	bge.w	Lzm_full_vc1		| 0x23630
	sub.w	G_CLIPY1,d7		| 0x23634
	add.w	d0,d7			| 0x2363a  clipped_height top-clipped
	move.w	G_CLIPY1,d0		| 0x2363c  screen_y = clipy1
Lzm_full_vc1:					| 0x23642
	cmp.w	G_CLIPY2,d1		| 0x23642
	ble.w	Lzm_full_hset		| 0x23648  -> 0x23702
	sub.w	d1,d7			| 0x2364c
	add.w	G_CLIPY2,d7		| 0x2364e  clipped_height bottom-clipped
	bra.w	Lzm_full_hset		| 0x23654  -> 0x23702

| --- HORIZONTAL CLIP (orig 0x23658-0x2369c), returns to Lzm_full_xstep ---
Lzm_full_hclip:					| 0x23658
	cmp.w	G_CLIPX2,d2		| 0x23658
	bgt.w	Lzm_full_ret		| 0x2365e  newx > clipx2 -> nothing
	move.w	d6,d1			| 0x23662
	add.w	d2,d1			| 0x23664
	subq.w	#1,d1			| 0x23666  x_extent = newx+clipped_width-1
	cmp.w	G_CLIPX1,d1		| 0x23668
	blt.w	Lzm_full_ret		| 0x2366e  right < clipx1 -> nothing
	cmp.w	G_CLIPX1,d2		| 0x23672
	bge.w	Lzm_full_hc1		| 0x23678
	sub.w	G_CLIPX1,d6		| 0x2367c
	add.w	d2,d6			| 0x23682  clipped_width left-clipped
	move.w	G_CLIPX1,d2		| 0x23684  screen_x = clipx1
Lzm_full_hc1:					| 0x2368a
	cmp.w	G_CLIPX2,d1		| 0x2368a
	ble.w	Lzm_full_xstep		| 0x23690  -> 0x23722
	sub.w	d1,d6			| 0x23694
	add.w	G_CLIPX2,d6		| 0x23696  clipped_width right-clipped
	bra.w	Lzm_full_xstep		| 0x2369c  -> 0x23722

/* ===========================================================================
 * LIFTED BODY — original zoomtofen render path (ftstpix == 0).
 * Entry register set (== original at 0x23734, hit-test branch NOT taken):
 *   d0=screen_y d2=screen_x d3=newf d4=zoom_y_step d5=zoom_x_step
 *   d6=clipped_width d7=clipped_height a2=bitmap a4=newy a5=bitmap_width
 *   a6=newx
 * ========================================================================= */
zoomtofen_body:
| --- shared framebuffer + texture_y setup (orig 0x2373e-0x2377a) ---
	movea.l	G_WLOGIC,a1		| 0x2373e
	move.w	d0,d1			| 0x23744
	sub.w	G_WLOGY1,d1		| 0x23746
	mulu.w	G_WLOGLARG,d1		| 0x2374c
	adda.l	d1,a1			| 0x23752
	adda.w	d2,a1			| 0x23754
	clr.w	G_ZTFLOWY		| 0x23756  (ztflowy &= 0xffff, BE high word)
	sub.l	d1,d1			| 0x2375c
	move.w	d0,d1			| 0x2375e
	sub.w	a4,d1			| 0x23760  d1 = screen_y - newy (y_offset)
	beq.w	Lzm_shared_done		| 0x23762 -> 0x2377c
	move.l	d4,d0			| 0x23766
	rol.l	#8,d0			| 0x23768
	mulu.w	d0,d1			| 0x2376a
	move.b	d1,G_ZTFLOWY		| 0x2376c  (BE high byte of ztflowy)
	lsr.l	#8,d1			| 0x23772
	move.w	a5,d0			| 0x23774  width+1 (orig: (2,a2)+1; a5 is endian-safe for DOS data)
	mulu.w	d0,d1			| 0x2377a  d1 = texture_y
Lzm_shared_done:				| 0x2377c
#if !defined(ALIS_NO_ZOOM_TRIM)
	move.l	d1,Lzr_ty		| first texture row * width
#endif
	btst	#0,d3			| 0x2377c  flip?
	bne.w	Lzm_flip		| 0x23780 -> 0x238a4

| --- NORMAL render (orig 0x23784-0x237f6) ---
	clr.w	G_ZTFLOWX		| 0x23784  (ztflowx &= 0xffff)
	sub.w	a6,d2			| 0x2378a  d2 = screen_x - newx
	beq.w	Lzm_n_lea		| 0x2378c -> 0x237a0
	move.l	d4,d0			| 0x23790
	rol.l	#8,d0			| 0x23792
	mulu.w	d0,d2			| 0x23794
	move.b	d2,G_ZTFLOWX		| 0x23796  (BE high byte of ztflowx)
	lsr.l	#8,d2			| 0x2379c
	add.l	d2,d1			| 0x2379e  texture_y += (d2>>8)
Lzm_n_lea:					| 0x237a0
	lea	0(a2,d1.l),a0		| 0x237a0  a0 = bitmap + texture_y ...
	move.w	G_DATAOFF,d0		|          ... + data_offset  (orig lea +8)
	adda.w	d0,a0			|
	movea.w	G_WLOGLARG,a4		| 0x237a4
	suba.w	d6,a4			| 0x237aa  a4 = wloglarg - clipped_width (line_skip)
	subq.w	#1,d6			| 0x237ac  clipped_width - 1
	subq.w	#1,d7			| 0x237ae  clipped_height - 1
	movea.w	d6,a6			| 0x237b0  saved row pixel count
	sub.l	d3,d3			| 0x237b2  d3 = 0 (addx zero source)
	move.l	G_ZTFLOWY,d1		| 0x237b4
#if !defined(ALIS_NO_ZOOM_TRIM)
	jbsr	zt_trim_n
	tst.w	d0
	jbeq	Lzt_n_go
#endif
Lzm_n_row:					| 0x237ba
	move.l	G_ZTFLOWX,d0		| 0x237ba
	move.w	d1,d0			| 0x237c0  d0 = [ztflowx hi][ztflowy lo]  REG-ALIAS
	move.w	a6,d6			| 0x237c2
Lzm_n_px:					| 0x237c4
	move.b	(0,a0,d0.w),d2		| 0x237c4
	beq.w	Lzm_n_skip		| 0x237c8 -> 0x237da
	move.b	d2,(a1)+		| 0x237cc
	add.l	d4,d0			| 0x237ce
	addx.w	d3,d0			| 0x237d0
	dbf	d6,Lzm_n_px		| 0x237d2
	bra.w	Lzm_n_rowend		| 0x237d6 -> 0x237e4
Lzm_n_skip:					| 0x237da
	addq.w	#1,a1			| 0x237da  transparent: advance dst, still step acc
	add.l	d4,d0			| 0x237dc
	addx.w	d3,d0			| 0x237de
	dbf	d6,Lzm_n_px		| 0x237e0
Lzm_n_rowend:				| 0x237e4
	add.l	d5,d1			| 0x237e4
	bcc.w	Lzm_n_nocarry		| 0x237e6
	add.w	a5,d1			| 0x237ea  += bitmap_width on fractional wrap
Lzm_n_nocarry:				| 0x237ec
	adda.w	a4,a1			| 0x237ec  framebuffer += line_skip
	dbf	d7,Lzm_n_row		| 0x237ee
	rts				| (orig 0x237f2 movem/rts; veneer restores)

| --- FLIPPED render (orig 0x238a4-0x23920) ---
Lzm_flip:					| 0x238a4
	sub.l	d3,d3			| 0x238a4
	move.w	a5,d3			| 0x238a6  raw width (orig: (2,a2); a5-1 is endian-safe for DOS data)
	subq.w	#1,d3			|
	add.l	d3,d1			| 0x238aa  bitmap_row_offset = raw_w + texture_y
	move.w	#-1,G_ZTFLOWX		| 0x238ac  ztflowx hi word = 0xffff
	sub.w	a6,d2			| 0x238b4  d2 = screen_x - newx
	beq.w	Lzm_f_lea		| 0x238b6 -> 0x238ca
	move.l	d4,d0			| 0x238ba
	rol.l	#8,d0			| 0x238bc
	mulu.w	d0,d2			| 0x238be
	sub.b	d2,G_ZTFLOWX		| 0x238c0  (BE high byte of ztflowx)
	lsr.l	#8,d2			| 0x238c6
	sub.l	d2,d1			| 0x238c8  bitmap_row_offset -= (d2>>8)
Lzm_f_lea:					| 0x238ca
	lea	0(a2,d1.l),a0		| 0x238ca  a0 = bitmap + row_offset ...
	move.w	G_DATAOFF,d0		|          ... + data_offset  (orig lea +8)
	adda.w	d0,a0			|
	movea.w	G_WLOGLARG,a4		| 0x238ce
	suba.w	d6,a4			| 0x238d4
	subq.w	#1,d6			| 0x238d6
	subq.w	#1,d7			| 0x238d8
	movea.w	d6,a6			| 0x238da
	sub.l	d3,d3			| 0x238dc
	move.l	G_ZTFLOWY,d1		| 0x238de
#if !defined(ALIS_NO_ZOOM_TRIM)
	jbsr	zt_trim_f
	tst.w	d0
	jbeq	Lzt_f_go
#endif
Lzm_f_row:					| 0x238e4
	move.l	G_ZTFLOWX,d0		| 0x238e4
	move.w	d1,d0			| 0x238ea  REG-ALIAS
	move.w	a6,d6			| 0x238ec
Lzm_f_px:					| 0x238ee
	move.b	(0,a0,d0.w),d2		| 0x238ee
	beq.w	Lzm_f_skip		| 0x238f2 -> 0x23904
	move.b	d2,(a1)+		| 0x238f6
	sub.l	d4,d0			| 0x238f8  backward step
	subx.w	d3,d0			| 0x238fa
	dbf	d6,Lzm_f_px		| 0x238fc
	bra.w	Lzm_f_rowend		| 0x23900 -> 0x2390e
Lzm_f_skip:					| 0x23904
	addq.w	#1,a1			| 0x23904
	sub.l	d4,d0			| 0x23906
	subx.w	d3,d0			| 0x23908
	dbf	d6,Lzm_f_px		| 0x2390a
Lzm_f_rowend:				| 0x2390e
	add.l	d5,d1			| 0x2390e
	bcc.w	Lzm_f_nocarry		| 0x23910
	add.w	a5,d1			| 0x23914
Lzm_f_nocarry:				| 0x23916
	adda.w	a4,a1			| 0x23916
	dbf	d7,Lzm_f_row		| 0x23918
	rts				| (orig 0x2391c movem/rts; veneer restores)


#if !defined(ALIS_NO_ZOOM_TRIM)
/* ---------------------------------------------------------------------------
 * Row trimming: draw only the opaque span of each row (spans from render3d.c zoom_rows).
 * The texture step is the same for every row, so one pass gives the accumulator at each
 * screen pixel (D) and, per screen row, the first pixel and count-1 of its span (T, -1 =
 * nothing). The trimmed loops start each row from D and run the original pixel loop,
 * so the output is unchanged. zt_trim_n/_f: d0 = 0 when the tables are ready, else the
 * plain loop runs. Preserve every other register.
 * In: d1 ztflowy, d2.w texture x offset, d4/d5 steps, d6/d7 w-1/h-1, a5 bitmap width.
 * ------------------------------------------------------------------------- */
#ifndef ALIS_ZOOM_TRIM_MINW
#define ALIS_ZOOM_TRIM_MINW 16
#endif
	.equ	ZT_MAXW, 512
	.equ	ZT_MAXH, 256

	.bss
	.even
Lzr_ty:	.skip	4		| first texture row * width
Lzr_r0:	.skip	2		| first texture row
Lzr_xn:	.skip	2		| largest column offset reached + 1
Lzr_bh:	.skip	2		| bitmap rows
Lzr_iy:	.skip	2		| whole texture rows per screen row
Lzr_K:	.skip	(ZT_MAXW+1)*2	| K[c] = first pixel whose column offset >= c
Lzr_D:	.skip	ZT_MAXW*4	| accumulator at each pixel (offset part relative to the row)
Lzr_T:	.skip	ZT_MAXH*4	| per row: first pixel, count-1
	.text
	.even

| Stack after the movem: d1 0, d2 4, d3 8, d4 12, d5 16, d6 20, d7 24 (.w at +2).
.macro ZT_TRIM flip
	movem.l	d1-d7/a0-a6,-(sp)
	movea.l	G_ROWS,a3
	move.l	a3,d0
	jbeq	9f
	cmp.w	#ALIS_ZOOM_TRIM_MINW-1,d6	| narrow sprites: not worth it
	jblt	9f
	cmp.w	#ZT_MAXW-1,d6
	jbhi	9f
	cmp.w	#ZT_MAXH-1,d7
	jbhi	9f
	tst.w	G_ZTFLOWX_LO		| the tables assume both offset words start at 0
	jbne	9f
	tst.w	G_ZTFLOWY_LO
	jbne	9f
	move.w	a5,d3			| bitmap width
	cmp.w	#ZT_MAXW,d3
	jbhi	9f
	move.l	-4(a3),d0		| bitmap rows
	move.w	d0,Lzr_bh
	move.l	Lzr_ty,d0
	divu.w	d3,d0
	jbvs	9f
	move.w	d0,Lzr_r0

	move.l	G_ZTFLOWX,d0
.if \flip
	move.w	d3,d0			| start at offset = width: no borrow from the fraction
.endif
	lea	Lzr_D,a4
	lea	Lzr_K,a6
	moveq	#0,d5			| next column offset to resolve
	moveq	#0,d6			| pixel
	moveq	#0,d1
	move.w	22(sp),d7
1:
.if \flip
	move.l	d0,d2
	sub.w	d3,d2
	move.l	d2,(a4)+
	move.w	d3,d2
	sub.w	d0,d2			| column offset, counted leftwards
.else
	move.l	d0,(a4)+
	move.w	d0,d2
.endif
	cmp.w	#ZT_MAXW-1,d2
	jbhi	9f			| column offset beyond the K table: plain loop
2:	cmp.w	d2,d5
	jbgt	3f
	move.w	d6,0(a6,d5.w*2)
	addq.w	#1,d5
	jbra	2b
3:	addq.w	#1,d6
.if \flip
	sub.l	d4,d0
	subx.w	d1,d0
.else
	add.l	d4,d0
	addx.w	d1,d0
.endif
	dbf	d7,1b
	move.w	d6,0(a6,d5.w*2)
	move.w	d5,Lzr_xn

	move.w	6(sp),d2		| texture x offset
.if \flip
	neg.w	d2
	add.w	d3,d2
	subq.w	#1,d2			| column of pixel 0
	move.w	d2,d0
	sub.w	d5,d0
	addq.w	#1,d0
	jbmi	9f			| would sample past the row start
.else
	move.w	d2,d0
	add.w	d5,d0
	cmp.w	d3,d0
	jbgt	9f			| would sample past the row end
.endif

	move.l	16(sp),d5		| row step
	moveq	#0,d0
	move.w	d5,d0
	divu.w	d3,d0			| exact: whole rows * width
	move.w	d0,Lzr_iy
	lsl.w	#2,d0
	movea.w	d0,a2
	moveq	#0,d6
	move.w	Lzr_r0,d6
	move.l	d6,d0
	lsl.l	#2,d0
	adda.l	d0,a3
	move.l	0(sp),d0		| row accumulator, as the render loop
	lea	Lzr_T,a4
	move.w	26(sp),d7
4:	cmp.w	Lzr_bh,d6
	jbcc	9f			| texture row outside the bitmap
	move.w	d0,d1
	jbmi	9f			| offsets must stay below 0x8000: no sign or carry effects
	add.w	d3,d1
	jbvs	9f
.if \flip
	cmp.w	d3,d0
	jbcs	7f			| the first texture row borrows: draw it whole
.endif
	move.l	(a3),d1
	jbmi	6f			| empty row
	move.w	d1,d4
	swap	d1
.if \flip
	neg.w	d4
	add.w	d2,d4
	neg.w	d1
	add.w	d2,d1
	addq.w	#1,d1
	exg	d1,d4
.else
	sub.w	d2,d1
	sub.w	d2,d4
	addq.w	#1,d4
.endif
	tst.w	d1
	jbpl	1f
	moveq	#0,d1
1:	cmp.w	Lzr_xn,d1
	jble	2f
	move.w	Lzr_xn,d1
2:	move.w	0(a6,d1.w*2),d1		| first pixel
	tst.w	d4
	jbpl	3f
	moveq	#0,d4
3:	cmp.w	Lzr_xn,d4
	jble	5f
	move.w	Lzr_xn,d4
5:	move.w	0(a6,d4.w*2),d4
	sub.w	d1,d4
	subq.w	#1,d4
	jbmi	6f
	move.w	d1,(a4)+
	move.w	d4,(a4)+
	jbra	8f
6:	move.l	#-1,(a4)+
.if \flip
	jbra	8f
7:	clr.w	(a4)+
	move.w	22(sp),(a4)+
.endif
8:	add.l	d5,d0
	jbcc	1f
	add.w	d3,d0
	addq.w	#1,d6
	addq.l	#4,a3
1:	add.w	Lzr_iy,d6
	adda.w	a2,a3
	dbf	d7,4b
	moveq	#0,d0
	jbra	1f
9:	moveq	#1,d0
1:	movem.l	(sp)+,d1-d7/a0-a6
	rts
.endm

zt_trim_n:
	ZT_TRIM 0
zt_trim_f:
	ZT_TRIM 1

.macro ZT_LOOP tag, step, stepx
Lzt_\tag\()_go:
	lea	Lzr_T,a3
	lea	Lzr_D,a4
	movea.w	G_WLOGLARG,a6
Lzt_\tag\()_row:
	move.l	(a3)+,d2
	jbmi	Lzt_\tag\()_next
	move.w	d2,d6
	swap	d2
	lea	0(a1,d2.w),a2
	move.l	0(a4,d2.w*4),d0
	add.w	d1,d0
Lzt_\tag\()_px:
	move.b	(0,a0,d0.w),d2
	beq.s	Lzt_\tag\()_skip
	move.b	d2,(a2)+
	\step	d4,d0
	\stepx	d3,d0
	dbf	d6,Lzt_\tag\()_px
	bra.s	Lzt_\tag\()_next
Lzt_\tag\()_skip:
	addq.w	#1,a2
	\step	d4,d0
	\stepx	d3,d0
	dbf	d6,Lzt_\tag\()_px
Lzt_\tag\()_next:
	add.l	d5,d1
	bcc.s	1f
	add.w	a5,d1
1:	adda.w	a6,a1
	dbf	d7,Lzt_\tag\()_row
	rts
.endm

	ZT_LOOP n, add.l, addx.w
	ZT_LOOP f, sub.l, subx.w
#endif /* ALIS_NO_ZOOM_TRIM */

#undef G_WLOGIC
#undef G_ZTFLOWX
#undef G_ZTFLOWY
#undef G_WLOGY1
#undef G_WLOGLARG
#undef G_DATAOFF
#undef G_MEM
#undef G_CLIPY1
#undef G_CLIPY2
#undef G_CLIPH
#undef G_CLIPX1
#undef G_CLIPX2
#undef G_CLIPL

#endif /* ALIS_RRQ_ASM_ZOOM */

/* ========================================================================
 * doland
 * ======================================================================== */

#if defined(ALIS_RRQ_ASM_DOLAND) && ALIS_RRQ_ASM_DOLAND

/* ---- doland_mirror mirror block (offsets = render3d_atari_glue.h, asserted in glue.c) ---- */
	.equiv	DL_ATLLAND,     0
	.equiv	DL_ATLPIX,      4
	.equiv	DL_ATALTI,      8
	.equiv	DL_ATALIAS,    12
	.equiv	DL_PTRDARK,    16
	.equiv	DL_BASESPRITE, 20
	.equiv	DL_ATEXSPRITE, 24
	.equiv	DL_MAPSCREEN,  28
	.equiv	DL_MEMFIX,     32
	.equiv	DL_CLIPX1,     36
	.equiv	DL_CLIPY2,     38
	.equiv	DL_LANDCLIPY1, 40
	.equiv	DL_LANDCLIPX2, 42
	.equiv	DL_LANDCLIPH,  44
	.equiv	DL_VBARCLIPX2, 46
	.equiv	DL_WLOGY1,     48
	.equiv	DL_WLOGLARG,   50
	.equiv	DL_VDARKW,     52
	.equiv	DL_PRECX,      54
	.equiv	DL_SPRITPROF,  56
	.equiv	DL_SPRITNEXT,  58
	.equiv	DL_SOLH,       60
	.equiv	DL_SOLPIXY,    62
	.equiv	DL_TOPH,       64
	.equiv	DL_TOPPIXY,    66
	.equiv	DL_VBARLARG,   68
	.equiv	DL_VBARX,      70
	.equiv	DL_VBARMID,    72
	.equiv	DL_VBARBOT,    74
	.equiv	DL_FTSTPIX,    76
	.equiv	DL_XTSTPIX,    78
	.equiv	DL_YTSTPIX,    80
	.equiv	DL_CXTSTPIX,   82
	.equiv	DL_CYTSTPIX,   84
	.equiv	DL_CZTSTPIX,   86
	.equiv	DL_DTSTPIX,    88
	.equiv	DL_NTSTPIX,    90
	.equiv	DL_ETSTPIX,    92
	.equiv	DL_IS4BIT,     94
	.equiv	DL_TEX_SAVED_PERSP, 96
	.equiv	DL_TEX_PERSP,  100
	.equiv	DL_TEX_VSTEP,  104
	.equiv	DL_TEX_HSTEP,  108
	.equiv	DL_TEX_V,      112
	.equiv	DL_TEX_BASE,   116
	.equiv	DL_TEX_WMASK,  120
	.equiv	DL_TEX_HMASK,  122
	.equiv	DL_TEX_DARK,   124
	.equiv	DL_SLOWBAR,    126
	.equiv	DL_SIZE,       128

	.bss
	.even
	.globl	SYM(doland_mirror)
SYM(doland_mirror):
	.skip	DL_SIZE
| Real-pointer copy of the atlland strip table, rebuilt per call by the glue.
| Its address is doland's a2 base: the B_* globals are short d16(a2) offsets from it.
	.equiv	DL_LANDTAB_MAX, 2048
	.equiv	LRRQ_VARS, DL_LANDTAB_MAX*4	| original 0x22716 data block (Lrrq_*, blob A)
	.globl	SYM(doland_landtab)
SYM(doland_landtab):
	.skip	LRRQ_VARS+30
| Real pointers to screen rows y = -64..319 (doland_pixtab[y]), rebuilt per call by the glue.
	.even
	.globl	SYM(doland_pixrows)
SYM(doland_pixrows):			| C view: doland_pixrows[y + 64]
	.skip	64*4
SYM(doland_pixtab):			| asm view: doland_pixtab[y]
	.skip	320*4

	.text
	.even

/* ---------------------------------------------------------------------------
 * C-ABI veneer: void doland_body_asm(u8 *scene_ptr, u8 *rc_ptr)
 * Loads A2/A3 (the original's calling convention) and runs the lifted doland.
 * ------------------------------------------------------------------------- */
	.globl	SYM(doland_body_asm)
SYM(doland_body_asm):
	movem.l	d2-d7/a2-a6,-(sp)	| C callee-saved (11 regs, 44 bytes)
	movea.l	48(sp),a2		| scene ptr   (44 + 4 return addr)
	movea.l	52(sp),a3		| rc ptr
	bsr	rrq_doland		| the lifted original (saves/restores all)
	movem.l	(sp)+,d2-d7/a2-a6
	rts

/* ---------------------------------------------------------------------------
 * void initltra_asm(void) — one-time table build (lifted initltra)
 * ------------------------------------------------------------------------- */
	.globl	SYM(initltra_asm)
SYM(initltra_asm):
	movem.l	d0-d7/a0-a6,-(sp)
	bsr	rrq_initltra
	movem.l	(sp)+,d0-d7/a0-a6
	rts

/* ===========================================================================
 * LIFTED CODE
 * Blob A: bars + initltra + line tables/bodies.
 * Blob C: Falcon CD textured (0x0B) fill.
 * Blob B: thunk veneers + doland + dotop (references Blob A's
 * barland/tbarland labels and the Lrrq_* data block).
 * ========================================================================= */

#include "../tools/asm_lift/blob_a_bars.inc.S"
#include "../tools/asm_lift/blob_c_textured.inc.S"
#include "../tools/asm_lift/blob_b_doland.inc.S"

#endif /* ALIS_RRQ_ASM_DOLAND */
