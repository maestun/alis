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

/*
 * destofen_atari.S — planar sprite blitters for sprite_planar.c (destofen_blk in image_atari.h).
 *
 * Source: sprite rows of chunks (16 px), 8 or 4 plane words per chunk. Dest: 8-plane interleaved
 * framebuffer (16-byte chunks; 4-plane sprites write planes 0-3 and leave 4-7). Transparent =
 * colour 0: per chunk mask = OR of the planes, dst = (dst & ~mask) | src. A shifted dest chunk
 * is ((prev << 16) | cur) >> shift per plane; prev = 0 left of the sprite, cur = 0 right of it.
 *
 * Row strides: dst = drawn chunks * 16 + dst_rowrem; src = (aligned) nchunks + src_hskip,
 * (shifted) 1 + middle chunks + src_hskip source chunks.
 *
 * Registers: d0-d3 = result plane pairs (p0:p1 .. p6:p7), d4 = mask, d5/d7 = scratch,
 * d6 = shift, a0 = src, a1 = dst, a2 = P.
 */

#include "m68k_asm.h"

	.text

#if !defined(__mcoldfire__)

/* ---- stores: d0-d3 (8 planes) or d0-d1 (4 planes) to (a1), a1 += 16 ---- */

| transparent: skip empty chunks, plain store for full ones
.macro	STORE_T np
	move.l	d0,d4
	or.l	d1,d4
.if \np == 8
	or.l	d2,d4
	or.l	d3,d4
.endif
	move.l	d4,d5
	swap	d5
	or.w	d5,d4			| low word = union
	beq.s	Le\@
	cmp.w	#-1,d4
	beq.s	Lf\@
	move.w	d4,d5
	swap	d4
	move.w	d5,d4
	not.l	d4			| ~mask:~mask
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d0,d5
	move.l	d5,(a1)+
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d1,d5
	move.l	d5,(a1)+
.if \np == 8
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d2,d5
	move.l	d5,(a1)+
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d3,d5
	move.l	d5,(a1)+
.else
	addq.l	#8,a1
.endif
	bra.s	Ld\@
Lf\@:
	move.l	d0,(a1)+
	move.l	d1,(a1)+
.if \np == 8
	move.l	d2,(a1)+
	move.l	d3,(a1)+
.else
	addq.l	#8,a1
.endif
	bra.s	Ld\@
Le\@:
	lea	16(a1),a1
Ld\@:
.endm

| opaque
.macro	STORE_O np
	move.l	d0,(a1)+
	move.l	d1,(a1)+
.if \np == 8
	move.l	d2,(a1)+
	move.l	d3,(a1)+
.else
	addq.l	#8,a1
.endif
.endm

| opaque end chunk: only the bits in the mask word d4 (src is already 0 outside it)
.macro	STORE_M np
	move.w	d4,d5
	swap	d4
	move.w	d5,d4
	not.l	d4
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d0,d5
	move.l	d5,(a1)+
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d1,d5
	move.l	d5,(a1)+
.if \np == 8
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d2,d5
	move.l	d5,(a1)+
	move.l	(a1),d5
	and.l	d4,d5
	or.l	d3,d5
	move.l	d5,(a1)+
.else
	addq.l	#8,a1
.endif
.endm

/* ---- shifted plane pairs; the chunk at k-cs(a0) is prev (middle/trail) or cur (lead) ---- */

| middle: prev at k-cs(a0), cur at k(a0) -> dr = r0:r1
.macro	MID_PAIR k, cs, dr
	move.l	(\k-\cs)(a0),\dr	| q0:q1
	move.l	\k(a0),d5		| c0:c1
	move.w	\dr,d7
	swap	d7
	move.w	d5,d7			| q1:c1
	swap	d5
	move.w	d5,\dr			| q0:c0
	lsr.l	d6,\dr
	lsr.l	d6,d7
	swap	\dr
	move.w	d7,\dr
.endm

| leading chunk: prev = 0
.macro	LEAD_PAIR k, cs, dr
	move.l	(\k-\cs)(a0),\dr	| c0:c1
	move.l	\dr,d7
	lsr.w	d6,d7			| c1 >> s
	clr.w	\dr
	lsr.l	d6,\dr			| hi = c0 >> s
	move.w	d7,\dr
.endm

| trailing chunk: cur = 0
.macro	TRAIL_PAIR k, cs, dr
	move.l	(\k-\cs)(a0),\dr	| q0:q1
	move.l	\dr,d7
	swap	d7
	clr.w	d7
	lsr.l	d6,d7			| lo = (q1 << 16) >> s
	clr.w	\dr
	lsr.l	d6,\dr			| lo = (q0 << 16) >> s
	swap	\dr
	move.w	d7,\dr
.endm

.macro	PAIRS kind, np
.if \np == 8
	\kind	0, 16, d0
	\kind	4, 16, d1
	\kind	8, 16, d2
	\kind	12, 16, d3
.else
	\kind	0, 8, d0
	\kind	4, 8, d1
.endif
.endm

