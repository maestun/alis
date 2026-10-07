/*
 * image_atari.S — m68k planar sprite blitters for image.c / sprite_planar.c /
 * image_draw_planar.c.
 *
 * destofen: faithful port of the original ALIS Falcon sprite blitter
 * (Robinson's Requiem CD, destofen @ rrq-falcon-cd.asm 0x0f182), transcribed
 * "as literally as possible, fixing only addressing" — see Docs/destofen-port.md.
 * The C wrapper (sprite_planar.c) fills the parameter block P and supplies
 * src/dst, matching the original dispatcher's entry state.
 *
 * Pixel format (both src sprite + dest framebuffer): Atari word-interleaved planar,
 * 16px chunks = 8 plane-words [p0..p7] (16 bytes), ascending address. Transparency =
 * colour 0: mask = NOT(OR of all 8 planes). Every drawn chunk is read-modify-write.
 *
 * cdestofen span: converts up to 16 chunky 8-bit pixels into one 8-plane chunk
 * (image_draw_planar.c inner loop).
 */

#include "m68k_asm.h"

	.text

/*
 * void destofen_8bpp_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   P->nchunks(+0) nlines(+2) dst_rowrem(+4, BYTES) src_hskip(+6, chunks) src_start(+8, chunks)
 *   src/dst already point at the first VISIBLE chunk (src_start kept 0 by the wrapper).
 * After movem.l d2-d7/a2-a6 (11 regs = 44 bytes): P@48(sp) src@52(sp) dst@56(sp).
 */
	.globl	SYM(destofen_8bpp_aligned_asm)
