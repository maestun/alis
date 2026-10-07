// Raw IKBD keyboard handler for the native Atari backend.
//
// Hooked into `kbdvec` (the keyboard scancode vector at Kbdvbase()-4), so we get
// make/break scancodes straight from the IKBD ISR — bypassing Bconin and AES
// entirely (Bconin desyncs and returns phantom keys in a non-VDI video mode).
//
// The ISR calls us with the scancode in d0.b: make = 0x01..0x7f, break = bit 7
// set. We keep a held-key array (g_ikbd_pressed[128]) and the last key pressed
// (g_ikbd_cur); sys.c's io_inkey/io_shiftkey read those for SDL-equivalent
// held-key behaviour. Per the kbdvec convention only d0/d1/a0 are used (scratch).
//
// Modelled on ScummVM's backends/platform/atari/atari_ikbd.S.

#include "m68k_asm.h"

	.globl	SYM(atari_ikbd_kbdvec)
	.globl	SYM(atari_ikbd_mousevec)
	.globl	SYM(g_ikbd_pressed)
	.globl	SYM(g_ikbd_cur)
	.globl	SYM(g_mouse_dx)
	.globl	SYM(g_mouse_dy)
	.globl	SYM(g_mouse_buttons)
	.globl	SYM(atari_ikbd_joyvec)
	.globl	SYM(g_joy0)
	.globl	SYM(g_joy1)
	.globl	SYM(g_ikbd_new)
	.globl	SYM(g_mouse_down)

	.text
SYM(atari_ikbd_kbdvec):
	clr.l	d1
	move.b	d0,d1				| d1 = scancode (cc set from the byte)
	bmi.b	ikbd_break			| bit 7 set -> key release
	lea	SYM(g_ikbd_pressed),a0
	st.b	(a0,d1.l)			| pressed[scancode] = 0xff
	move.b	d1,SYM(g_ikbd_cur)		| remember last key pressed
	st.b	SYM(g_ikbd_new)			| ...until io_inkey has reported it once
	rts
ikbd_break:
	and.l	#0x7f,d1
	lea	SYM(g_ikbd_pressed),a0
	clr.b	(a0,d1.l)			| pressed[scancode] = 0
	rts

// IKBD relative-mouse packet handler, hooked into mousevec (Kbdvbase()->mousevec).
// The IKBD ISR calls us with a0 -> 3-byte packet: status (%111110rl, r=right
// button, l=left button), dx (signed), dy (signed). We accumulate the deltas
// into g_mouse_dx/dy (never zeroed: atari_mouse_vbl integrates them with a
// last-seen pattern) and latch the button bits. Only d0 is touched.
SYM(atari_ikbd_mousevec):
	move.l	d0,-(sp)
	move.b	(a0),d0				| status byte
	and.b	#3,d0				| keep right(bit0)+left(bit1)
	move.b	d0,SYM(g_mouse_buttons)
	or.b	d0,SYM(g_mouse_down)		| latch presses until the next poll
	move.b	1(a0),d0			| dx (signed)
	ext.w	d0
	add.w	d0,SYM(g_mouse_dx)
	move.b	2(a0),d0			| dy (signed)
	ext.w	d0
	add.w	d0,SYM(g_mouse_dy)
	move.l	(sp)+,d0
	rts

// IKBD joystick packet handler, hooked into joyvec (Kbdvbase()->joyvec).
// a0 -> [header 0xFE/0xFF, joystick 0, joystick 1]; byte = %f000rldu. While
// the mouse is on, only joystick 1 reports and its fire arrives as the right
// mouse button (mousevec), not bit 7 here.
SYM(atari_ikbd_joyvec):
	move.l	d0,-(sp)
	move.b	1(a0),d0
	and.b	#0x8f,d0
	move.b	d0,SYM(g_joy0)
	move.b	2(a0),d0
	and.b	#0x8f,d0
	move.b	d0,SYM(g_joy1)
	move.l	(sp)+,d0
	rts

	.bss
	.even
SYM(g_ikbd_pressed):
	ds.b	128
SYM(g_ikbd_cur):
	ds.b	1
SYM(g_ikbd_new):
	ds.b	1
SYM(g_mouse_down):
	ds.b	1
	.even
SYM(g_mouse_dx):
	ds.w	1
SYM(g_mouse_dy):
	ds.w	1
SYM(g_mouse_buttons):
	ds.b	1
SYM(g_joy0):
	ds.b	1
SYM(g_joy1):
	ds.b	1