/* ---- aligned: one source chunk per dest chunk ---- */
| void k(const destofen_blk *P, const u16 *src, u16 *dst); 9 regs saved -> P at 40(sp)
.macro	ALIGNED name, np, opaque
	.globl	SYM(\name)
SYM(\name):
	movem.l	d2-d7/a2-a4,-(sp)
	movea.l	40(sp),a2
	movea.l	44(sp),a0
	movea.l	48(sp),a1
	moveq	#0,d0
	move.w	(a2),d0
	lsl.l	#4,d0
	movea.l	d0,a4			| row span (bytes)
	moveq	#0,d0
	move.w	4(a2),d0
	movea.l	d0,a3			| dst_rowrem
	moveq	#0,d7
	move.w	6(a2),d7		| src_hskip
.if \np == 8
	lsl.l	#4,d7
.else
	lsl.l	#3,d7
.endif
	move.w	2(a2),d6		| rows - 1
1:	lea	(a1,a4.l),a2		| row end
2:
.if \np == 8
	movem.l	(a0)+,d0-d3
.else
	movem.l	(a0)+,d0-d1
.endif
.if \opaque
	STORE_O	\np
.else
	STORE_T	\np
.endif
	cmpa.l	a2,a1
	bne.s	2b
	adda.l	a3,a1
	adda.l	d7,a0
	dbf	d6,1b
	movem.l	(sp)+,d2-d7/a2-a4
	rts
.endm

/* ---- shifted: lead (prev = 0) + middles + trail (cur = 0) ---- */
| locals: 0(sp) dst stride, 4(sp) src stride, 8(sp) src start, 12(sp) middle bytes;
| 11 regs saved + 16 -> P at 64(sp)
.macro	SHIFTED name, np, opaque
	.globl	SYM(\name)
SYM(\name):
	movem.l	d2-d7/a2-a6,-(sp)
	lea	-16(sp),sp
	movea.l	64(sp),a2
	movea.l	68(sp),a3		| row src
	movea.l	72(sp),a4		| row dst
	moveq	#0,d6
	move.w	14(a2),d6		| shift
	moveq	#0,d1
	move.w	(a2),d1			| nchunks
	move.l	d1,d2
.if \opaque
	addq.l	#2,d2			| opaque: nchunks = middles, rowrem counts lead + trail
.else
	tst.b	16(a2)
	bne.s	3f
	subq.l	#1,d1			| transparent: nchunks = lead + middles
3:
.endif
	move.l	d1,d3
	lsl.l	#4,d3
	move.l	d3,12(sp)		| middle bytes
	lsl.l	#4,d2
	moveq	#0,d3
	move.w	4(a2),d3
	add.l	d3,d2
	move.l	d2,0(sp)		| dst stride
	moveq	#0,d3
	move.w	6(a2),d3
	add.l	d1,d3
	addq.l	#1,d3
.if \np == 8
	lsl.l	#4,d3
.else
	lsl.l	#3,d3
.endif
	move.l	d3,4(sp)		| src stride
	moveq	#0,d3
	move.w	8(a2),d3
	addq.l	#1,d3
.if \np == 8
	lsl.l	#4,d3
.else
	lsl.l	#3,d3
.endif
	move.l	d3,8(sp)		| first middle's cur
	moveq	#0,d3
	move.w	2(a2),d3
	addq.l	#1,d3
	mulu.w	d2,d3
	lea	(a4,d3.l),a6		| end of rows
1:	movea.l	a3,a0
	adda.l	8(sp),a0
	movea.l	a4,a1
.if !\opaque
	tst.b	16(a2)
	bne	2f
.endif
	PAIRS	LEAD_PAIR, \np
.if \opaque
	move.w	10(a2),d4
	STORE_M	\np
.else
	STORE_T	\np
.endif
2:	movea.l	a1,a5
	adda.l	12(sp),a5
	cmpa.l	a5,a1
	beq	5f
3:	PAIRS	MID_PAIR, \np
	lea	(\np*2)(a0),a0
.if \opaque
	STORE_O	\np
.else
	STORE_T	\np
.endif
	cmpa.l	a5,a1
	bne	3b
5:
.if !\opaque
	tst.b	17(a2)
	bne	6f
.endif
	PAIRS	TRAIL_PAIR, \np
.if \opaque
	move.w	12(a2),d4
	STORE_M	\np
.else
	STORE_T	\np
.endif
6:	adda.l	0(sp),a4
	adda.l	4(sp),a3
	cmpa.l	a6,a4
	bne	1b
	lea	16(sp),sp
	movem.l	(sp)+,d2-d7/a2-a6
	rts
.endm

	ALIGNED	nd_8bpp_aligned, 8, 0
	ALIGNED	nd_8bpp_opaque_aligned, 8, 1
	ALIGNED	nd_4bpp_aligned, 4, 0
	ALIGNED	nd_4bpp_opaque_aligned, 4, 1
	SHIFTED	nd_8bpp_shifted, 8, 0
	SHIFTED	nd_8bpp_opaque_shifted, 8, 1
	SHIFTED	nd_4bpp_shifted, 4, 0
	SHIFTED	nd_4bpp_opaque_shifted, 4, 1

#endif
