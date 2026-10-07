;***********************************************************************
;***********										 ***********
;***********										 ***********
;***********		Routine de Replay Soundtracker		 ***********
;***********			  2 + 2 * x  voies				 ***********
;***********										 ***********
;***********		 Transmission par port Host			 ***********
;***********		   Routine en Host Command			 ***********
;***********										 ***********
;***********		   Par Simplet / ABSTRACT			 ***********
;***********										 ***********
;***********************************************************************


PBC		equ		$ffe0			; Port B Control Register
PCC		equ		$ffe1			; Port C Control register
HCR		equ		$ffe8			; Host Control Register
HSR		equ		$ffe9			; Host Status Register
HRX		equ		$ffeb			; Host Receive Register
HTX		equ		$ffeb			; Host Transmit Register
CRA		equ		$ffec			; SSI Control Register A
CRB		equ		$ffed			; SSI Control Register B
SSISR	equ		$ffee			; SSI Status Register
TX		equ		$ffef			; SSI Serial Transmit data/shift register
IPR		equ		$ffff			; Interrupt Priority Register

;	Host Control Register Bit Flags

HCIE		equ		2				; Host Command Interrupt Enable

;	Host Status Register Bit Flags

HRDF		equ		0				; Host Receive Data Full
HTDE		equ		1				; Host Transmit Data Empty


		org		p:$0
		jmp		Start

		org		p:$10
		jsr		Spl_Out

		org		p:$26
		jsr		Soundtrack_Rout

		org		p:$28				; Pas de SSI
		movep	#$0800,X:<<IPR			; Host au niveau 2


; Routine sous interruption de Replay du son par le SSI

		org		p:$40

Spl_Out	jset		#2,X:<<SSISR,Right_Out	; D�tecte le premier transfert

Left_Out	movep	X:(r7),X:<<TX
		rti
Right_Out	movep	Y:(r7)+,X:<<TX
		rti

;
; Ca commence la :
;

Start	movep	#1,X:<<PBC			; Port B en Host
		movep	#$1f8,X:<<PCC			; Port C en SSI
		movep	#$4100,X:<<CRA			; 1 voie 16 bits Stereo
		movep	#$5800,X:<<CRB			; autorise l'IT d'�mission
		movep	#$3800,X:<<IPR			; le SSI au niveau d'IT 3
									; et le Host en IPL 2
		bset		#HCIE,X:<<HCR			; Autorise ITs Host Command


; Initialisations Registres

		movec		#-1,m0
		movec		#3699,m7
		movec		m0,m1
		movec		m7,m6
		move		#>1400,n4
		movec		m0,m4

; Efface le buffer
		clr		b
		move		#SampleBuffer,r0

		DO		#3700,Clear_Sample
		move		b,X:(r0)
		move		b,Y:(r0)+
Clear_Sample


; Pour v�rifier la connexion

Conct_Get	jclr		#HRDF,X:<<HSR,Conct_Get
		movep	X:<<HRX,x0

Conct_Snd	jclr		#HTDE,X:<<HSR,Conct_Snd
		movep	#12345678,X:<<HTX


		move		#SampleBuffer,r7

; Autorise les interruptions (IPL0)
		andi		#%11111100,mr

;
; Boucle principale qui ne fait strictement rien
;

Loop		jmp		<Loop

;
; Routine SoundTracker en Host Command
;

; Host protocol: Length, then an 8-bit Active_Mask, then per-voice data for
; ONLY the active voices (bit set), in voice order 0..7. We receive each active
; voice into its fixed per-voice slot (so its resample phase persists), CLEAR the
; output window, then MIX (accumulate) the active voices onto the cleared buffer.
; Inactive voices contribute nothing; a zero mask = pure silence (window cleared).
; Slots: voices 0/1 at 1400 (L=X / R=Y), 2/3 at 2800, 4/5 at 4200, 6/7 at 5600.

Soundtrack_Rout
		jclr		#HRDF,X:<<HSR,Soundtrack_Rout
		movep	X:<<HRX,a
		move		a1,X:<Length

Get_Mask	jclr		#HRDF,X:<<HSR,Get_Mask
		movep	X:<<HRX,a
		move		a1,X:<Active_Mask

		move		X:<CalcNext,x0
		move		x0,X:<Calc
		move		r7,X:<CalcNext