SYM(destofen_8bpp_aligned_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src (first visible chunk)
	move.l	56(sp),a3		| a3 = dst (first visible chunk)

	| --- transcribed from 0x10d2a (flip already excluded by the wrapper) ---
	move.w	6(a1),d0
	lsl.w	#4,d0
	movea.w	d0,a0			| a0 = src_hskip * 16 (bytes skipped per row)
	move.w	2(a1),d5		| d5 = nlines (row dbf counter)
	move.w	8(a1),d4
	ext.l	d4
	lsl.l	#4,d4
	adda.l	d4,a2			| a2 += src_start * 16  (wrapper keeps this 0)
	movea.w	4(a1),a6		| a6 = dst_rowrem (bytes added to dst between rows)
	movea.w	(a1),a4
	subq.w	#1,a4			| a4 = nchunks - 1

.Lrow8u:
	move.w	a4,d4			| d4 = chunk dbf counter
.Lcol8u:
	movem.l	(a2)+,d0-d3		| one 8-plane chunk: d0=[p0:p1] d1=[p2:p3] d2=[p4:p5] d3=[p6:p7]
	move.l	d0,d7
	or.l	d1,d7
	or.l	d2,d7
	or.l	d3,d7			| d7 = OR of all planes (2 packed words)
	move.l	d7,d6
	swap	d7
	or.l	d6,d7			| fold halves: both words = per-pixel "any plane set"
	beq.s	.Lcu_skip		| union==0: fully transparent chunk — no dst traffic at all
	not.l	d7			| d7 = ~union = transparency mask (both words)
	beq.s	.Lcu_opq		| mask==0 (union was all-ones): fully opaque chunk
.Lcu_rmw:
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d0,d6
	move.l	d6,(a3)+		| RMW planes 0:1
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d1,d6
	move.l	d6,(a3)+		| RMW planes 2:3   (a3 +8)
	cmpi.w	#4,18(a1)
	beq.s	.Lcu_skip2		| nplanes==4 -> skip high planes, advance to chunk end
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d2,d6
	move.l	d6,(a3)+		| RMW planes 4:5   (a3 +12)
	cmpi.w	#6,18(a1)
	beq.s	.Lcu_skip3		| nplanes==6
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d3,d6
	move.l	d6,(a3)+		| RMW planes 6:7   (a3 +16)
	bra.s	.Lcu_next
.Lcu_opq:
	cmpi.w	#8,18(a1)		| reduced-plane sprite: RMW path is already a pure
	bne.s	.Lcu_rmw		|   store when d7==0 and must not touch high planes
	movem.l	d0-d3,(a3)		| opaque 8-plane chunk: burst store, no dst read
.Lcu_skip:
	lea	16(a3),a3		| bit-exact: skipped chunk wrote dst back unchanged,
	bra.s	.Lcu_next		|   opaque chunk wrote (dst & 0) | src = src
.Lcu_skip3:
	addq.l	#4,a3			| skip planes 6:7
	bra.s	.Lcu_next
.Lcu_skip2:
	addq.l	#8,a3			| skip planes 4:5,6:7
.Lcu_next:
	dbf	d4,.Lcol8u
	adda.w	a0,a2			| next row: skip clipped-off source chunks
	adda.w	a6,a3			| next row: dest row remainder
	dbf	d5,.Lrow8u

	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_8bpp_opaque_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   OPAQUE word-aligned: derived from destofen_8bpp_aligned_asm by FORCING the transparency mask
 *   to all-ones — i.e. write every source plane directly (index-0 = black IS drawn), no union, no
 *   RMW. The wrapper only routes sprites whose width is a multiple of 16 (no zero-padding chunk to
 *   draw black past the edge) and that lie fully within the viewport. nplanes-skip preserved.
 */
	.globl	SYM(destofen_8bpp_opaque_aligned_asm)
SYM(destofen_8bpp_opaque_aligned_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src
	move.l	56(sp),a3		| a3 = dst
	move.w	6(a1),d0
	lsl.w	#4,d0
	movea.w	d0,a0			| a0 = src_hskip * 16
	move.w	2(a1),d5		| d5 = nlines
	move.w	8(a1),d4
	ext.l	d4
	lsl.l	#4,d4
	adda.l	d4,a2			| a2 += src_start * 16
	movea.w	4(a1),a6		| a6 = dst_rowrem
	movea.w	(a1),a4
	subq.w	#1,a4			| a4 = nchunks - 1
.Loru_row:
	move.w	a4,d4
.Loru_col:
	movem.l	(a2)+,d0-d3		| full 8-plane src chunk (a2 += 16)
	move.l	d0,(a3)+		| planes 0:1 (direct write)
	move.l	d1,(a3)+		| planes 2:3
	cmpi.w	#4,18(a1)
	beq.s	.Loru_n2		| nplanes==4: high planes keep bg
	move.l	d2,(a3)+		| planes 4:5
	cmpi.w	#6,18(a1)
	beq.s	.Loru_n3
	move.l	d3,(a3)+		| planes 6:7
	bra.s	.Loru_next
.Loru_n3:
	addq.l	#4,a3
	bra.s	.Loru_next
.Loru_n2:
	addq.l	#8,a3
.Loru_next:
	dbf	d4,.Loru_col
	adda.w	a0,a2			| next row source skip
	adda.w	a6,a3			| next row dest remainder
	dbf	d5,.Loru_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_8bpp_opaque_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   OPAQUE shifted, NON-CLIPPED, width a multiple of 16 (wrapper-gated). Every dest plane =
 *   ((prev<<16)|cur) >> s, no transparency. Middle chunks are written directly; the leading
 *   (prev=0, cur>>s) and trailing (cur=0, (prev<<16)>>s) chunks RMW with the end-masks.
 *   We process one plane per data register and ZERO-EXTEND the word loads (moveq+move.w / swap) so
 *   there is no movem.w sign-extension contaminating the shift. P->nchunks(+0) = spc-1 (middle
 *   count). src(+52)=row base, dst(+56)=leading dest chunk. shift(+14)=s (1..15).
 */
	.globl	SYM(destofen_8bpp_opaque_shifted_asm)
SYM(destofen_8bpp_opaque_shifted_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src (row base)
	move.l	56(sp),a3		| a3 = dst (leading chunk)
	move.w	14(a1),d5		| d5 = shift (lsr.l count)
	move.w	6(a1),d0
	lsl.w	#4,d0
	movea.w	d0,a4			| a4 = src_hskip * 16 (0)
	move.w	2(a1),d7		| d7 = nlines
	movea.w	4(a1),a5		| a5 = dst_rowrem
	movea.w	(a1),a0			| a0 = middle count (spc-1)
.Loos_row:
	move.w	d7,-(sp)		| save row counter
	movea.l	a2,a6			| a6 = src row base (prev / leading reads)

	| --- LEADING chunk: prev = 0, cur = src[0] >> s, RMW with leftmask (preserve bg in cols 0..s-1).
	|     Without the RMW, cols 0..s-1 (which are BEFORE the sprite) get written black. ---
	move.w	10(a1),d4		| d4 = leftmask = 0xffff>>s
	move.w	d4,d7
	not.w	d7			| d7 = ~leftmask
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	(a6),d0			| cur p0
	move.w	2(a6),d1		| cur p1
	move.w	4(a6),d2		| cur p2
	move.w	6(a6),d3		| cur p3
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3			| cur >> s
	and.w	d4,d0			| sprite pixels (cols s..15)
	move.w	(a3),d6
	and.w	d7,d6			| keep bg (cols 0..s-1)
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 0
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 1
	and.w	d4,d2
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,(a3)+		| plane 2
	and.w	d4,d3
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,(a3)+		| plane 3
	cmpi.w	#4,18(a1)
	beq.s	.Lol_skip4
	moveq	#0,d0
	moveq	#0,d1
	move.w	8(a6),d0
	move.w	10(a6),d1
	lsr.l	d5,d0
	lsr.l	d5,d1
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 4
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 5
	cmpi.w	#6,18(a1)
	beq.s	.Lol_skip6
	moveq	#0,d0
	moveq	#0,d1
	move.w	12(a6),d0
	move.w	14(a6),d1
	lsr.l	d5,d0
	lsr.l	d5,d1
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 6
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 7
	bra.s	.Lol_done
.Lol_skip6:
	addq.l	#4,a3
	bra.s	.Lol_done
.Lol_skip4:
	addq.l	#8,a3
.Lol_done:
	lea	16(a2),a2		| a2 = cur pointer at src[1]

	| --- MIDDLE chunks: prev = (a6)+, cur = (a2)+ (swap+move.w cleans the sign-ext) ---
	move.w	a0,d6
	subq.w	#1,d6
	bmi.w	.Loos_trail		| no middle chunks
.Loos_midloop:
	movem.w	(a6)+,d0-d3		| prev p0-p3
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| [prev:cur] p0-p3
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	move.w	d0,(a3)+
	move.w	d1,(a3)+
	move.w	d2,(a3)+
	move.w	d3,(a3)+
	movem.w	(a6)+,d0-d3		| prev p4-p7
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| [prev:cur] p4-p7
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	cmpi.w	#4,18(a1)
	beq.s	.Lom_skip4		| nplanes==4: a6/a2 already advanced, skip writes
	move.w	d0,(a3)+
	move.w	d1,(a3)+
	cmpi.w	#6,18(a1)
	beq.s	.Lom_skip6
	move.w	d2,(a3)+
	move.w	d3,(a3)+
	bra.s	.Lom_done
.Lom_skip6:
	addq.l	#4,a3
	bra.s	.Lom_done
.Lom_skip4:
	addq.l	#8,a3
.Lom_done:
	dbf	d6,.Loos_midloop

	| --- TRAILING chunk: prev = src[spc-1] (at a6), cur = 0 → (prev<<16)>>s spill, RMW with
	|     rightmask (spill is in cols 0..s-1; cols s..15 are PAST the sprite, keep bg). ---
.Loos_trail:
	move.w	12(a1),d4		| d4 = rightmask = ~(0xffff>>s)
	move.w	d4,d7
	not.w	d7			| d7 = ~rightmask
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	(a6),d0
	move.w	2(a6),d1
	move.w	4(a6),d2
	move.w	6(a6),d3		| prev p0-p3 (low word, high = 0)
	swap	d0
	swap	d1
	swap	d2
	swap	d3			| prev << 16
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3			| (prev<<16) >> s = spill
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 0
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 1
	and.w	d4,d2
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,(a3)+		| plane 2
	and.w	d4,d3
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,(a3)+		| plane 3
	cmpi.w	#4,18(a1)
	beq.s	.Lot_skip4
	moveq	#0,d0
	moveq	#0,d1
	move.w	8(a6),d0
	move.w	10(a6),d1
	swap	d0
	swap	d1
	lsr.l	d5,d0
	lsr.l	d5,d1
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 4
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 5
	cmpi.w	#6,18(a1)
	beq.s	.Lot_skip6
	moveq	#0,d0
	moveq	#0,d1
	move.w	12(a6),d0
	move.w	14(a6),d1
	swap	d0
	swap	d1
	lsr.l	d5,d0
	lsr.l	d5,d1
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| plane 6
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| plane 7
	bra.s	.Lot_done
.Lot_skip6:
	addq.l	#4,a3
	bra.s	.Lot_done
.Lot_skip4:
	addq.l	#8,a3
.Lot_done:
	adda.w	a4,a2			| next row source (src_hskip=0 → a2 already at next row base)
	adda.w	a5,a3			| next row dest remainder
	move.w	(sp)+,d7
	dbf	d7,.Loos_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_8bpp_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   Arbitrary-X (shift = P->shift, 1..15), no flip. Transcribed from 0x10b76. The
 *   leftfrac/rightfrac table lookup is removed: the wrapper passes the END-MASK WORDS
 *   directly in P->leftfrac(+10) = 0xffff>>s and P->rightfrac(+12) = ~(0xffff>>s).
 *   P->leftclip(+16)/rightclip(+17) != 0 drop the leading/trailing chunk for viewport-clipped
 *   sprites. src = row base, dst = first visible dest chunk. FP register == a6.
 */
	.globl	SYM(destofen_8bpp_shifted_asm)
SYM(destofen_8bpp_shifted_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src (row base)
	move.l	56(sp),a3		| a3 = dst (leading chunk)
	move.w	14(a1),d5		| d5 = shift

	move.w	6(a1),d0
	lsl.w	#4,d0
	movea.w	d0,a4			| a4 = src_hskip * 16
	move.w	2(a1),d7		| d7 = nlines
	move.w	8(a1),d3
	ext.l	d3
	lsl.l	#4,d3
	adda.l	d3,a2			| a2 += src_start * 16
	movea.w	4(a1),a5		| a5 = dst_rowrem
	| (original 0x10b9a/0x10ba4 leftfrac/rightfrac table lookups removed — P holds masks)
	move.w	d5,d6
	swap	d5
	move.w	#16,d5
	sub.w	d6,d5
	swap	d5			| d5 = (16-shift):(shift)
	tst.b	16(a1)
	bne.s	.Ls_nlc
	subq.w	#1,(a1)			| not left-clipped -> nchunks--
.Ls_nlc:
	movea.w	(a1),a0
	subq.w	#1,a0			| a0 = middle dbf (nchunks-1)

.Ls_row:
	move.w	d7,-(sp)		| save row counter
	movea.l	a2,a6			| a6(FP) = src row base
	lea	16(a2),a2		| a2 = +16 (past leading chunk)
	tst.b	16(a1)
	bne.w	.Ls_mid			| left-clipped -> skip leading

	| --- LEADING chunk (left end-masked) ---
	movem.l	(a6),d0-d3
	move.l	d0,d6
	or.l	d1,d6
	or.l	d2,d6
	or.l	d3,d6
	move.l	d6,d4
	swap	d6
	or.l	d4,d6
	lsr.l	d5,d6
	move.w	10(a1),d4		| left end-mask
	and.w	d4,d6
	not.l	d6
	lsr.l	d5,d0
	and.w	d4,d0
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d0,d7
	move.l	d7,(a3)+		| planes 0:1
	lsr.l	d5,d1
	and.w	d4,d1
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d1,d7
	move.l	d7,(a3)+		| planes 2:3   (a3 +8)
	cmpi.w	#4,18(a1)
	beq.s	.Ll_skip2
	lsr.l	d5,d2
	and.w	d4,d2
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d2,d7
	move.l	d7,(a3)+		| planes 4:5   (a3 +12)
	cmpi.w	#6,18(a1)
	beq.s	.Ll_skip3
	lsr.l	d5,d3
	and.w	d4,d3
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d3,d7
	move.l	d7,(a3)+		| planes 6:7   (a3 +16)
	bra.s	.Ll_done
.Ll_skip3:
	addq.l	#4,a3
	bra.s	.Ll_done
.Ll_skip2:
	addq.l	#8,a3
.Ll_done:

.Ls_mid:
	move.w	a0,d6
	bmi.w	.Ls_trail		| no middle chunks -> trailing
	cmpi.w	#4,18(a1)
	beq.w	.Ls_midloop4		| 16-colour: skip the always-zero high 4 planes (read+shift+union)
.Ls_midloop:
	movem.w	(a6)+,d0-d3		| prev chunk p0-p3 -> low words
	swap	d0
	swap	d1
	swap	d2
	swap	d3			| prev -> HIGH halves
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| cur chunk p0-p3 -> LOW halves
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3			| [prev:cur] >> shift
	move.w	d0,d7
	or.w	d1,d7
	or.w	d2,d7
	or.w	d3,d7			| partial union (p0-p3)
	movem.w	d0-d3,-(sp)		| stash shifted p0-p3
	movem.w	(a6)+,d0-d3		| prev chunk p4-p7
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| cur chunk p4-p7
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	or.w	d0,d7
	or.w	d1,d7
	or.w	d2,d7
	or.w	d3,d7			| full union
	beq.w	.Lms_skip		| fully transparent chunk: pop stash, no dst traffic
	not.w	d7			| transparency mask (word)
	beq.w	.Lms_opq		| fully opaque chunk: pure stores, no dst reads
.Lms_rmw:
	move.w	(a3),d4
	and.w	d7,d4
	or.w	(sp)+,d4
	move.w	d4,(a3)+		| plane 0
	move.w	(a3),d4
	and.w	d7,d4
	or.w	(sp)+,d4
	move.w	d4,(a3)+		| plane 1
	move.w	(a3),d4
	and.w	d7,d4
	or.w	(sp)+,d4
	move.w	d4,(a3)+		| plane 2
	move.w	(a3),d4
	and.w	d7,d4
	or.w	(sp)+,d4
	move.w	d4,(a3)+		| plane 3   (a3 +8, stack balanced)
	cmpi.w	#4,18(a1)
	beq.s	.Lm_skip4		| nplanes==4 -> skip planes 4-7
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d0,d4
	move.w	d4,(a3)+		| plane 4
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d1,d4
	move.w	d4,(a3)+		| plane 5   (a3 +12)
	cmpi.w	#6,18(a1)
	beq.s	.Lm_skip6		| nplanes==6
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d2,d4
	move.w	d4,(a3)+		| plane 6
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d3,d4
	move.w	d4,(a3)+		| plane 7   (a3 +16)
	bra.s	.Lm_done
.Lm_skip6:
	addq.l	#4,a3			| skip planes 6,7
	bra.s	.Lm_done
.Lm_skip4:
	addq.l	#8,a3			| skip planes 4,5,6,7
	bra.s	.Lm_done
.Lms_opq:
	cmpi.w	#8,18(a1)		| reduced planes: RMW path (d7==0 -> pure store,
	bne.s	.Lms_rmw		|   high planes untouched)
	move.w	(sp)+,(a3)+
	move.w	(sp)+,(a3)+
	move.w	(sp)+,(a3)+
	move.w	(sp)+,(a3)+		| planes 0-3 from stash, no dst read
	movem.w	d0-d3,(a3)		| planes 4-7
	addq.l	#8,a3
	bra.s	.Lm_done
.Lms_skip:
	addq.l	#8,sp			| discard stashed p0-3
	lea	16(a3),a3		| bit-exact: RMW with union==0 wrote dst unchanged
.Lm_done:
	dbf	d6,.Ls_midloop
	bra.w	.Ls_trail

	| --- 4-plane middle (16-colour): identical output to the 8-plane path with nplanes=4, but the
	|     high 4 planes (always zero) are not read/shifted/union'd, and p0-3 are written straight
	|     from the registers (no stash). Pointers advance the same 16 bytes/chunk (addq skips the
	|     unread high planes). ---
.Ls_midloop4:
	movem.w	(a6)+,d0-d3		| prev p0-p3 (a6 += 8)
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| cur p0-p3 (a2 += 8)
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3			| [prev:cur] >> shift
	move.w	d0,d7
	or.w	d1,d7
	or.w	d2,d7
	or.w	d3,d7			| union (p0-3 only; high planes are zero)
	beq.s	.Lm4_skip		| fully transparent chunk: no dst traffic
	not.w	d7			| transparency mask
	beq.s	.Lm4_opq		| fully opaque chunk: pure stores, no dst reads
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d0,d4
	move.w	d4,(a3)+		| plane 0
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d1,d4
	move.w	d4,(a3)+		| plane 1
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d2,d4
	move.w	d4,(a3)+		| plane 2
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d3,d4
	move.w	d4,(a3)+		| plane 3
.Lm4_next:
	addq.l	#8,a6			| skip prev p4-p7 (unread)
	addq.l	#8,a2			| skip cur p4-p7 (unread)
	addq.l	#8,a3			| skip dst p4-p7
	dbf	d6,.Ls_midloop4
	bra.s	.Lm4_out
.Lm4_opq:
	movem.w	d0-d3,(a3)		| planes 0-3, no dst read (this loop is nplanes==4 only)
.Lm4_skip:
	addq.l	#8,a3			| dst p0-3 untouched/just written; advance
	bra.s	.Lm4_next
.Lm4_out:

.Ls_trail:
	tst.b	17(a1)
	bne.s	.Ls_nextrow		| right-clipped -> no trailing residual
	movem.l	(a6),d0-d3
	swap	d5			| d5 low word = 16-shift now
	move.l	d0,d6
	or.l	d1,d6
	or.l	d2,d6
	or.l	d3,d6
	move.l	d6,d4
	swap	d6
	or.l	d4,d6
	lsl.l	d5,d6
	move.w	12(a1),d4		| right end-mask
	ext.l	d4
	swap	d4
	and.l	d4,d6
	not.l	d6
	lsl.l	d5,d0
	and.l	d4,d0
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d0,d7
	move.l	d7,(a3)+		| planes 0:1
	lsl.l	d5,d1
	and.l	d4,d1
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d1,d7
	move.l	d7,(a3)+		| planes 2:3   (a3 +8)
	cmpi.w	#4,18(a1)
	beq.s	.Lt_skip2
	lsl.l	d5,d2
	and.l	d4,d2
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d2,d7
	move.l	d7,(a3)+		| planes 4:5   (a3 +12)
	cmpi.w	#6,18(a1)
	beq.s	.Lt_skip3
	lsl.l	d5,d3
	and.l	d4,d3
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d3,d7
	move.l	d7,(a3)+		| planes 6:7   (a3 +16)
	bra.s	.Lt_done
.Lt_skip3:
	addq.l	#4,a3			| advance to chunk end (+16)
	bra.s	.Lt_done
.Lt_skip2:
	addq.l	#8,a3
.Lt_done:
	lea	-16(a3),a3		| back up A3 (trailing doesn't advance the column)
	swap	d5			| restore d5 = (16-shift):(shift)

.Ls_nextrow:
	adda.w	a4,a2			| next row source (skip clipped-off chunks)
	adda.w	a5,a3			| next row dest remainder
	move.w	(sp)+,d7
	dbf	d7,.Ls_row

	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_edge_asm(const destofen_edge_blk *P, const u16 *prev, const u16 *cur, u16 *dst)
 *   Draws ONE transparent, masked double-shift chunk per row — the pixel-exact CLIP boundary chunk
 *   that the original blitter skipped. Plane value = ((prev<<16)|cur) >> s; transparency union;
 *   RMW with (union & mask): dst = (dst & ~(un&cm)) | (d & cm). Branch-free zero handling: for a
 *   leading/spill chunk the wrapper points prev/cur at a static zero chunk with stride 0.
 *   P: nlines(+2)=h-1, dst_stride(+4 bytes), prev_stride(+6 bytes), cur_stride(+8 bytes),
 *      mask(+10 = cm), shift(+14 = s), nplanes(+18). prev/cur are NOT advanced internally.
 */
	.globl	SYM(destofen_edge_asm)
SYM(destofen_edge_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = prev ptr
	move.l	56(sp),a3		| a3 = cur ptr
	move.l	60(sp),a0		| a0 = dst ptr
	move.w	14(a1),d5		| d5 = shift
	move.w	10(a1),d4		| d4 = mask (cm)
	move.w	2(a1),d7		| d7 = nlines (dbf count)
	movea.w	4(a1),a4		| a4 = dst_stride (bytes)
	movea.w	6(a1),a5		| a5 = prev_stride
	movea.w	8(a1),a6		| a6 = cur_stride
.Lmc_row:
	move.w	d7,-(sp)		| save row counter
	| --- planes 0-3: [prev:cur] >> s ---
	movem.w	(a2),d0-d3		| prev p0-p3 (low words; sign-ext cleaned by swap+move.w)
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a3),d0
	move.w	2(a3),d1
	move.w	4(a3),d2
	move.w	6(a3),d3		| [prev:cur] p0-p3
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	move.w	d0,d6
	or.w	d1,d6
	or.w	d2,d6
	or.w	d3,d6			| partial union (p0-p3)
	cmpi.w	#4,18(a1)
	bne.s	.Lmc_full		| 16-colour: skip the always-zero p4-7 (read+union)
	and.w	d4,d6			| wm = union(p0-3) & cm
	move.w	d6,d7
	not.w	d7			| nm = ~wm
	bra.w	.Lmc_p03_nopop		| write p0-3 from d0-d3 (no stash)
.Lmc_full:
	movem.w	d0-d3,-(sp)		| stash shifted p0-p3
	| --- planes 4-7 ---
	movem.w	8(a2),d0-d3		| prev p4-p7
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	8(a3),d0
	move.w	10(a3),d1
	move.w	12(a3),d2
	move.w	14(a3),d3		| [prev:cur] p4-p7
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	or.w	d0,d6
	or.w	d1,d6
	or.w	d2,d6
	or.w	d3,d6			| full union (all 8 planes)
	and.w	d4,d6			| union & mask = write mask (wm)
	move.w	d6,d7
	not.w	d7			| d7 = ~wm = nm
	| --- RMW planes 4-7 (in d0-d3), gated by nplanes ---
	cmpi.w	#4,18(a1)
	beq.s	.Lmc_p03		| nplanes==4 -> skip high planes
	and.w	d4,d0
	move.w	8(a0),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,8(a0)		| plane 4
	and.w	d4,d1
	move.w	10(a0),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,10(a0)		| plane 5
	cmpi.w	#6,18(a1)
	beq.s	.Lmc_p03
	and.w	d4,d2
	move.w	12(a0),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,12(a0)		| plane 6
	and.w	d4,d3
	move.w	14(a0),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,14(a0)		| plane 7
.Lmc_p03:
	movem.w	(sp)+,d0-d3		| restore shifted p0-p3 (8-plane path)
.Lmc_p03_nopop:
	and.w	d4,d0
	move.w	(a0),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a0)			| plane 0
	and.w	d4,d1
	move.w	2(a0),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,2(a0)		| plane 1
	and.w	d4,d2
	move.w	4(a0),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,4(a0)		| plane 2
	and.w	d4,d3
	move.w	6(a0),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,6(a0)		| plane 3
	adda.w	a4,a0			| dst += dst_stride
	adda.w	a5,a2			| prev += prev_stride
	adda.w	a6,a3			| cur += cur_stride
	move.w	(sp)+,d7		| restore row counter
	dbf	d7,.Lmc_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_edge_opaque_asm(const destofen_edge_blk *P, const u16 *prev, const u16 *cur, u16 *dst)
 *   OPAQUE variant of destofen_edge_asm: one clip-masked double-shift chunk per row, writing
 *   EVERY in-mask pixel (index-0 black included) — no transparency union. dst = (dst & ~cm) | (d & cm),
 *   d = ((prev<<16)|cur) >> s. Used for opaque odd-width / edge-clipped sprites (per visible chunk).
 *   Zero handling: leading/spill boundary → prev/cur point at a static zero chunk with stride 0.
 *   P: nlines(+2)=h-1, dst_stride(+4), prev_stride(+6), cur_stride(+8), mask(+10), shift(+14), nplanes(+18).
 */
	.globl	SYM(destofen_edge_opaque_asm)
SYM(destofen_edge_opaque_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = prev
	move.l	56(sp),a3		| a3 = cur
	move.l	60(sp),a0		| a0 = dst
	move.w	14(a1),d5		| d5 = shift
	move.w	10(a1),d4		| d4 = cm (mask)
	move.w	d4,d7
	not.w	d7			| d7 = ~cm (nm)
	move.w	2(a1),d6		| d6 = nlines (dbf count)
	movea.w	4(a1),a4		| a4 = dst_stride
	movea.w	6(a1),a5		| a5 = prev_stride
	movea.w	8(a1),a6		| a6 = cur_stride
.Lmco_row:
	| plane 0
	move.w	(a2),d0
	swap	d0
	move.w	(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,(a0)
	| plane 1
	move.w	2(a2),d0
	swap	d0
	move.w	2(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	2(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,2(a0)
	| plane 2
	move.w	4(a2),d0
	swap	d0
	move.w	4(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	4(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,4(a0)
	| plane 3
	move.w	6(a2),d0
	swap	d0
	move.w	6(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	6(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,6(a0)
	cmpi.w	#4,18(a1)
	beq.s	.Lmco_adv		| nplanes==4
	| plane 4
	move.w	8(a2),d0
	swap	d0
	move.w	8(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	8(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,8(a0)
	| plane 5
	move.w	10(a2),d0
	swap	d0
	move.w	10(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	10(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,10(a0)
	cmpi.w	#6,18(a1)
	beq.s	.Lmco_adv		| nplanes==6
	| plane 6
	move.w	12(a2),d0
	swap	d0
	move.w	12(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	12(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,12(a0)
	| plane 7
	move.w	14(a2),d0
	swap	d0
	move.w	14(a3),d0
	lsr.l	d5,d0
	and.w	d4,d0
	move.w	14(a0),d1
	and.w	d7,d1
	or.w	d0,d1
	move.w	d1,14(a0)
.Lmco_adv:
	adda.w	a4,a0			| dst += dst_stride
	adda.w	a5,a2			| prev += prev_stride
	adda.w	a6,a3			| cur += cur_stride
	dbf	d6,.Lmco_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_4bpp_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   4-PLANE SOURCE variant of destofen_8bpp_shifted_asm (16-colour games). Reads 8-byte (4-plane)
 *   source chunks — src_hskip/src_start scale ×8 (not ×16), the leading/trailing read 2 longwords
 *   (p0-3) instead of 4, the middle reads 4 planes and advances 8. Writes 4 planes into the 8-plane
 *   framebuffer (dst chunk stays 16 bytes; +8 skips the high planes, which carry 0 / clinepal base).
 *   Same P layout + transparency/leftfrac/rightfrac/leftclip/rightclip semantics as the 8-plane one.
 */
	.globl	SYM(destofen_4bpp_shifted_asm)
SYM(destofen_4bpp_shifted_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src (row base, 4-plane)
	move.l	56(sp),a3		| a3 = dst (leading chunk, 8-plane fb)
	move.w	14(a1),d5		| d5 = shift
	move.w	6(a1),d0
	lsl.w	#3,d0
	movea.w	d0,a4			| a4 = src_hskip * 8 (4-plane chunk bytes)
	move.w	2(a1),d7		| d7 = nlines
	move.w	8(a1),d3
	ext.l	d3
	lsl.l	#3,d3
	adda.l	d3,a2			| a2 += src_start * 8
	movea.w	4(a1),a5		| a5 = dst_rowrem
	move.w	d5,d6
	swap	d5
	move.w	#16,d5
	sub.w	d6,d5
	swap	d5			| d5 = (16-shift):(shift)
	tst.b	16(a1)
	bne.s	.L4s_nlc
	subq.w	#1,(a1)			| not left-clipped -> nchunks--
.L4s_nlc:
	movea.w	(a1),a0
	subq.w	#1,a0			| a0 = middle dbf (nchunks-1)
.L4s_row:
	move.w	d7,-(sp)
	movea.l	a2,a6			| a6 = src row base
	lea	8(a2),a2		| a2 = past leading chunk (4-plane = +8)
	tst.b	16(a1)
	bne.w	.L4s_mid		| left-clipped -> skip leading
	| --- LEADING chunk (left end-masked, 2 longwords) ---
	movem.l	(a6),d0-d1		| p0:p1, p2:p3
	move.l	d0,d6
	or.l	d1,d6
	move.l	d6,d4
	swap	d6
	or.l	d4,d6			| fold -> low word = per-pixel union (4 planes)
	lsr.l	d5,d6
	move.w	10(a1),d4		| left end-mask
	and.w	d4,d6
	not.l	d6
	lsr.l	d5,d0
	and.w	d4,d0
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d0,d7
	move.l	d7,(a3)+		| planes 0:1
	lsr.l	d5,d1
	and.w	d4,d1
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d1,d7
	move.l	d7,(a3)+		| planes 2:3
	addq.l	#8,a3			| skip dst p4-7 -> a3 += 16
.L4s_mid:
	move.w	a0,d6
	bmi.w	.L4s_trail
.L4s_midloop:
	movem.w	(a6)+,d0-d3		| prev p0-3 (a6 += 8)
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| cur p0-3 (a2 += 8)
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	move.w	d0,d7
	or.w	d1,d7
	or.w	d2,d7
	or.w	d3,d7
	beq.s	.L4s_skip		| fully transparent chunk: no dst traffic
	not.w	d7
	beq.s	.L4s_opq		| fully opaque chunk: pure stores, no dst reads
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d0,d4
	move.w	d4,(a3)+		| plane 0
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d1,d4
	move.w	d4,(a3)+		| plane 1
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d2,d4
	move.w	d4,(a3)+		| plane 2
	move.w	(a3),d4
	and.w	d7,d4
	or.w	d3,d4
	move.w	d4,(a3)+		| plane 3
	addq.l	#8,a3			| skip dst p4-7 -> a3 += 16
.L4s_next:
	dbf	d6,.L4s_midloop
	bra.s	.L4s_trail
.L4s_opq:
	movem.w	d0-d3,(a3)		| planes 0-3, no dst read (4-plane loop)
.L4s_skip:
	lea	16(a3),a3
	bra.s	.L4s_next
.L4s_trail:
	tst.b	17(a1)
	bne.s	.L4s_nextrow
	movem.l	(a6),d0-d1		| prev p0-3 (2 longwords)
	swap	d5			| d5 low = 16-shift
	move.l	d0,d6
	or.l	d1,d6
	move.l	d6,d4
	swap	d6
	or.l	d4,d6
	lsl.l	d5,d6
	move.w	12(a1),d4		| right end-mask
	ext.l	d4
	swap	d4
	and.l	d4,d6
	not.l	d6
	lsl.l	d5,d0
	and.l	d4,d0
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d0,d7
	move.l	d7,(a3)+		| planes 0:1
	lsl.l	d5,d1
	and.l	d4,d1
	move.l	(a3),d7
	and.l	d6,d7
	or.l	d1,d7
	move.l	d7,(a3)+		| planes 2:3
	addq.l	#8,a3			| advance to chunk end (+16)
	lea	-16(a3),a3		| back up (trailing doesn't advance column)
	swap	d5			| restore d5 = (16-shift):(shift)
.L4s_nextrow:
	adda.w	a4,a2			| next row source
	adda.w	a5,a3			| next row dest remainder
	move.w	(sp)+,d7
	dbf	d7,.L4s_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_4bpp_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   4-PLANE SOURCE, word-aligned. Pure 4-plane (no nplanes branch): reads 2 longwords (p0-3) per
 *   8-byte source chunk, transparency union, RMW p0:1 / p2:3 into the 8-plane framebuffer (+8 skips
 *   the dst high planes). src_hskip/src_start scale ×8.
 */
	.globl	SYM(destofen_4bpp_aligned_asm)
SYM(destofen_4bpp_aligned_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1		| a1 = P
	move.l	52(sp),a2		| a2 = src (4-plane)
	move.l	56(sp),a3		| a3 = dst (8-plane fb)
	move.w	6(a1),d0
	lsl.w	#3,d0
	movea.w	d0,a0			| a0 = src_hskip * 8
	move.w	2(a1),d5		| d5 = nlines
	move.w	8(a1),d4
	ext.l	d4
	lsl.l	#3,d4
	adda.l	d4,a2			| a2 += src_start * 8
	movea.w	4(a1),a6		| a6 = dst_rowrem
	movea.w	(a1),a4
	subq.w	#1,a4			| a4 = nchunks - 1
.L4u_row:
	move.w	a4,d4
.L4u_col:
	movem.l	(a2)+,d0-d1		| p0:p1, p2:p3 (a2 += 8)
	move.l	d0,d7
	or.l	d1,d7			| union (2 longwords)
	move.l	d7,d6
	swap	d7
	or.l	d6,d7			| fold -> both words = per-pixel union
	beq.s	.L4u_skip		| fully transparent chunk: no dst traffic
	not.l	d7			| ~union = transparency mask
	beq.s	.L4u_opq		| fully opaque chunk: pure stores, no dst reads
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d0,d6
	move.l	d6,(a3)+		| planes 0:1
	move.l	(a3),d6
	and.l	d7,d6
	or.l	d1,d6
	move.l	d6,(a3)+		| planes 2:3
	addq.l	#8,a3			| skip dst p4-7 -> a3 += 16
.L4u_next:
	dbf	d4,.L4u_col
	adda.w	a0,a2			| next row source skip
	adda.w	a6,a3			| next row dest remainder
	dbf	d5,.L4u_row
	bra.s	.L4u_done
.L4u_opq:
	movem.l	d0-d1,(a3)		| planes 0:1, 2:3 — no dst read (4-plane routine)
.L4u_skip:
	lea	16(a3),a3
	bra.s	.L4u_next
.L4u_done:
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_4bpp_opaque_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   4-PLANE SOURCE, opaque, word-aligned. Write-only (no union/RMW): copy p0-3 verbatim into the
 *   8-plane framebuffer; the dst high planes are left as-is (0 / clinepal base). src ×8.
 */
	.globl	SYM(destofen_4bpp_opaque_aligned_asm)
SYM(destofen_4bpp_opaque_aligned_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1
	move.l	52(sp),a2
	move.l	56(sp),a3
	move.w	6(a1),d0
	lsl.w	#3,d0
	movea.w	d0,a0			| src_hskip * 8
	move.w	2(a1),d5		| nlines
	move.w	8(a1),d4
	ext.l	d4
	lsl.l	#3,d4
	adda.l	d4,a2			| src_start * 8
	movea.w	4(a1),a6		| dst_rowrem
	movea.w	(a1),a4
	subq.w	#1,a4			| nchunks - 1
.L4ou_row:
	move.w	a4,d4
.L4ou_col:
	movem.l	(a2)+,d0-d1		| p0:p1, p2:p3 (a2 += 8)
	move.l	d0,(a3)+		| planes 0:1 (direct write, opaque)
	move.l	d1,(a3)+		| planes 2:3
	addq.l	#8,a3			| skip dst p4-7 -> a3 += 16
	dbf	d4,.L4ou_col
	adda.w	a0,a2
	adda.w	a6,a3
	dbf	d5,.L4ou_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/*
 * void destofen_4bpp_opaque_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst)
 *   4-PLANE SOURCE, opaque, shifted, non-clipped (mirror of destofen_8bpp_opaque_shifted_asm, p0-3
 *   only). Leading = cur>>s RMW leftmask; middle = ((prev<<16)|cur)>>s direct write; trailing =
 *   (prev<<16)>>s RMW rightmask. Writes 4 planes into the 8-plane fb (+8 skips dst high). src ×8.
 */
	.globl	SYM(destofen_4bpp_opaque_shifted_asm)
SYM(destofen_4bpp_opaque_shifted_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a1
	move.l	52(sp),a2
	move.l	56(sp),a3
	move.w	14(a1),d5		| shift
	move.w	6(a1),d0
	lsl.w	#3,d0
	movea.w	d0,a4			| src_hskip * 8
	move.w	2(a1),d7		| nlines
	movea.w	4(a1),a5		| dst_rowrem
	movea.w	(a1),a0			| middle count (spc-1)
.L4os_row:
	move.w	d7,-(sp)
	movea.l	a2,a6
	| --- LEADING: cur>>s, RMW leftmask ---
	move.w	10(a1),d4		| leftmask
	move.w	d4,d7
	not.w	d7			| ~leftmask
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	(a6),d0
	move.w	2(a6),d1
	move.w	4(a6),d2
	move.w	6(a6),d3
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| p0
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| p1
	and.w	d4,d2
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,(a3)+		| p2
	and.w	d4,d3
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,(a3)+		| p3
	addq.l	#8,a3			| skip dst p4-7 -> +16
	lea	8(a2),a2		| cur at src[1]
	| --- MIDDLE: direct write ---
	move.w	a0,d6
	subq.w	#1,d6
	bmi.w	.L4os_trail
.L4os_midloop:
	movem.w	(a6)+,d0-d3		| prev p0-3 (a6 += 8)
	swap	d0
	swap	d1
	swap	d2
	swap	d3
	move.w	(a2)+,d0
	move.w	(a2)+,d1
	move.w	(a2)+,d2
	move.w	(a2)+,d3		| cur p0-3 (a2 += 8)
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3
	move.w	d0,(a3)+		| p0
	move.w	d1,(a3)+		| p1
	move.w	d2,(a3)+		| p2
	move.w	d3,(a3)+		| p3
	addq.l	#8,a3			| skip dst p4-7 -> +16
	dbf	d6,.L4os_midloop
.L4os_trail:
	| --- TRAILING: (prev<<16)>>s spill, RMW rightmask ---
	move.w	12(a1),d4		| rightmask
	move.w	d4,d7
	not.w	d7			| ~rightmask
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	(a6),d0
	move.w	2(a6),d1
	move.w	4(a6),d2
	move.w	6(a6),d3		| prev p0-3
	swap	d0
	swap	d1
	swap	d2
	swap	d3			| prev << 16
	lsr.l	d5,d0
	lsr.l	d5,d1
	lsr.l	d5,d2
	lsr.l	d5,d3			| spill
	and.w	d4,d0
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d0,d6
	move.w	d6,(a3)+		| p0
	and.w	d4,d1
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d1,d6
	move.w	d6,(a3)+		| p1
	and.w	d4,d2
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d2,d6
	move.w	d6,(a3)+		| p2
	and.w	d4,d3
	move.w	(a3),d6
	and.w	d7,d6
	or.w	d3,d6
	move.w	d6,(a3)+		| p3
	addq.l	#8,a3			| skip dst p4-7 -> +16
	adda.w	a4,a2			| next row source
	adda.w	a5,a3			| next row dest remainder
	move.w	(sp)+,d7
	dbf	d7,.L4os_row
	movem.l	(sp)+,d2-d7/a2-a6
#endif
	rts

/* ======================================================================== */

/*
 * void cdestofen_span_asm(
 *     const u8 *src,     // source chunky pixels (sequential)
 *     u16 *dst,          // destination planar (8 words per 16px group)
 *     s32 count,         // number of pixels (1-16)
 *     s32 startbit       // bit offset within 16px group (0-15)
 * )
 *
 * Standard m68k C calling convention: args on stack.
 * After movem.l d2-d7/a2-a3,-(sp) saves 8 regs = 32 bytes:
 *   36(sp) = src, 40(sp) = dst, 44(sp) = count, 48(sp) = startbit
 */
	.globl	SYM(cdestofen_span_asm)
SYM(cdestofen_span_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a3,-(sp)

	move.l	36(sp),a0		/* src */
	move.l	40(sp),a3		/* dst (save in a3, a1 used later) */
	move.l	44(sp),d0		/* count */
	move.l	48(sp),d1		/* startbit */

	/* p0-p3 in d2-d5, tmask in d6, bit in d7 */
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	/* p4-p7 on stack */
	sub.w	#8,sp
	clr.l	(sp)
	clr.l	4(sp)
	moveq	#0,d6			/* tmask */

	/* d7 = 0x8000 >> startbit */
	move.w	#0x8000,d7
	lsr.w	d1,d7

	subq.w	#1,d0			/* for dbf */
.Ltrans_loop:
	moveq	#0,d1
	move.b	(a0)+,d1
	beq.s	.Ltrans_skip

	or.w	d7,d6			/* tmask |= bit */

	btst	#0,d1
	beq.s	.Lt0
	or.w	d7,d2
.Lt0:	btst	#1,d1
	beq.s	.Lt1
	or.w	d7,d3
.Lt1:	btst	#2,d1
	beq.s	.Lt2
	or.w	d7,d4
.Lt2:	btst	#3,d1
	beq.s	.Lt3
	or.w	d7,d5
.Lt3:	btst	#4,d1
	beq.s	.Lt4
	or.w	d7,(sp)
.Lt4:	btst	#5,d1
	beq.s	.Lt5
	or.w	d7,2(sp)
.Lt5:	btst	#6,d1
	beq.s	.Lt6
	or.w	d7,4(sp)
.Lt6:	btst	#7,d1
	beq.s	.Lt7
	or.w	d7,6(sp)
.Lt7:
.Ltrans_skip:
	lsr.w	#1,d7
	dbf	d0,.Ltrans_loop

	/* Write back with AND-OR mask */
	tst.w	d6
	beq.s	.Ltrans_done

	move.l	a3,a1			/* restore dst ptr */
	not.w	d6			/* nm = ~tmask */

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d2,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d3,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d4,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d5,d0
	move.w	d0,(a1)+

	move.w	(sp)+,d2		/* p4 */
	move.w	(sp)+,d3		/* p5 */
	move.w	(sp)+,d4		/* p6 */
	move.w	(sp)+,d5		/* p7 */

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d2,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d3,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d4,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d5,d0
	move.w	d0,(a1)

	movem.l	(sp)+,d2-d7/a2-a3
	rts

.Ltrans_done:
	add.w	#8,sp
	movem.l	(sp)+,d2-d7/a2-a3
	rts
#else
	rts
#endif


/*
 * void cdestofen_span_opaque_asm(
 *     const u8 *src,
 *     u16 *dst,
 *     s32 count,
 *     s32 startbit
 * )
 *
 * Opaque version. For count==16 && startbit==0: direct write.
 * Otherwise: mask write (same as transparent but mask = all written pixels).
 */
	.globl	SYM(cdestofen_span_opaque_asm)
SYM(cdestofen_span_opaque_asm):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a3,-(sp)

	move.l	36(sp),a0		/* src */
	move.l	40(sp),a3		/* dst */
	move.l	44(sp),d0		/* count */
	move.l	48(sp),d1		/* startbit */

	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	sub.w	#8,sp
	clr.l	(sp)
	clr.l	4(sp)

	move.w	#0x8000,d7
	lsr.w	d1,d7

	/* Build pixel mask in d6 for partial writes */
	moveq	#0,d6
	move.l	d0,a2			/* save count */

	subq.w	#1,d0
.Lopaque_loop:
	or.w	d7,d6			/* always set mask bit */
	moveq	#0,d1
	move.b	(a0)+,d1

	btst	#0,d1
	beq.s	.Lo0
	or.w	d7,d2
.Lo0:	btst	#1,d1
	beq.s	.Lo1
	or.w	d7,d3
.Lo1:	btst	#2,d1
	beq.s	.Lo2
	or.w	d7,d4
.Lo2:	btst	#3,d1
	beq.s	.Lo3
	or.w	d7,d5
.Lo3:	btst	#4,d1
	beq.s	.Lo4
	or.w	d7,(sp)
.Lo4:	btst	#5,d1
	beq.s	.Lo5
	or.w	d7,2(sp)
.Lo5:	btst	#6,d1
	beq.s	.Lo6
	or.w	d7,4(sp)
.Lo6:	btst	#7,d1
	beq.s	.Lo7
	or.w	d7,6(sp)
.Lo7:
	lsr.w	#1,d7
	dbf	d0,.Lopaque_loop

	move.l	a3,a1			/* dst */

	/* Check if full 16px group (d6 == 0xFFFF) */
	cmp.w	#0xFFFF,d6
	bne.s	.Lopaque_partial

	/* Full group: direct write, no read-modify-write */
	move.w	d2,(a1)+
	move.w	d3,(a1)+
	move.w	d4,(a1)+
	move.w	d5,(a1)+
	move.w	(sp)+,(a1)+
	move.w	(sp)+,(a1)+
	move.w	(sp)+,(a1)+
	move.w	(sp)+,(a1)
	movem.l	(sp)+,d2-d7/a2-a3
	rts

.Lopaque_partial:
	/* Partial: mask write */
	not.w	d6			/* nm = ~mask */

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d2,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d3,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d4,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d5,d0
	move.w	d0,(a1)+

	move.w	(sp)+,d2
	move.w	(sp)+,d3
	move.w	(sp)+,d4
	move.w	(sp)+,d5

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d2,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d3,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d4,d0
	move.w	d0,(a1)+

	move.w	(a1),d0
	and.w	d6,d0
	or.w	d5,d0
	move.w	d0,(a1)

	movem.l	(sp)+,d2-d7/a2-a3
	rts
#else
	rts
#endif

/* ---------------------------------------------------------------------------
 * void c2p8_rect(const u8 *src, s32 src_pitch, u8 *dst, s32 dst_pitch, s32 chunks, s32 rows)
 * 8-bit chunky -> 8-plane interleaved, 16 pixels (one chunk = 8 plane words) at a time.
 * The Kalms transpose, step for step as kalms16_8 in opcodes.c. The chunk loop fits the
 * 030 I-cache; a4/a5 (pinned by the C build) are not used.
 * ------------------------------------------------------------------------- */
	.globl	SYM(c2p8_rect)
SYM(c2p8_rect):
#if !defined(__mcoldfire__)
	movem.l	d2-d7/a2-a3/a6,-(sp)	| 9 regs: args from 40(sp)
	movea.l	40(sp),a2		| src row
	movea.l	48(sp),a3		| dst row
	movea.l	52(sp),a6		| dst pitch
	move.l	#0x0f0f0f0f,d4
	move.l	#0x00ff00ff,d5
	bra	3f
1:	movea.l	a2,a0
	movea.l	a3,a1
	move.w	58(sp),d6		| chunks (low word)
	subq.w	#1,d6
2:	movem.l	(a0)+,d0-d3
	move.l	d1,d7			| 4-bit
	lsr.l	#4,d7
	eor.l	d0,d7
	and.l	d4,d7
	eor.l	d7,d0
	lsl.l	#4,d7
	eor.l	d7,d1
	move.l	d3,d7
	lsr.l	#4,d7
	eor.l	d2,d7
	and.l	d4,d7
	eor.l	d7,d2
	lsl.l	#4,d7
	eor.l	d7,d3
	move.l	d2,d7			| 8-bit
	lsr.l	#8,d7
	eor.l	d0,d7
	and.l	d5,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d2
	move.l	d3,d7
	lsr.l	#8,d7
	eor.l	d1,d7
	and.l	d5,d7
	eor.l	d7,d1
	lsl.l	#8,d7
	eor.l	d7,d3
	move.l	d2,d7			| 1-bit
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	#0x55555555,d7
	eor.l	d7,d0
	add.l	d7,d7
	eor.l	d7,d2
	move.l	d3,d7
	lsr.l	#1,d7
	eor.l	d1,d7
	and.l	#0x55555555,d7
	eor.l	d7,d1
	add.l	d7,d7
	eor.l	d7,d3
	move.w	d2,d7			| exchange low words d0<->d2, d1<->d3
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2
	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3
	move.l	d2,d7			| 2-bit
	lsr.l	#2,d7
	eor.l	d0,d7
	and.l	#0x33333333,d7
	eor.l	d7,d0
	lsl.l	#2,d7
	eor.l	d7,d2
	move.l	d3,d7
	lsr.l	#2,d7
	eor.l	d1,d7
	and.l	#0x33333333,d7
	eor.l	d7,d1
	lsl.l	#2,d7
	eor.l	d7,d3
	swap	d3			| planes 0:1, 2:3, 4:5, 6:7
	move.l	d3,(a1)+
	swap	d1
	move.l	d1,(a1)+
	swap	d2
	move.l	d2,(a1)+
	swap	d0
	move.l	d0,(a1)+
	dbf	d6,2b
	adda.l	44(sp),a2		| src pitch
	adda.l	a6,a3
3:	subq.w	#1,62(sp)		| rows (low word)
	bpl	1b
	movem.l	(sp)+,d2-d7/a2-a3/a6
#endif
	rts
