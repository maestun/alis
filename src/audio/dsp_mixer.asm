; =========================================================================
; DSP56001 8-Channel Sample Mixer
;
; Outputs mixed audio via SSI. Receives commands via host port.
; Loaded by NoCrew bootstrap at P:$0040.
;
; Commands (host port):
;   $1 = UPLOAD: slot(1), length(1), then length sample words
;   $2 = PLAY:   channel(1), slot(1), step(1), volume(1), loop(1)
;   $3 = STOP:   channel(1)
;
; X memory layout:
;   x:$0000-$0007  channel slot     x:$0008-$000F  channel step
;   x:$0010-$0017  channel volume   x:$0018-$001F  channel pos_lo
;   x:$0020-$0027  channel pos_hi   x:$0028-$002F  channel length
;   x:$0030-$0037  channel loop     x:$0038-$003F  channel active
;   x:$0040-$0043  slot base addr   x:$0044-$0047  slot length
;   x:$0048        mix accumulator  x:$0049        scratch (saved pos)
;
; Y memory layout:
;   y:$0100  slot 0 sample data (up to 8K words)
;   y:$2100  slot 1
;   y:$4100  slot 2
;   y:$6100  slot 3
; =========================================================================

PCC   equ $FFE1     ; Port C Control Register
PCDDR equ $FFE3     ; Port C Data Direction Register
PCD   equ $FFE5     ; Port C Data Register
HSR   equ $FFE9     ; Host Status Register (DSP side)
HRX   equ $FFEB     ; Host Receive Data Register (CPU→DSP)
HTX   equ $FFEB     ; Host Transmit Data Register (DSP→CPU)
CRA   equ $FFEC     ; SSI Control Register A
CRB   equ $FFED     ; SSI Control Register B
SSR   equ $FFEE     ; SSI Status Register (read) — bit 6 = TDE
TX    equ $FFEF     ; SSI TX Data Register

    org p:$0040     ; matches NoCrew bootstrap load address

    ; --- SSI init. MP2's CRB=$7808 puts SSI in network mode (MOD=1)
    ;     which uses time-slot multiplexing — wrong for our 1-write-per-
    ;     TDE mixer. Use CRB=$3800 (normal mode, TX+RE+SYN+FSL, slave
    ;     clocks) instead. Keep MP2's Port C bring-up to actually wire
    ;     SSI's pins to the matrix.
    movep   #$000,x:<<PCC          ; reset Port C
    movep   #$4100,x:<<CRA         ; 16-bit, DC=1 (2-word stereo frame)
    movep   #$3800,x:<<CRB         ; TE+RE+GCK+SYN+FSL, normal mode (MOD=0)
    movep   #$1E0,x:<<PCC          ; enable SC0/SC1/SCK/SRD/STD on Port C
    movep   #$010,x:<<PCDDR        ; SC0 direction
    movep   #$000,x:<<PCD          ; clear frame syncs

    ; Clear channel state x:$0000-$0049
    clr     a
    move    #$0000,r0
    move    #>$004A,b
clr_loop:
    move    a,x:(r0)+
    move    #>1,x0
    sub     x0,b
    tst     b
    jne     clr_loop

    ; Initialize slot base addresses (Y memory)
    move    #>$0100,a
    move    a,x:$0040
    move    #>$2100,a
    move    a,x:$0041
    move    #>$4100,a
    move    a,x:$0042
    move    #>$6100,a
    move    a,x:$0043

; =========================================================================
main_loop:
    ; --- SSI output (non-blocking, polled) ---
    btst    #6,x:<<SSR             ; TDE? (TX FIFO ready for next word)
    jeq     check_cmd

    ; Mix one sample by summing all active channels
    clr     a
    move    a,x:$0048              ; clear accumulator
    move    #>8,b