; --- Receive active voices into their slots (host I/O; only active ones) ---

		jclr		#0,X:<Active_Mask,RxV1
		move		#>1400,r0
		jsr		<Receive_Voice_Left
RxV1		jclr		#1,X:<Active_Mask,RxV2
		move		#>1400,r0
		jsr		<Receive_Voice_Right
RxV2		jclr		#2,X:<Active_Mask,RxV3
		move		#>2800,r0
		jsr		<Receive_Voice_Left
RxV3		jclr		#3,X:<Active_Mask,RxV4
		move		#>2800,r0
		jsr		<Receive_Voice_Right
RxV4		jclr		#4,X:<Active_Mask,RxV5
		move		#>4200,r0
		jsr		<Receive_Voice_Left
RxV5		jclr		#5,X:<Active_Mask,RxV6
		move		#>4200,r0
		jsr		<Receive_Voice_Right
RxV6		jclr		#6,X:<Active_Mask,RxV7
		move		#>5600,r0
		jsr		<Receive_Voice_Left
RxV7		jclr		#7,X:<Active_Mask,RxDone
		move		#>5600,r0
		jsr		<Receive_Voice_Right
RxDone

; --- Clear the output window [Calc..Calc+Length] in X (left) and Y (right) ---

		move		X:<Calc,r6
		clr		a
		do		X:<Length,ClrOut
		move		a,X:(r6)
		move		a,Y:(r6)+
ClrOut

; --- Mix active voices (accumulate onto the cleared buffer) ---

		jclr		#0,X:<Active_Mask,MxV1
		move		#>1400,r0
		jsr		<Mix_Voice_Left
MxV1		jclr		#1,X:<Active_Mask,MxV2
		move		#>1400,r0
		jsr		<Mix_Voice_Right
MxV2		jclr		#2,X:<Active_Mask,MxV3
		move		#>2800,r0
		jsr		<Mix_Voice_Left
MxV3		jclr		#3,X:<Active_Mask,MxV4
		move		#>2800,r0
		jsr		<Mix_Voice_Right
MxV4		jclr		#4,X:<Active_Mask,MxV5
		move		#>4200,r0
		jsr		<Mix_Voice_Left
MxV5		jclr		#5,X:<Active_Mask,MxV6
		move		#>4200,r0
		jsr		<Mix_Voice_Right
MxV6		jclr		#6,X:<Active_Mask,MxV7
		move		#>5600,r0
		jsr		<Mix_Voice_Left
MxV7		jclr		#7,X:<Active_Mask,MxDone
		move		#>5600,r0
		jsr		<Mix_Voice_Right
MxDone

		rti


; Routine de r�ception d'infos voie et sample � Gauche

Receive_Voice_Left
Receive_Left_Volume
		jclr		#HRDF,X:<<HSR,Receive_Left_Volume
		movep	X:<<HRX,X:(r0)+
Receive_Left_Frequence
		jclr		#HRDF,X:<<HSR,Receive_Left_Frequence
		movep	X:<<HRX,x0
		move		x0,X:(r0)+
		move		X:<Length,x1
		mpy		x0,x1,a		(r0)+
Send_Left_Length
		jclr		#HTDE,X:<<HSR,Send_Left_Length
		movep	a1,X:<<HTX
Receive_Left_Length
		jclr		#HRDF,X:<<HSR,Receive_Left_Length
		movep	X:<<HRX,b

		lsl		b	#>$80,y0
		move			#>$8000,y1

		DO		b1,Receive_Left_Loop
Receive_Left_Sample
		jclr		#HRDF,X:<<HSR,Receive_Left_Sample
		movep	X:<<HRX,x0
		mpy		x0,y0,a		x0,X:(r0)+
		mpy		x0,y1,a		a0,X:(r0)+
		move		a0,X:(r0)+
Receive_Left_Loop
		rts

; Routine de calcul de la premiere voie a Gauche

Calc_Voice_Left
		move		X:<Calc,r6		; Adresse
		move		X:(r0)+,x1		; Volume
		lua		(r0)+,r1
		move		X:(r0)+,y1		; Fr�quence
		move		X:(r1)+,b			; Fractionnaire

		move		X:(r1),x0			; Premier
		mpy		x0,x1,a			; Sample


		DO		X:<Length,Calc_Voice_Left_Loop

		add		y1,b
		jec		<CL_NoNext
		bclr		#23,b1
		move		X:(r1)+,x0
		mpy		x0,x1,a
CL_NoNext	move		a1,X:(r6)+

Calc_Voice_Left_Loop
		move		b,X:(r0)
		rts

; Routine de calcul d'une voie suppl�mentaire � Gauche

Mix_Voice_Left
		move		X:<Calc,r6		; Adresse
		move		X:(r0)+,x1		; Volume
		lua		(r0)+,r1
		move		X:(r0)+,y1		; Fr�quence
		move		X:(r1)+,b			; Fractionnaire

		move		X:(r1),x0			; Premier Sample


		DO		X:<Length,Mix_Voice_Left_Loop

		add		y1,b				X:(r6),a
		jec		<ML_NoNext
		bclr		#23,b1
		move		X:(r1)+,x0
ML_NoNext	mac		x0,x1,a
		move		a1,X:(r6)+

Mix_Voice_Left_Loop
		move		b,X:(r0)
		rts

; Routine de r�ception d'infos voie et sample � Droite

Receive_Voice_Right
Receive_Right_Volume
		jclr		#HRDF,X:<<HSR,Receive_Right_Volume
		movep	X:<<HRX,Y:(r0)+
Receive_Right_Frequence
		jclr		#HRDF,X:<<HSR,Receive_Right_Frequence
		movep	X:<<HRX,x0
		move		x0,Y:(r0)+
		move		X:<Length,x1
		mpy		x0,x1,a		(r0)+
Send_Right_Length
		jclr		#HTDE,X:<<HSR,Send_Right_Length
		movep	a1,X:<<HTX
Receive_Right_Length
		jclr		#HRDF,X:<<HSR,Receive_Right_Length
		movep	X:<<HRX,b

		lsl		b	#>$80,y0
		move			#>$8000,y1

		DO		b1,Receive_Right_Loop
Receive_Right_Sample
		jclr		#HRDF,X:<<HSR,Receive_Right_Sample
		movep	X:<<HRX,x0
		mpy		x0,y0,a		x0,Y:(r0)+
		mpy		x0,y1,a		a0,Y:(r0)+
		move		a0,Y:(r0)+
Receive_Right_Loop
		rts

; Routine de calcul de la premiere voie a Droite

Calc_Voice_Right
		move		X:<Calc,r6		; Adresse
		move		Y:(r0)+,x1		; Volume
		lua		(r0)+,r1
		move		Y:(r0)+,y1		; Fr�quence
		move		Y:(r1)+,b			; Fractionnaire

		move		Y:(r1),x0			; Premier
		mpy		x0,x1,a			; Sample


		DO		X:<Length,Calc_Voice_Right_Loop

		add		y1,b
		jec		<CR_NoNext
		bclr		#23,b1
		move		Y:(r1)+,x0
		mpy		x0,x1,a
CR_NoNext	move		a1,Y:(r6)+

Calc_Voice_Right_Loop
		move		b,Y:(r0)
		rts

; Routine de calcul d'une voie suppl�mentaire � Droite

Mix_Voice_Right
		move		X:<Calc,r6		; Adresse
		move		Y:(r0)+,x1		; Volume
		lua		(r0)+,r1
		move		Y:(r0)+,y1		; Fr�quence
		move		Y:(r1)+,b			; Fractionnaire

		move		Y:(r1),x0			; Premier Sample

		DO		X:<Length,Mix_Voice_Right_Loop

		add		y1,b			Y:(r6),a
		jec		<MR_NoNext
		bclr		#23,b1
		move		Y:(r1)+,x0
MR_NoNext	mac		x0,x1,a
		move		a1,Y:(r6)+

Mix_Voice_Right_Loop
		move		b,Y:(r0)
		rts

; Zone de donn�es

			org		X:0

Active_Mask	DC		0			; bit v set = voice v is active this frame
Length		DS		1
Calc			DC		SampleBuffer
CalcNext		DC		SampleBuffer

			org		X:1*1400
Voice_1
Voice_2
			org		X:2*1400
Voices_Sup

			org		Y:3*4096
SampleBuffer
			END