mix_ch:
    tst     b
    jeq     mix_done

    ; Channel index = 8 - b → use as n0
    move    #>8,a
    sub     b,a
    move    a1,n0

    ; Skip channel if !active (x:$0038 + n0)
    move    #$0038,r0
    nop
    move    x:(r0+n0),a
    tst     a
    jeq     mix_next

    ; Load position (pos_hi at x:$0020 + n0)
    move    #$0020,r0
    nop
    move    x:(r0+n0),a
    move    a1,x:$0049             ; save pos

    ; Load length (x:$0028 + n0)
    move    #$0028,r0
    nop
    move    x:(r0+n0),x0

    ; If pos >= length, stop/loop the channel
    move    x:$0049,a
    cmp     x0,a
    jge     mix_stop

    ; Look up slot base addr (x:$0040 + slot)
    move    #$0000,r0
    nop
    move    x:(r0+n0),a            ; slot number
    move    a1,n1
    move    #$0040,r1
    nop
    move    x:(r1+n1),r2           ; y base

    ; Read sample at position
    move    x:$0049,a
    move    a1,n2
    nop
    move    y:(r2+n2),x0

    ; Accumulate
    move    x:$0048,a
    add     x0,a
    move    a,x:$0048

    ; pos++
    move    #$0020,r0
    nop
    move    x:(r0+n0),a
    move    #>1,x0
    add     x0,a
    move    #$0020,r0
    nop
    move    a,x:(r0+n0)
    jmp     mix_next

mix_stop:
    ; Loop or deactivate
    move    #$0030,r0
    nop
    move    x:(r0+n0),a
    tst     a
    jeq     mix_deactivate
    move    #>1,x0
    sub     x0,a
    move    #$0030,r0
    nop
    move    a,x:(r0+n0)            ; loop--
    clr     a
    move    #$0020,r0
    nop
    move    a,x:(r0+n0)            ; pos = 0
    jmp     mix_next

mix_deactivate:
    clr     a
    move    #$0038,r0
    nop
    move    a,x:(r0+n0)            ; active = 0

mix_next:
    move    #>1,x0
    sub     x0,b
    jmp     mix_ch

mix_done:
    ; Send mixed sample to SSI TX
    move    x:$0048,a
    movep   a,x:<<TX

; =========================================================================
check_cmd:
    btst    #0,x:<<HSR             ; HRDF? (host wrote a word)
    jeq     main_loop

    movep   x:<<HRX,a              ; cmd byte
    move    #>1,x0
    cmp     x0,a
    jeq     cmd_upload
    move    #>2,x0
    cmp     x0,a
    jeq     cmd_play
    move    #>3,x0
    cmp     x0,a
    jeq     cmd_stop
    jmp     main_loop              ; unknown cmd — ignore

; --- UPLOAD: slot, length, sample-words ---
cmd_upload:
    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; slot number
    move    a1,n1
    move    #$0040,r1
    nop
    move    x:(r1+n1),r2           ; r2 = y dest base

    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; length
    move    a1,b
    move    #$0044,r1
    nop
    move    a,x:(r1+n1)            ; store slot length

    tst     b
    jeq     main_loop
upl_loop:
    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,y:(r2)+
    move    #>1,x0
    sub     x0,b
    tst     b
    jne     upl_loop
    jmp     main_loop

; --- PLAY: channel, slot, step, volume, loop ---
cmd_play:
    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; channel
    move    a1,n0

    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; slot
    move    #$0000,r0
    nop
    move    a,x:(r0+n0)

    ; copy slot length to channel length
    move    a1,n1
    move    #$0044,r1
    nop
    move    x:(r1+n1),a
    move    #$0028,r0
    nop
    move    a,x:(r0+n0)

    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; step
    move    #$0008,r0
    nop
    move    a,x:(r0+n0)

    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; volume
    move    #$0010,r0
    nop
    move    a,x:(r0+n0)

    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; loop
    move    #$0030,r0
    nop
    move    a,x:(r0+n0)

    ; reset position, activate
    clr     a
    move    #$0020,r0
    nop
    move    a,x:(r0+n0)
    move    #>1,a
    move    #$0038,r0
    nop
    move    a,x:(r0+n0)
    jmp     main_loop

; --- STOP: channel ---
cmd_stop:
    jclr    #0,x:<<HSR,*
    movep   x:<<HRX,a              ; channel
    move    a1,n0
    clr     a
    move    #$0038,r0
    nop
    move    a,x:(r0+n0)
    jmp     main_loop
