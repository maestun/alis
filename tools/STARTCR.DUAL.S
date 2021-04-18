; -----------------------------
; ISHAR LEGEND OF THE FORTRESS
; ATARI ST
; START.PRG - CRACKED VERSION
; DISASM'ED WITH EASYRIDER4
; -----------------------------
 TEXT 
Begin:

; =============================================================================
; ENCRYPTED CRACK CODE (intro screen + crack)
; THIS CODE WILL BE DECRYPTED AT RUNTIME
; THE LAST INSTRUCTION WILL JUMP @FN_START
; =============================================================================
00AA9A  41FA0022                  LEA       34(PC),A0            DATA_ENCRYPTED_CRACK
00AA9E  203C00000168              MOVE.L    #$168,D0             ; $168: size of encrypted data
00AAA4  44FC001F                  MOVE      #$1F,CCR             ; set X N Z V C bits
00AAA8  600C                      BRA.S     12(PC)               __DECRYPT_CRACK_START
00AAAA  323008FE            __DECRYPT_CRACK_LOOP:
                                  MOVE.W    -2(A0,D0.L),D1       
00AAAE  B541                      EOR.W     D2,D1       
00AAB0  318108FE                  MOVE.W    D1,-2(A0,D0.L)       
00AAB4  5580                      SUBQ.L    #2,D0
00AAB6  40C2                __DECRYPT_CRACK_START:
                                  MOVE      SR,D2
00AAB8  B142                      EOR.W     D0,D2
00AABA  4A80                      TST.L     D0
00AABC  66EC                      BNE.S     -20(PC)              __DECRYPT_CRACK_LOOP


; --- crack commence à $aabe:
; push adresse $aae6 et appelle Supexec (XBIOS), puis corrige pile
; hide mouse
; push adresse $ab32 puis appelle Cconws (GEMDOS) pour afficher le texte à afficher (242 bytes), puis corrige pile
; show mouse
; appelle Crawin (GEMDOS) pour attendre touche clavier, puis corrige pile
; branchement à $ac24 => instruction jmp FN_START
; => le crack est donc à l'adresse $aae6 -> $ab32, le voici:

;  move sr,-(a7)                                    ; 00AAE6: 40E7 
;  ori.w #$700,sr                                   ; 00AAE8: 007C 0700 
;  movem.l +$26(pc),d0-6 {$00AB16}                  ; 00AAEC: 4CFA 007F 0026 
;  movem.l d0-6,$140.W                              ; 00AAF2: 48F8 007F 0140 
;  move (a7)+,sr                                    ; 00AAF8: 46DF 
;  lea +$5128(pc),a0 {$00FC24}                      ; 00AAFA: 41FA 5128 
;  move.l #$303c0001,+$d4e(a0)                      ; 00AAFE: 217C 303C 0001 0D4E 
;  move.w #$4e40,+$5e8e(a0)                         ; 00AB06: 317C 4E40 5E8E 
;  move.l #$140,$80.w                               ; 00AB0C: 21FC 0000 0140 0080 
;  rts                                              ; 00AB14: 4E75 
;  cmpi.w #$41,d7                                   ; 00AB16: 0C47 0041 
;  blt.s +18 {$00AB2E}                              ; 00AB1A: 6D12 
;  cmpi.w #$5a,d7                                   ; 00AB1C: 0C47 005A 
;  bgt.s +12 {$00AB2E}                              ; 00AB20: 6E0C 
;  cmpi.w #$58,(a4)                                 ; 00AB22: 0C54 0058 
;  bne.s +6 {$00AB2E}                               ; 00AB26: 6606 
;  addq.w #2,a4                                     ; 00AB28: 544C 
;  move.w d7,d6                                     ; 00AB2A: 3C07 
;  rte                                              ; 00AB2C: 4E73 
;  move.w (a4)+,d6                                  ; 00AB2E: 3C1C 
;  rte                                              ; 00AB30: 4E73 



;00AAAA  323008FEB5413181          DC.B      $32,$30,$08,$FE,$B5,$41,$31,$81 ; crack decrunch code ?
;00AAB2  08FE5580                  DC.B      $08,$FE,$55,$80 
;00AAB6  40C2B1424A8066EC    __L0000:
;                                  DC.B      $40,$C2,$B1,$42,$4A,$80,$66,$EC 
00AABE  4B7803223C3A032E    DATA_ENCRYPTED_CRACK:
                                  DC.B      $4B,$78,$03,$22,$3C,$3A,$03,$2E 
00AAC6  4D445F83A3044B6A          DC.B      $4D,$44,$5F,$83,$A3,$04,$4B,$6A 
00AACE  03763C28031F4D59          DC.B      $03,$76,$3C,$28,$03,$1F,$4D,$59 
00AAD6  5F95A3153C220327          DC.B      $5F,$95,$A3,$15,$3C,$22,$03,$27 
00AADE  4D6357AB63260268          DC.B      $4D,$63,$57,$AB,$63,$26,$02,$68 
00AAE6  43CD0350042E4FCA          DC.B      $43,$CD,$03,$50,$04,$2E,$4F,$CA ; code executé en mode supexec
00AAEE  034D03124BCE0347          DC.B      $03,$4D,$03,$12,$4B,$CE,$03,$47 
00AAF6  027A45E342C45268          DC.B      $02,$7A,$45,$E3,$42,$C4,$52,$68 
00AAFE  223E337803470E06          DC.B      $22,$3E,$33,$78,$03,$47,$0E,$06 
00AB06  32364D0C5DC022AC          DC.B      $32,$36,$4D,$0C,$5D,$C0,$22,$AC 
00AB0E  0352021403D64D2D          DC.B      $03,$52,$02,$14,$03,$D6,$4D,$2D 
00AB16  0F1D031D6E4C0F27          DC.B      $0F,$1D,$03,$1D,$6E,$4C,$0F,$27 
00AB1E  03386D680F320330          DC.B      $03,$38,$6D,$68,$0F,$32,$03,$30 
00AB26  656C57203F694D03          DC.B      $65,$6C,$57,$20,$3F,$69,$4D,$03 
00AB2E  3F6E4D0718330972          DC.B      $3F,$6E,$4D,$07,$18,$33,$09,$72 
00AB36  0970235C235E23A0          DC.B      $09,$70,$23,$5C,$23,$5E,$23,$A0 
00AB3E  23A223A423A646C4          DC.B      $23,$A2,$23,$A4,$23,$A6,$46,$C4 
00AB46  4ADE46AC53DC46C3          DC.B      $4A,$DE,$46,$AC,$53,$DC,$46,$C3 
00AB4E  46DC5799099C09B8          DC.B      $46,$DC,$57,$99,$09,$9C,$09,$B8 
00AB56  23BA23BC23BE2380          DC.B      $23,$BA,$23,$BC,$23,$BE,$23,$80 
00AB5E  4AF14BE5518646C6          DC.B      $4A,$F1,$4B,$E5,$51,$86,$46,$C6 
00AB66  64C66ADF6B8E75D5          DC.B      $64,$C6,$6A,$DF,$6B,$8E,$75,$D5 
00AB6E  71C16ADB6DBB09B2          DC.B      $71,$C1,$6A,$DB,$6D,$BB,$09,$B2 
00AB76  239A239C239E23E0          DC.B      $23,$9A,$23,$9C,$23,$9E,$23,$E0 
00AB7E  23815185408D468C          DC.B      $23,$81,$51,$85,$40,$8D,$46,$8C 
00AB86  23885AEC578646F0          DC.B      $23,$88,$5A,$EC,$57,$86,$46,$F0 
00AB8E  4EFC40FA42F80ED2          DC.B      $4E,$FC,$40,$FA,$42,$F8,$0E,$D2 
00AB96  45884692409623A3          DC.B      $45,$88,$46,$92,$40,$96,$23,$A3 
00AB9E  51A340AF23A45AC8          DC.B      $51,$A3,$40,$AF,$23,$A4,$5A,$C8 
00ABA6  4FC450C247C02BBD          DC.B      $4F,$C4,$50,$C2,$47,$C0,$2B,$BD 
00ABAE  42B646D44AA223BD          DC.B      $42,$B6,$46,$D4,$4A,$A2,$23,$BD 
00ABB6  42A94AB951D70F0A          DC.B      $42,$A9,$4A,$B9,$51,$D7,$0F,$0A 
00ABBE  082222242251435A          DC.B      $08,$22,$22,$24,$22,$51,$43,$5A 
00ABC6  4C434C4B2C202331          DC.B      $4C,$43,$4C,$4B,$2C,$20,$23,$31 
00ABCE  22746D6622626A7D          DC.B      $22,$74,$6D,$66,$22,$62,$6A,$7D 
00ABD6  224A636F71696D52          DC.B      $22,$4A,$63,$6F,$71,$69,$6D,$52 
00ABDE  662F080422062208          DC.B      $66,$2F,$08,$04,$22,$06,$22,$08 
00ABE6  220A6D426E572244          DC.B      $22,$0A,$6D,$42,$6E,$57,$22,$44 
00ABEE  7B426714256E5A60          DC.B      $7B,$42,$67,$14,$25,$6E,$5A,$60 
00ABF6  5A625A645A665A67          DC.B      $5A,$62,$5A,$64,$5A,$66,$5A,$67 
00ABFE  0F48084E22662268          DC.B      $0F,$48,$08,$4E,$22,$66,$22,$68 
00AC06  4518470956074C17          DC.B      $45,$18,$47,$09,$56,$07,$4C,$17 
00AC0E  5172561B22174E14          DC.B      $51,$72,$56,$1B,$22,$17,$4E,$14 
00AC16  2215570E22185029          DC.B      $22,$15,$57,$0E,$22,$18,$50,$29 
00AC1E  472C463702664C8E          DC.B      $47,$2C,$46,$37,$02,$66,$4C,$8E 
00AC26  00011016                  DC.L      $00011016            FN_START
00AC2A  4E75                __L0002:
                                  RTS                           
00AC2C  4E75                __L0003:
                                  RTS                           



00AC2E                      ADDR_INITIAL_STACK:
                                  DCB.W     2,0 
00AC32                      L0005:DCB.W     8,0 
00AC42                      L0006:DCB.W     2,0 
00AC46                      L0007:DCB.W     2,0 
00AC4A                      L0008:DCB.W     2,0 
00AC4E                      L0009:DCB.W     2,0 
00AC52                      L000A:DCB.W     2,0 
00AC56  0000                L000B:DC.B      $00,$00 
00AC58  0000                __L000C:
                                  DC.B      $00,$00 
00AC5A                      __L000D:
                                  DCB.W     2,0 
00AC5E                      L000E:DCB.W     2,0 
00AC62                      L000F:DCB.W     2,0 
00AC66                      __L0010:
                                  DCB.W     2,0 
00AC6A                      L0011:DCB.W     2,0 
00AC6E                      L0012:DCB.W     2,0 
00AC72  0000                L0013:DC.B      $00,$00 
00AC74  0000                W_DRIVE_NUMBER:
                                  DC.B      $00,$00 
00AC76  00                  L0015:DC.B      $00 
00AC77  00                  L0016:DC.B      $00 
00AC78                      L0017:DCB.W     2,0 
00AC7C  5C00                L0018:DC.B      $5C,$00 
00AC7E                      L0019:DCB.W     2,0 
00AC82  00                  L001A:DC.B      $00 
00AC83  00                  L001B:DC.B      $00 
00AC84                      L001C:DCB.W     2,0 
00AC88  0000                L001D:DC.B      $00,$00 
00AC8A  0000                L001E:DC.B      $00,$00 
00AC8C  0000                L001F:DC.B      $00,$00 
00AC8E  0000077705550555    __L0020:
                                  DC.B      $00,$00,$07,$77,$05,$55,$05,$55 
00AC96  0555055505550555          DC.B      $05,$55,$05,$55,$05,$55,$05,$55 
00AC9E  0555055505550555          DC.B      $05,$55,$05,$55,$05,$55,$05,$55 
00ACA6  0000007007000770          DC.B      $00,$00,$00,$70,$07,$00,$07,$70 
00ACAE  0000077705550555    __L0021:
                                  DC.B      $00,$00,$07,$77,$05,$55,$05,$55 
00ACB6  0555055505550555          DC.B      $05,$55,$05,$55,$05,$55,$05,$55 
00ACBE  0555055505550555          DC.B      $05,$55,$05,$55,$05,$55,$05,$55 
00ACC6  0000007707070777          DC.B      $00,$00,$00,$77,$07,$07,$07,$77 
00ACCE  0000070007770770    __L0022:
                                  DC.B      $00,$00,$07,$00,$07,$77,$07,$70 
00ACD6  0007011100770070          DC.B      $00,$07,$01,$11,$00,$77,$00,$70 
00ACDE  0404040004440420          DC.B      $04,$04,$04,$00,$04,$44,$04,$20 
00ACE6  0004000000440040          DC.B      $00,$04,$00,$00,$00,$44,$00,$40 
00ACEE  077707000060        L0023:DC.B      $07,$77,$07,$00,$00,$60 
00ACF4  0007000505200050    __L0024:
                                  DC.B      $00,$07,$00,$05,$05,$20,$00,$50 
00ACFC  0666055502220077          DC.B      $06,$66,$05,$55,$02,$22,$00,$77 
00AD04  0055070705050550          DC.B      $00,$55,$07,$07,$05,$05,$05,$50 
00AD0C                      __L0025:
                                  DCB.W     49,0
00AD6E                      L0026:DCB.W     64,0
00ADEE                      L0027:DCB.W     16,0
00AE0E                      L0028:DCB.W     2,0 
00AE12                      __L0029:
                                  DCB.W     2,0 
00AE16                      __L002A:
                                  DCB.W     2,0 
00AE1A                      L002B:DCB.W     16,0
00AE3A                      L002C:DCB.W     2,0 
00AE3E  00FF                __L002D:
                                  DC.B      $00,$FF 
00AE40  000000FF000000FF    __L002E:
                                  DC.B      $00,$00,$00,$FF,$00,$00,$00,$FF 
00AE48  0000                      DC.B      $00,$00 
00AE4A  00FF0000            __L002F:
                                  DC.B      $00,$FF,$00,$00 
00AE4E  FFFFFFFF00000000    L0030:DC.B      $FF,$FF,$FF,$FF,$00,$00,$00,$00 
00AE56                            DCB.W     126,0 
00AF52                      BUF_PATH:
                                  DCB.W     32,0


; =============================================================================
; TODO: convert to code !!!
00AF92  2079                FN_INIT_1:
                                  DC.B      $20,$79 
00AF94  0000AC2E                  DC.L      $0000AC2E            ADDR_INITIAL_STACK
00AF98  2A680004DBFC0000          DC.B      $2A,$68,$00,$04,$DB,$FC,$00,$00 
00AFA0  008023CD                  DC.B      $00,$80,$23,$CD 
00AFA4  0000AC32                  DC.L      $0000AC32            L0005
00AFA8  203C                      DC.B      $20,$3C 
00AFAA  00018054                  DC.L      $00018054            BUF_L072F_1028
00AFAE  0080000000FF5280          DC.B      $00,$80,$00,$00,$00,$FF,$52,$80 
00AFB6  23C0                      DC.B      $23,$C0 
00AFB8  0000AC52                  DC.L      $0000AC52            L000A
00AFBC  23C0                      DC.B      $23,$C0 
00AFBE  0000AC5E                  DC.L      $0000AC5E            L000E
00AFC2  23C0                      DC.B      $23,$C0 
00AFC4  0000AC5A                  DC.L      $0000AC5A            __L000D
00AFC8  068000007D0023C0          DC.B      $06,$80,$00,$00,$7D,$00,$23,$C0 
00AFD0  0000AC78                  DC.L      $0000AC78            L0017
00AFD4  06800000020023C0          DC.B      $06,$80,$00,$00,$02,$00,$23,$C0 
00AFDC  00017062                  DC.L      $00017062            L065C
00AFE0  3F3C00024E4E548F          DC.B      $3F,$3C,$00,$02,$4E,$4E,$54,$8F 
00AFE8  23C0                      DC.B      $23,$C0 
00AFEA  0000AC4E                  DC.L      $0000AC4E            L0009
00AFEE  23C0                      DC.B      $23,$C0 
00AFF0  0000AC4A                  DC.L      $0000AC4A            L0008
00AFF4  23C0                      DC.B      $23,$C0 
00AFF6  0000AC56                  DC.L      $0000AC56            L000B
00AFFA  04800000040023C0          DC.B      $04,$80,$00,$00,$04,$00,$23,$C0 
00B002  00017066                  DC.L      $00017066            L065D
00B006  A00023C8                  DC.B      $A0,$00,$23,$C8 
00B00A  0000AC62                  DC.L      $0000AC62            L000F
00B00E  23E80008                  DC.B      $23,$E8,$00,$08 
00B012  0000AC66                  DC.L      $0000AC66            __L0010
00B016  23E8000C                  DC.B      $23,$E8,$00,$0C 
00B01A  0000AC6A                  DC.L      $0000AC6A            L0011
00B01E  23FC                      DC.B      $23,$FC 
00B020  0000BE5A                  DC.L      $0000BE5A            L009C
00B024  0000AC6E                  DC.L      $0000AC6E            L0012
00B028  A00A                      DC.B      $A0,$0A              ; LINE-A: hide mouse

00B02A  2F3C0000B13A              MOVE.L    #$0000B13A,-(A7)     EXCEPTION_VECTOR_18
00B030  3F3C0018                  MOVE.W    #$18,-(A7)           
00B034  3F3C0005                  MOVE.W    #5,-(A7)             ; Setexc - Set Exception Vector
00B038  4E4D                      TRAP      #$D                  ; BIOS                 
00B03A  508F                      ADDQ.L    #8,A7               
00B03C  2F3C0000B13C              MOVE.L    #$0000B13C,-(A7)     EXCEPTION_VECTOR_45
00B042  3F3C0045                  MOVE.W    #$45,-(A7)          
00B046  3F3C0005                  MOVE.W    #5,-(A7)             ; Setexc - Set Exception Vector
00B04A  4E4D                      TRAP      #$D                  ; BIOS                 
00B04C  508F                      ADDQ.L    #8,A7               
00B04E  23C00000AC42              MOVE.L    D0,$0000AC42         L0006
00B054  13FC00010000AC77          MOVE.B    #1,$0000AC77         L0016
00B05C  42A7                      CLR.L     -(A7)               
00B05E  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B062  4E41                      TRAP      #1     				 ; GEMDOS                  
00B064  DFFC00000006              ADDA.L    #6,A7               
00B06A  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00B070  1E3900000025              MOVE.B    $25.L,D7            
00B076  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00B07C  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B080  4E41                      TRAP      #1     				 ; GEMDOS                  
00B082  DFFC00000006              ADDA.L    #6,A7               
00B088  4A07                      TST.B     D7                  
00B08A  6A000008                  BPL       8(PC)                __init_1
00B08E  42390000AC77              CLR.B     $0000AC77            L0016
00B094  4EB90000B14C        __init_1:
                                  JSR       $0000B14C            __L0038 ; nothing, just rts
00B09A  4EB90000B6A6              JSR       $0000B6A6            FN_INIT_1_1
00B0A0  4EB90000B282              JSR       $0000B282            __L0041
00B0A6  4EB90000EFA2              JSR       $0000EFA2            __L026A
00B0AC  4EB90000B582              JSR       $0000B582            __L0050
00B0B2  4EB90000B14E              JSR       $0000B14E            __L0039
00B0B8  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00B0BC  3F3C0019                  MOVE.W    #$19,-(A7)           ; Dgetdrv - Obtain the current drive
00B0C0  4E41                      TRAP      #1     				 ; GEMDOS                  
00B0C2  548F                      ADDQ.L    #2,A7                
00B0C4  33C00000AC74              MOVE.W    D0,$0000AC74         W_DRIVE_NUMBER
00B0CA  3F3C0000                  MOVE.W    #0,-(A7)             ; ignored :p TODO: change this for other drives ?
00B0CE  2F3C0000AF52              MOVE.L    #$0000AF52,-(A7)     BUF_PATH
00B0D4  3F3C0047                  MOVE.W    #$47,-(A7)           ; Dgetpath - Obtain current path specification
00B0D8  4E41                      TRAP      #1     				 ; GEMDOS                  
00B0DA  508F                      ADDQ.L    #8,A7               
00B0DC  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00B0E0  4E75                      RTS                           


; =============================================================================
00B0E2  4EB90000B60C        SYS_TERMINATE_PROGRAM:
                                  JSR       $0000B60C            __L0052
00B0E8  4EB90000EFEE              JSR       $0000EFEE            __L026B
00B0EE  4EB90000B6C8              JSR       $0000B6C8            __L0059
00B0F4  4EB90000B33E              JSR       $0000B33E            __L0042
00B0FA  4AB9                      DC.B      $4A,$B9 
00B0FC  0000AC42                  DC.L      $0000AC42            L0006
00B100  67000014                  BEQ       20(PC)               __terminate
00B104  2F390000AC42              MOVE.L    $0000AC42,-(A7)      L0006
00B10A  3F3C0045                  MOVE.W    #$45,-(A7)           ; restore $45 exception vector
00B10E  3F3C0005                  MOVE.W    #5,-(A7)             ; Setexc - Set Exception Vector
00B112  4E4D                      TRAP      #$D                  ; BIOS                 
00B114  508F                      ADDQ.L    #8,A7               
00B116  A009                __terminate:
                                  DC.B      $A0,$09              ; LINE-A - Show mouse
00B118  3F3CFFFF                  MOVE.W    #$FFFF,-(A7)        
00B11C  2F390000AC4A              MOVE.L    $0000AC4A,-(A7)      L0008
00B122  2F390000AC4A              MOVE.L    $0000AC4A,-(A7)      L0008
00B128  3F3C0005                  MOVE.W    #5,-(A7)             ; Setscreen - Initialize Screen
00B12C  4E4E                      TRAP      #$E                  ; XBIOS                 
00B12E  DFFC0000000C              ADDA.L    #$C,A7              
00B134  4267                      CLR.W     -(A7)                ; Pterm0 - Terminate program
00B136  4E41                      TRAP      #1     				 ; GEMDOS                  
00B138  4E75                      RTS                           

00B13A  4E73                EXCEPTION_VECTOR_18:
                                  RTE                           

00B13C  52B9000004BA        EXCEPTION_VECTOR_45:
                                  ADDQ.L    #1,$4BA.L            ; Counter for 200hz system clock
00B142  08B9000500FFFA11          BCLR      #5,$FFFA11.L         ; Interrupt In-service B
00B14A  4E73                      RTE

00B14C  4E75                __L0038:
                                  RTS                           
00B14E  2F3CFFFFFFFF        __L0039:
                                  MOVE.L    #$FFFFFFFF,-(A7)    
00B154  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B15A  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B160  3F3C0010                  MOVE.W    #$10,-(A7)           ; Keytbl - Define the mapping table for the keyboard scancodes to key-codes   
00B164  4E4E                      TRAP      #$E     			 ; XBIOS                 
00B166  DFFC0000000E              ADDA.L    #$E,A7              
00B16C  2040                      MOVEA.L   D0,A0               
00B16E  2250                      MOVEA.L   (A0),A1             
00B170  10290010                  MOVE.B    16(A1),D0           
00B174  123C0000                  MOVE.B    #0,D1               
00B178  B03C0061                  CMP.B     #$61,D0             ;(#"a") 
00B17C  6700003C                  BEQ       60(PC)               L003A
00B180  123C0002                  MOVE.B    #2,D1               
00B184  10290015                  MOVE.B    21(A1),D0           
00B188  B03C007A                  CMP.B     #$7A,D0             ;(#"z") 
00B18C  6700002C                  BEQ       44(PC)               L003A
00B190  22680004                  MOVEA.L   4(A0),A1            
00B194  123C0003                  MOVE.B    #3,D1               
00B198  10290033                  MOVE.B    51(A1),D0           
00B19C  B03C003F                  CMP.B     #$3F,D0             ;(#"?") 
00B1A0  67000018                  BEQ       24(PC)               L003A
00B1A4  2250                      MOVEA.L   (A0),A1             
00B1A6  123C0001                  MOVE.B    #1,D1               
00B1AA  10290015                  MOVE.B    21(A1),D0           
00B1AE  B03C0079                  CMP.B     #$79,D0             ;(#"y") 
00B1B2  67000006                  BEQ       6(PC)                L003A
00B1B6  123C0001                  MOVE.B    #1,D1               
00B1BA  13C100017071        L003A:MOVE.B    D1,$00017071         L0663
00B1C0  4E75                      RTS                           


00B1C2  43F900FF8207        __L003B:
                                  LEA       $FF8207.L,A1         ; Video address pointer (Mid byte)
00B1C8  4228001A                  CLR.B     26(A0)              
00B1CC  117C00FF0020              MOVE.B    #$FF,32(A0)         
00B1D2  223C00007D00              MOVE.L    #$7D00,D1           
00B1D8  D2B90000AC56              ADD.L     $0000AC56,D1         L000B
00B1DE  30390000AC8C              MOVE.W    $0000AC8C,D0         L001F
00B1E4  B0790000AC8C        __L003C:
                                  CMP.W     $0000AC8C,D0         L001F
00B1EA  6700FFF8                  BEQ       -8(PC)               __L003C
00B1EE  117C0007001A              MOVE.B    #7,26(A0)           
00B1F4  01090000            __L003D:
                                  MOVEP.W   0(A1),D0            
00B1F8  B041                      CMP.W     D1,D0               
00B1FA  6B00FFF8                  BMI       -8(PC)               __L003D
00B1FE  18280020                  MOVE.B    32(A0),D4           
00B202  4404                      NEG.B     D4                  
00B204  060400FF                  ADDI.B    #$FF,D4             
00B208  4228001A                  CLR.B     26(A0)              
00B20C  117C00FF0020              MOVE.B    #$FF,32(A0)         
00B212  223C00003E80              MOVE.L    #$3E80,D1           
00B218  D2B90000AC56              ADD.L     $0000AC56,D1         L000B
00B21E  30390000AC8C              MOVE.W    $0000AC8C,D0         L001F
00B224  B0790000AC8C        __L003E:
                                  CMP.W     $0000AC8C,D0         L001F
00B22A  6700FFF8                  BEQ       -8(PC)               __L003E
00B22E  117C0007001A              MOVE.B    #7,26(A0)           
00B234  01090000            __L003F:
                                  MOVEP.W   0(A1),D0            
00B238  B041                      CMP.W     D1,D0               
00B23A  6B00FFF8                  BMI       -8(PC)               __L003F
00B23E  1A280020                  MOVE.B    32(A0),D5           
00B242  4405                      NEG.B     D5                  
00B244  060500FF                  ADDI.B    #$FF,D5             
00B248  4240                      CLR.W     D0                  
00B24A  1005                      MOVE.B    D5,D0               
00B24C  D040                      ADD.W     D0,D0               
00B24E  9004                      SUB.B     D4,D0               
00B250  33C00000AC8A              MOVE.W    D0,$0000AC8A         L001E
00B256  4280                      CLR.L     D0                  
00B258  1004                      MOVE.B    D4,D0               
00B25A  90790000AC8A              SUB.W     $0000AC8A,D0         L001E
00B260  E148                      LSL.W     #8,D0               
00B262  80FC00C8                  DIVU      #$C8,D0             
00B266  33C00000AC88              MOVE.W    D0,$0000AC88         L001D
00B26C  53790000AC88              SUBQ.W    #1,$0000AC88         L001D
00B272  4E75                      RTS                           
00B274  52790000AC8C        __L0040:
                                  ADDQ.W    #1,$0000AC8C         L001F
00B27A  2F390000AC46              MOVE.L    $0000AC46,-(A7)      L0007
00B280  4E75                      RTS                           



00B282  42390001718A        __L0041:
                                  
                                  CLR.B     $0001718A            L06DB
00B288  42390001718B              CLR.B     $0001718B            L06DC
00B28E  423900017189              CLR.B     $00017189            L06DA
00B294  4239000170DF              CLR.B     $000170DF            L0691
00B29A  4EB90000B4DA              JSR       $0000B4DA            L004B
00B2A0  42A7                      CLR.L     -(A7)               
00B2A2  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B2A6  4E41                      TRAP      #1     				       ; GEMDOS                  
00B2A8  DFFC00000006              ADDA.L    #6,A7               
00B2AE  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00B2B4  007C0700                  ORI.W     #$700,SR            
00B2B8  23F9000000700000AC46      MOVE.L    $70.L,$0000AC46      L0007
00B2C2  23FC0000B27400000070      MOVE.L    #$0000B274,$70.L     __L0040
00B2CC  23F9000001200000AC84      MOVE.L    $120.L,$0000AC84     L001C
00B2D6  23FC0000B45C00000120      MOVE.L    #$0000B45C,$120.L    __L0048
00B2E0  41F9FFFFFA01              LEA       $FFFFFA01.L,A0       ; MFP 68901 - Multi Function Peripheral Chip
00B2E6  022800FE0012              ANDI.B    #$FE,18(A0)         
00B2EC  022800FE000E              ANDI.B    #$FE,14(A0)         
00B2F2  022800FE000A              ANDI.B    #$FE,10(A0)         
00B2F8  022800FE0006              ANDI.B    #$FE,6(A0)          
00B2FE  027CFBFF                  ANDI.W    #$FBFF,SR           
00B302  4EB90000B1C2              JSR       $0000B1C2            __L003B
00B308  007C0700                  ORI.W     #$700,SR            
00B30C  23FC0000B3AC00000070      MOVE.L    #$0000B3AC,$70.L     __L0043
00B316  4228001A                  CLR.B     26(A0)              
00B31A  002800010006              ORI.B     #1,6(A0)            
00B320  002800010012              ORI.B     #1,18(A0)           
00B326  027CFBFF                  ANDI.W    #$FBFF,SR           
00B32A  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00B330  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B334  4E41                      TRAP      #1     				       ; GEMDOS                  
00B336  DFFC00000006              ADDA.L    #6,A7               
00B33C  4E75                      RTS                           
00B33E  42A7                __L0042:
                                  CLR.L     -(A7)               
00B340  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B344  4E41                      TRAP      #1     				       ; GEMDOS                  
00B346  DFFC00000006              ADDA.L    #6,A7               
00B34C  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00B352  41F9FFFFFA01              LEA       $FFFFFA01.L,A0       ; MFP 68901 - Multi Function Peripheral Chip
00B358  022800FE0012              ANDI.B    #$FE,18(A0)         
00B35E  022800FE000E              ANDI.B    #$FE,14(A0)         
00B364  022800FE000A              ANDI.B    #$FE,10(A0)         
00B36A  022800FE0006              ANDI.B    #$FE,6(A0)          
00B370  4228001A                  CLR.B     26(A0)              
00B374  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00B37A  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B37E  4E41                      TRAP      #1     				       ; GEMDOS                  
00B380  DFFC00000006              ADDA.L    #6,A7               
00B386  2F390000AC46              MOVE.L    $0000AC46,-(A7)      L0007
00B38C  3F3C001C                  MOVE.W    #$1C,-(A7)          
00B390  3F3C0005                  MOVE.W    #5,-(A7)             ; Setexc - Set Exception Vector
00B394  4E4D                      TRAP      #$D                  ; BIOS                 
00B396  508F                      ADDQ.L    #8,A7               
00B398  2F390000AC84              MOVE.L    $0000AC84,-(A7)      L001C
00B39E  3F3C0048                  MOVE.W    #$48,-(A7)          
00B3A2  3F3C0005                  MOVE.W    #5,-(A7)             ; Setexc - Set Exception Vector
00B3A6  4E4D                      TRAP      #$D                  ; BIOS                 
00B3A8  508F                      ADDQ.L    #8,A7               
00B3AA  4E75                      RTS                           
00B3AC  52790000AC8C        __L0043:
                                  ADDQ.W    #1,$0000AC8C         L001F
00B3B2  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00B3B6  4A390000AC82              TST.B     $0000AC82            L001A
00B3BC  67000014                  BEQ       20(PC)               __L0044
00B3C0  42390000AC82              CLR.B     $0000AC82            L001A
00B3C6  20390000AC4E              MOVE.L    $0000AC4E,D0         L0009
00B3CC  23C00000AC56              MOVE.L    D0,$0000AC56         L000B
00B3D2  207C0000AE1A        __L0044:
                                  MOVEA.L   #$0000AE1A,A0        L002B
00B3D8  23C80000AE0E              MOVE.L    A0,$0000AE0E         L0028
00B3DE  43F9FFFFFA01              LEA       $FFFFFA01.L,A1       ; MFP 68901 - Multi Function Peripheral Chip
00B3E4  4229001A                  CLR.B     26(A1)              
00B3E8  4A39000170DF              TST.B     $000170DF            L0691
00B3EE  67000012                  BEQ       18(PC)               __L0045
00B3F2  3010                      MOVE.W    (A0),D0             
00B3F4  9068FFF8                  SUB.W     -8(A0),D0           
00B3F8  13400020                  MOVE.B    D0,32(A1)           
00B3FC  137C0007001A              MOVE.B    #7,26(A1)           
00B402  4A39000171AA        __L0045:
                                  TST.B     $000171AA            L06EA
00B408  66000010                  BNE       16(PC)               L0046
00B40C  4A390001718C              TST.B     $0001718C            L06DD
00B412  67000006                  BEQ       6(PC)                L0046
00B416  46FC2500                  MOVE      #$2500,SR           



00B41A  4EB900016CF4        L0046:JSR       $00016CF4            L063C
00B420  4A39000170DF              TST.B     $000170DF            L0691
00B426  6700001E                  BEQ       30(PC)               __L0047
00B42A  20790000AE16              MOVEA.L   $0000AE16,A0         __L002A
00B430  43F9FFFF8240              LEA       $FFFF8240.L,A1       ; Video palette register 0
00B436  22D8                      MOVE.L    (A0)+,(A1)+         
00B438  22D8                      MOVE.L    (A0)+,(A1)+         
00B43A  22D8                      MOVE.L    (A0)+,(A1)+         
00B43C  22D8                      MOVE.L    (A0)+,(A1)+         
00B43E  22D8                      MOVE.L    (A0)+,(A1)+         
00B440  22D8                      MOVE.L    (A0)+,(A1)+         
00B442  22D8                      MOVE.L    (A0)+,(A1)+         
00B444  22D8                      MOVE.L    (A0)+,(A1)+         
00B446  46FC2000            __L0047:
                                  MOVE      #$2000,SR           
00B44A  4EB900016DDC              JSR       $00016DDC            __L0645
00B450  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00B454  2F390000AC46              MOVE.L    $0000AC46,-(A7)      L0007
00B45A  4E75                      RTS                           
00B45C  46FC2500            __L0048:
                                  MOVE      #$2500,SR           
00B460  08B90000FFFFFA0F          BCLR      #0,$FFFFFA0F.L       ; Interrupt In-service A
00B468  48E7E0C0                  MOVEM.L   A0-A1/D0-D2,-(A7)   
00B46C  20790000AE0E              MOVEA.L   $0000AE0E,A0         L0028
00B472  43F9FFFF8240              LEA       $FFFF8240.L,A1       ; Video palette register 0
00B478  7200                      MOVEQ     #0,D1               
00B47A  343C03E8                  MOVE.W    #$3E8,D2            
00B47E  32390000AC58              MOVE.W    $0000AC58,D1         __L000C
00B484  D2680002                  ADD.W     2(A0),D1            
00B488  5342                __L0049:
                                  SUBQ.W    #1,D2               
00B48A  6B00000C                  BMI       12(PC)               __L004A
00B48E  0109FFC7                  MOVEP.W   -57(A1),D0          
00B492  B041                      CMP.W     D1,D0               
00B494  6B00FFF2                  BMI       -14(PC)              __L0049
00B498  4239FFFFFA1B        __L004A:
                                  CLR.B     $FFFFFA1B.L          ; Timer B Control
00B49E  30280008                  MOVE.W    8(A0),D0            
00B4A2  9050                      SUB.W     (A0),D0             
00B4A4  04400006                  SUBI.W    #6,D0               
00B4A8  13C0FFFFFA21              MOVE.B    D0,$FFFFFA21.L       ; Timer B Data
00B4AE  13FC0007FFFFFA1B          MOVE.B    #7,$FFFFFA1B.L       ; Timer B Control
00B4B6  06B9000000080000AE0E      ADDI.L    #8,$0000AE0E         L0028
00B4C0  20680004                  MOVEA.L   4(A0),A0            
00B4C4  22D8                      MOVE.L    (A0)+,(A1)+         
00B4C6  22D8                      MOVE.L    (A0)+,(A1)+         
00B4C8  22D8                      MOVE.L    (A0)+,(A1)+         
00B4CA  22D8                      MOVE.L    (A0)+,(A1)+         
00B4CC  22D8                      MOVE.L    (A0)+,(A1)+         
00B4CE  22D8                      MOVE.L    (A0)+,(A1)+         
00B4D0  22D8                      MOVE.L    (A0)+,(A1)+         
00B4D2  22D8                      MOVE.L    (A0)+,(A1)+         
00B4D4  4CDF0307                  MOVEM.L   (A7)+,A0-A1/D0-D2   
00B4D8  4E73                      RTE                           
00B4DA  4A39000170DE        L004B:TST.B     $000170DE            L0690
00B4E0  67000010                  BEQ       16(PC)               L004C
00B4E4  0C390004000170DE          CMPI.B    #4,$000170DE         L0690
00B4EC  67000004                  BEQ       4(PC)                L004C
00B4F0  4E75                      RTS                           
00B4F2  48E7FFFE            L004C:MOVEM.L   A0-A6/D0-D7,-(A7)   
00B4F6  42A7                      CLR.L     -(A7)               
00B4F8  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B4FC  4E41                      TRAP      #1     				 ; GEMDOS                  
00B4FE  DFFC00000006              ADDA.L    #6,A7               
00B504  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00B50A  40E7                      MOVE      SR,-(A7)            
00B50C  007C0500                  ORI.W     #$500,SR            
00B510  3E3C0003                  MOVE.W    #3,D7               
00B514  41F90000AE3A              LEA       $0000AE3A,A0         L002C
00B51A  43F90000AE12              LEA       $0000AE12,A1         __L0029
00B520  3018                __L004D:
                                  MOVE.W    (A0)+,D0            
00B522  B07C00C8                  CMP.W     #$C8,D0             
00B526  6E000034                  BGT       52(PC)               __L004F
00B52A  3200                      MOVE.W    D0,D1               
00B52C  67000010                  BEQ       16(PC)               __L004E
00B530  C0F90000AC88              MULU      $0000AC88,D0         L001D
00B536  E048                      LSR.W     #8,D0               
00B538  D0790000AC8A              ADD.W     $0000AC8A,D0         L001E
00B53E  32C0                __L004E:
                                  MOVE.W    D0,(A1)+            
00B540  C2FC00A0                  MULU      #$A0,D1             
00B544  04410018                  SUBI.W    #$18,D1             
00B548  32C1                      MOVE.W    D1,(A1)+            
00B54A  4280                      CLR.L     D0                  
00B54C  3018                      MOVE.W    (A0)+,D0            
00B54E  EB48                      LSL.W     #5,D0               
00B550  06800000ACEE              ADDI.L    #$0000ACEE,D0        L0023
00B556  22C0                      MOVE.L    D0,(A1)+            
00B558  51CFFFC6                  DBF       D7,-58(PC)           __L004D
00B55C  32FC00FF            __L004F:
                                  MOVE.W    #$FF,(A1)+          
00B560  4259                      CLR.W     (A1)+               
00B562  22FC0000ACEE              MOVE.L    #$0000ACEE,(A1)+     L0023
00B568  46DF                      MOVE      (A7)+,SR            
00B56A  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00B570  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B574  4E41                      TRAP      #1     				 ; GEMDOS                  
00B576  DFFC00000006              ADDA.L    #6,A7               
00B57C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00B580  4E75                      RTS                           
00B582  3F3C0004            __L0050:
                                  MOVE.W    #4,-(A7)             ; Getrez - Return current screen resolution code
00B586  4E4E                      TRAP      #$E                  ; XBIOS                 
00B588  548F                      ADDQ.L    #2,A7               
00B58A  13C00000AC76              MOVE.B    D0,$0000AC76         L0015
00B590  0C3900010000AC76          CMPI.B    #1,$0000AC76         L0015
00B598  66000026                  BNE       38(PC)               __L0051
00B59C  3F3C0000                  MOVE.W    #0,-(A7)            
00B5A0  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B5A6  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B5AC  3F3C0005                  MOVE.W    #5,-(A7)             ; Setscreen - Initialize Screen
00B5B0  4E4E                      TRAP      #$E                  ; XBIOS                 
00B5B2  DFFC0000000C              ADDA.L    #$C,A7              
00B5B8  13FC00000000AC76          MOVE.B    #0,$0000AC76         L0015
00B5C0  3F3CFFFF            __L0051:
                                  MOVE.W    #$FFFF,-(A7)        
00B5C4  3F3C0040                  MOVE.W    #$40,-(A7)           ; Blitmode - Blitter-chip configuration
00B5C8  4E4E                      TRAP      #$E                  ; XBIOS                 
00B5CA  588F                      ADDQ.L    #4,A7               
00B5CC  0240FFFE                  ANDI.W    #$FFFE,D0           
00B5D0  3F00                      MOVE.W    D0,-(A7)            
00B5D2  3F3C0040                  MOVE.W    #$40,-(A7)           ; Blitmode - Blitter-chip configuration
00B5D6  4E4E                      TRAP      #$E                  ; XBIOS                 
00B5D8  588F                      ADDQ.L    #4,A7               
00B5DA  427900017232              CLR.W     $00017232            L0710
00B5E0  427900017234              CLR.W     $00017234            L0711
00B5E6  33FC013F00017236          MOVE.W    #$13F,$00017236      L0712
00B5EE  33FC00C700017238          MOVE.W    #$C7,$00017238       L0713
00B5F6  33FC00500001723A          MOVE.W    #$50,$0001723A       L0714
00B5FE  4EB90000E32C              JSR       $0000E32C            __L020B
00B604  4EB90000E562              JSR       $0000E562            __L021E
00B60A  4E75                      RTS                           



00B60C  0C3900010000AC76    __L0052:
                                  CMPI.B    #1,$0000AC76         L0015
00B614  6600001E                  BNE       30(PC)               __L0053
00B618  3F3C0000                  MOVE.W    #0,-(A7)            
00B61C  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B622  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B628  3F3C0005                  MOVE.W    #5,-(A7)             ; Setscreen - Initialize Screen
00B62C  4E4E                      TRAP      #$E                  ; XBIOS                 
00B62E  DFFC0000000C              ADDA.L    #$C,A7              
00B634  303C0001            __L0053:
                                  MOVE.W    #1,D0               
00B638  4EB90000BE62              JSR       $0000BE62            L009E
00B63E  4E75                      RTS                           
00B640  4E75                __L0054:
                                  RTS                           
00B642  4A390000AC82        L0055:TST.B     $0000AC82            L001A
00B648  6600FFF8                  BNE       -8(PC)               L0055
00B64C  4E75                      RTS                           
00B64E  48E7FFFE            L0056:MOVEM.L   A0-A6/D0-D7,-(A7)   
00B652  42A7                      CLR.L     -(A7)               
00B654  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B658  4E41                      TRAP      #1     				       ; GEMDOS                  
00B65A  DFFC00000006              ADDA.L    #6,A7               
00B660  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00B666  20390000AC4E              MOVE.L    $0000AC4E,D0         L0009
00B66C  E088                      LSR.L     #8,D0               
00B66E  13C000FF8203              MOVE.B    D0,$FF8203.L         ; Video screen memory position (Mid byte)
00B674  E088                      LSR.L     #8,D0               
00B676  13C000FF8201              MOVE.B    D0,$FF8201.L         ; Video screen memory position (High byte)
00B67C  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00B682  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00B686  4E41                      TRAP      #1     				       ; GEMDOS                  
00B688  DFFC00000006              ADDA.L    #6,A7               
00B68E  23F90000AC520000AC5E      MOVE.L    $0000AC52,$0000AC5E  L000A L000E
00B698  13FC00010000AC82          MOVE.B    #1,$0000AC82         L001A
00B6A0  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00B6A4  4E75                      RTS                           


; =============================================================================
00B6A6  48E7FFFE            FN_INIT_1_1:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00B6AA  3F3C00FF            __init_1_1_1:
                                  MOVE.W    #$FF,-(A7)          
00B6AE  3F3C0006                  MOVE.W    #6,-(A7)             ; Crawio - Permit unbuffered input/output
00B6B2  4E41                      TRAP      #1     				 ; GEMDOS                  
00B6B4  588F                      ADDQ.L    #4,A7               
00B6B6  4A80                      TST.L     D0                  
00B6B8  6600FFF0                  BNE       -16(PC)              __init_1_1_1
00B6BC  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00B6C0  4EB90000C24C              JSR       $0000C24C            SYS_SUPEXEC_INIT_KEYBOARD
00B6C6  4E75                      RTS                           



00B6C8  4EB90000C2D2        __L0059:
                                  JSR       $0000C2D2            SYS_SUPEXEC_DEINIT_KEYBOARD
00B6CE  3F3C00FF            __L005A:
                                  MOVE.W    #$FF,-(A7)          
00B6D2  3F3C0006                  MOVE.W    #6,-(A7)             ; Crawio - Permit unbuffered input/output
00B6D6  4E41                      TRAP      #1     				 ; GEMDOS                  
00B6D8  588F                      ADDQ.L    #4,A7               
00B6DA  4A80                      TST.L     D0                  
00B6DC  6600FFF0                  BNE       -16(PC)              __L005A
00B6E0  4E75                      RTS                           
00B6E2  4EB90000CB66              JSR       $0000CB66            __L012A
00B6E8  4E75                      RTS                           
00B6EA  4EB90000CB8C              JSR       $0000CB8C            __L012C
00B6F0  4E75                      RTS                           
00B6F2  30390000CAB2        L005B:MOVE.W    $0000CAB2,D0         L011B
00B6F8  32390000CAB4              MOVE.W    $0000CAB4,D1         L011C
00B6FE  4242                      CLR.W     D2                  
00B700  14390000CAB6              MOVE.B    $0000CAB6,D2         L011D
00B706  4E75                      RTS                           
00B708  33C00000CAB2        __L005C:
                                  MOVE.W    D0,$0000CAB2         L011B
00B70E  33C10000CAB4              MOVE.W    D1,$0000CAB4         L011C
00B714  4E75                      RTS                           
00B716  4A00                __L005D:
                                  TST.B     D0                  
00B718  6600000C                  BNE       12(PC)               __L005E
00B71C  4240                      CLR.W     D0                  
00B71E  10390000CAB0              MOVE.B    $0000CAB0,D0         L011A
00B724  4E75                      RTS                           
00B726  4240                __L005E:
                                  CLR.W     D0                  
00B728  10390000CAAF              MOVE.B    $0000CAAF,D0         L0119
00B72E  4E75                      RTS                           
00B730  4A00                __L005F:
                                  TST.B     D0                  
00B732  66000046                  BNE       70(PC)               __L0064
00B736  4240                      CLR.W     D0                  
00B738  083900070000CAB0          BTST      #7,$0000CAB0         L011A
00B740  67000006                  BEQ       6(PC)                __L0060
00B744  08C00000                  BSET      #0,D0               
00B748  083900020000CB37    __L0060:
                                  BTST      #2,$0000CB37         L011F
00B750  67000006                  BEQ       6(PC)                __L0061
00B754  08C00001                  BSET      #1,D0               
00B758  083900030000CB37    __L0061:
                                  BTST      #3,$0000CB37         L011F
00B760  67000006                  BEQ       6(PC)                __L0062
00B764  08C00002                  BSET      #2,D0               
00B768  0C3900E10000CB38    __L0062:
                                  CMPI.B    #$E1,$0000CB38       L0120
00B770  66000006                  BNE       6(PC)                __L0063
00B774  08C00007                  BSET      #7,D0               
00B778  4E75                __L0063:
                                  RTS                           
00B77A  4240                __L0064:
                                  CLR.W     D0                  
00B77C  4E75                      RTS                           
00B77E  4240                L0065:CLR.W     D0                  
00B780  10390000CB38              MOVE.B    $0000CB38,D0         L0120
00B786  4E75                      RTS                           
00B788  4240                L0066:CLR.W     D0                  
00B78A  10390000CB37              MOVE.B    $0000CB37,D0         L011F
00B790  4E75                      RTS                           
00B792  4EB90000B77E        L0067:JSR       $0000B77E            L0065
00B798  4A00                      TST.B     D0                  
00B79A  66000006                  BNE       6(PC)                __L0068
00B79E  6000FFF2                  BRA       -14(PC)              L0067
00B7A2  48E78000            __L0068:
                                  MOVEM.L   D0,-(A7)            
00B7A6  4EB90000B77E        __L0069:
                                  JSR       $0000B77E            L0065
00B7AC  4A00                      TST.B     D0                  
00B7AE  6600FFF6                  BNE       -10(PC)              __L0069
00B7B2  4CDF0001                  MOVEM.L   (A7)+,D0            
00B7B6  4E75                      RTS                           
00B7B8  3F3C0011            SYS_RANDOM:
                                  MOVE.W    #$11,-(A7)           ; Random - Random number generator
00B7BC  4E4E                      TRAP      #$E                  ; XBIOS                 
00B7BE  548F                      ADDQ.L    #2,A7               
00B7C0  4E75                      RTS                           
00B7C2  48E7FF7E            __L006B:
                                  MOVEM.L   A1-A6/D0-D7,-(A7)   
00B7C6  4239000170DE              CLR.B     $000170DE            L0690
00B7CC  4242                      CLR.W     D2                  
00B7CE  20790000AC32              MOVEA.L   $0000AC32,A0         L0005
00B7D4  5488                      ADDQ.L    #2,A0               
00B7D6  1028FFFF                  MOVE.B    -1(A0),D0           
00B7DA  41F90000B908              LEA       $0000B908,A0         __L0071
00B7E0  0C3900020000AC76          CMPI.B    #2,$0000AC76         L0015
00B7E8  66000118                  BNE       280(PC)              L0070
00B7EC  13FC0003000170DE          MOVE.B    #3,$000170DE         L0690
00B7F4  600000F4                  BRA       244(PC)              L006E
00B7F8  B03C0041                  CMP.B     #$41,D0             ;(#"A") 
00B7FC  67000104                  BEQ       260(PC)              L0070
00B800  13FC0001000170DE          MOVE.B    #1,$000170DE         L0690
00B808  B03C0049                  CMP.B     #$49,D0             ;(#"I") 
00B80C  67000068                  BEQ       104(PC)              L006C
00B810  13FC0005000170DE          MOVE.B    #5,$000170DE         L0690
00B818  B03C004A                  CMP.B     #$4A,D0             ;(#"J") 
00B81C  67000058                  BEQ       88(PC)               L006C
00B820  13FC0002000170DE          MOVE.B    #2,$000170DE         L0690
00B828  B03C0045                  CMP.B     #$45,D0             ;(#"E") 
00B82C  67000048                  BEQ       72(PC)               L006C
00B830  13FC0006000170DE          MOVE.B    #6,$000170DE         L0690
00B838  B03C0056                  CMP.B     #$56,D0             ;(#"V") 
00B83C  670000C4                  BEQ       196(PC)              L0070
00B840  13FC0003000170DE          MOVE.B    #3,$000170DE         L0690
00B848  B03C004D                  CMP.B     #$4D,D0             ;(#"M") 
00B84C  67000028                  BEQ       40(PC)               L006C
00B850  13FC0004000170DE          MOVE.B    #4,$000170DE         L0690
00B858  B03C0043                  CMP.B     #$43,D0             ;(#"C") 
00B85C  670000A4                  BEQ       164(PC)              L0070
00B860  13FC0007000170DE          MOVE.B    #7,$000170DE         L0690
00B868  B03C004B                  CMP.B     #$4B,D0             ;(#"K") 
00B86C  67000008                  BEQ       8(PC)                L006C
00B870  4239000170DE              CLR.B     $000170DE            L0690
00B876  0C390001000170DE    L006C:CMPI.B    #1,$000170DE         L0690
00B87E  6700006A                  BEQ       106(PC)              L006E
00B882  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
00B88A  67000026                  BEQ       38(PC)               __L006D
00B88E  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
00B896  67000060                  BEQ       96(PC)               L006F
00B89A  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
00B8A2  67000054                  BEQ       84(PC)               L006F
00B8A6  0C3900020000AC76          CMPI.B    #2,$0000AC76         L0015
00B8AE  6700003A                  BEQ       58(PC)               L006E
00B8B2  0C3900000000AC76    __L006D:
                                  CMPI.B    #0,$0000AC76         L0015
00B8BA  6600002E                  BNE       46(PC)               L006E
00B8BE  48E70080                  MOVEM.L   A0,-(A7)            
00B8C2  3F3C0001                  MOVE.W    #1,-(A7)            
00B8C6  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B8CC  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00B8D2  3F3C0005                  MOVE.W    #5,-(A7)             ; Setscreen - Initialize Screen
00B8D6  4E4E                      TRAP      #$E                  ; XBIOS                 
00B8D8  DFFC0000000C              ADDA.L    #$C,A7              
00B8DE  4CDF0100                  MOVEM.L   (A7)+,A0            
00B8E2  13FC00010000AC76          MOVE.B    #1,$0000AC76         L0015
00B8EA  3E3CFFFE            L006E:MOVE.W    #$FFFE,D7           
00B8EE  4EB90000BF6C              JSR       $0000BF6C            L00A8
00B8F4  6000000C                  BRA       12(PC)               L0070
00B8F8  3E3CFFF0            L006F:MOVE.W    #$FFF0,D7           
00B8FC  4EB90000BF6C              JSR       $0000BF6C            L00A8
00B902  4CDF7EFF            L0070:MOVEM.L   (A7)+,A1-A6/D0-D7   
00B906  4E75                      RTS                           
00B908  4D41494E2E414F00    __L0071:
                                  DC.B      $4D,$41,$49,$4E,$2E,$41,$4F,$00 
00B910  0000                      DC.B      $00,$00 
00B912  1039000170DE        __L0072:
                                  MOVE.B    $000170DE,D0         L0690
00B918  67000042                  BEQ       66(PC)               L0073
00B91C  B03C0001                  CMP.B     #1,D0               
00B920  6700007E                  BEQ       126(PC)              __L0079
00B924  B03C0002                  CMP.B     #2,D0               
00B928  67000086                  BEQ       134(PC)              __L007B
00B92C  B03C0003                  CMP.B     #3,D0               
00B930  6700002A                  BEQ       42(PC)               L0073
00B934  B03C0004                  CMP.B     #4,D0               
00B938  67000088                  BEQ       136(PC)              __L007E
00B93C  B03C0005                  CMP.B     #5,D0               
00B940  67000068                  BEQ       104(PC)              __L007A
00B944  B03C0006                  CMP.B     #6,D0               
00B948  6700006C                  BEQ       108(PC)              __L007C
00B94C  B03C0007                  CMP.B     #7,D0               
00B950  6700006A                  BEQ       106(PC)              __L007D
00B954  B03C0008                  CMP.B     #8,D0               
00B958  67000040                  BEQ       64(PC)               __L0078
00B95C  20790000AC4A        L0073:MOVEA.L   $0000AC4A,A0         L0008
00B962  B1FC00078000              CMPA.L    #$78000,A0          
00B968  6600000A                  BNE       10(PC)               __L0074
00B96C  303C03E8                  MOVE.W    #$3E8,D0            
00B970  60000018                  BRA       24(PC)               L0076
00B974  B1FC000F8000        __L0074:
                                  CMPA.L    #$F8000,A0          
00B97A  6600000A                  BNE       10(PC)               __L0075
00B97E  303C03F2                  MOVE.W    #$3F2,D0            
00B982  60000006                  BRA       6(PC)                L0076
00B986  303C03FC            __L0075:
                                  MOVE.W    #$3FC,D0            
00B98A  4A390000AC76        L0076:TST.B     $0000AC76            L0015
00B990  67000006                  BEQ       6(PC)                __L0077
00B994  06400001                  ADDI.W    #1,D0               
00B998  4E75                __L0077:
                                  RTS                           
00B99A  303C0064            __L0078:
                                  MOVE.W    #$64,D0             ;(#"d") 
00B99E  4E75                      RTS                           
00B9A0  303C07EE            __L0079:
                                  MOVE.W    #$7EE,D0            
00B9A4  4E75                      RTS                           
00B9A6  303C07EF                  MOVE.W    #$7EF,D0            
00B9AA  303C07F0            __L007A:
                                  MOVE.W    #$7F0,D0            
00B9AE  4E75                      RTS                           
00B9B0  303C07F1            __L007B:
                                  MOVE.W    #$7F1,D0            
00B9B4  4E75                      RTS                           
00B9B6  303C07F2            __L007C:
                                  MOVE.W    #$7F2,D0            
00B9BA  4E75                      RTS                           
00B9BC  303C0FA1            __L007D:
                                  MOVE.W    #$FA1,D0            
00B9C0  4E75                      RTS                           
00B9C2  303C0BB8            __L007E:
                                  MOVE.W    #$BB8,D0            
00B9C6  4E75                      RTS                           
00B9C8  303C0064            __L007F:
                                  MOVE.W    #$64,D0             ;(#"d") 
00B9CC  4E75                      RTS                           
00B9CE  4240                __L0080:
                                  CLR.W     D0                  
00B9D0  4E75                      RTS                           

; =============================================================================
; SYS_SET_TIME
; D0 / D1 / D2
00B9D2  48E70100            SYS_SET_TIME:
                                  MOVEM.L   D7,-(A7)            
00B9D6  3E3C000B                  MOVE.W    #$B,D7              
00B9DA  EF60                      ASL.W     D7,D0               
00B9DC  EB41                      ASL.W     #5,D1               
00B9DE  D041                      ADD.W     D1,D0               
00B9E0  E24A                      LSR.W     #1,D2               
00B9E2  D042                      ADD.W     D2,D0               
00B9E4  3F00                      MOVE.W    D0,-(A7)            
00B9E6  3F3C002D                  MOVE.W    #$2D,-(A7)           ; Tsettime - Set the time 
00B9EA  4E41                      TRAP      #1     				 ; GEMDOS                  
00B9EC  588F                      ADDQ.L    #4,A7               
00B9EE  4CDF0080                  MOVEM.L   (A7)+,D7            
00B9F2  4E75                      RTS                           



; =============================================================================
; SYS_GET_TIME
; return data in D0 / D1 / D2
00B9F4  4240                SYS_GET_TIME:
                                  CLR.W     D0                  
00B9F6  10390000CB3C              MOVE.B    $0000CB3C,D0         L0124
00B9FC  4241                      CLR.W     D1                  
00B9FE  12390000CB3D              MOVE.B    $0000CB3D,D1         L0125
00BA04  4242                      CLR.W     D2                  
00BA06  14390000CB3E              MOVE.B    $0000CB3E,D2         L0126
00BA0C  4E75                      RTS                           



00BA0E  B63C0002            L0083:CMP.B     #2,D3               
00BA12  63000004                  BLS       4(PC)                __L0084
00BA16  4E75                      RTS                           
00BA18  4A39000171DA        __L0084:
                                  TST.B     $000171DA            L06F5
00BA1E  67000004                  BEQ       4(PC)                __L0085
00BA22  4E75                      RTS                           
00BA24  122B0006            __L0085:
                                  MOVE.B    6(A3),D1            
00BA28  E609                      LSR.B     #3,D1               
00BA2A  3003                      MOVE.W    D3,D0               
00BA2C  06400088                  ADDI.W    #$88,D0             
00BA30  3F00                      MOVE.W    D0,-(A7)            
00BA32  0241000F                  ANDI.W    #$F,D1              
00BA36  3F01                      MOVE.W    D1,-(A7)            
00BA38  610000F4                  BSR       244(PC)              L008E
00BA3C  588F                      ADDQ.L    #4,A7               
00BA3E  3803                      MOVE.W    D3,D4               
00BA40  D844                      ADD.W     D4,D4               
00BA42  06440080                  ADDI.W    #$80,D4             
00BA46  3F04                      MOVE.W    D4,-(A7)            
00BA48  322B000A                  MOVE.W    10(A3),D1           
00BA4C  E649                      LSR.W     #3,D1               
00BA4E  4240                      CLR.W     D0                  
00BA50  1001                      MOVE.B    D1,D0               
00BA52  3F00                      MOVE.W    D0,-(A7)            
00BA54  610000D8                  BSR       216(PC)              L008E
00BA58  588F                      ADDQ.L    #4,A7               
00BA5A  5244                      ADDQ.W    #1,D4               
00BA5C  3F04                      MOVE.W    D4,-(A7)            
00BA5E  E049                      LSR.W     #8,D1               
00BA60  1001                      MOVE.B    D1,D0               
00BA62  0240000F                  ANDI.W    #$F,D0              
00BA66  3F00                      MOVE.W    D0,-(A7)            
00BA68  610000C4                  BSR       196(PC)              L008E
00BA6C  588F                      ADDQ.L    #4,A7               
00BA6E  3F3C0007                  MOVE.W    #7,-(A7)            
00BA72  4267                      CLR.W     -(A7)               
00BA74  610000B8                  BSR       184(PC)              L008E
00BA78  588F                      ADDQ.L    #4,A7               
00BA7A  B67C0002                  CMP.W     #2,D3               
00BA7E  6E000048                  BGT       72(PC)               L0087
00BA82  3803                      MOVE.W    D3,D4               
00BA84  06440003                  ADDI.W    #3,D4               
00BA88  07C0                      BSET      D3,D0               
00BA8A  09C0                      BSET      D4,D0               
00BA8C  122B0002                  MOVE.B    2(A3),D1            
00BA90  08010000                  BTST      #0,D1               
00BA94  67000004                  BEQ       4(PC)                __L0086
00BA98  0780                      BCLR      D3,D0               
00BA9A  08010001            __L0086:
                                  BTST      #1,D1               
00BA9E  67000028                  BEQ       40(PC)               L0087
00BAA2  0980                      BCLR      D4,D0               
00BAA4  61000030                  BSR       48(PC)               __L0088
00BAA8  6700001E                  BEQ       30(PC)               L0087
00BAAC  48E78000                  MOVEM.L   D0,-(A7)            
00BAB0  302B000A                  MOVE.W    10(A3),D0           
00BAB4  EE48                      LSR.W     #7,D0               
00BAB6  E648                      LSR.W     #3,D0               
00BAB8  3F3C0086                  MOVE.W    #$86,-(A7)          
00BABC  3F00                      MOVE.W    D0,-(A7)            
00BABE  6100006E                  BSR       110(PC)              L008E
00BAC2  588F                      ADDQ.L    #4,A7               
00BAC4  4CDF0001                  MOVEM.L   (A7)+,D0            
00BAC8  3F3C0087            L0087:MOVE.W    #$87,-(A7)          
00BACC  3F00                      MOVE.W    D0,-(A7)            
00BACE  6100005E                  BSR       94(PC)               L008E
00BAD2  588F                      ADDQ.L    #4,A7               
00BAD4  4E75                      RTS                           
00BAD6  48E7C080            __L0088:
                                  MOVEM.L   A0/D0-D1,-(A7)      
00BADA  123C0001                  MOVE.B    #1,D1               
00BADE  102B0001                  MOVE.B    1(A3),D0            
00BAE2  41F900017D04              LEA       $00017D04,A0         L0724
00BAE8  B0280001                  CMP.B     1(A0),D0            
00BAEC  6C000006                  BGE       6(PC)                __L0089
00BAF0  6100002E                  BSR       46(PC)               L008C
00BAF4  41F900017D14        __L0089:
                                  LEA       $00017D14,A0         L0726
00BAFA  B0280001                  CMP.B     1(A0),D0            
00BAFE  6C000006                  BGE       6(PC)                __L008A
00BB02  6100001C                  BSR       28(PC)               L008C
00BB06  41F900017D24        __L008A:
                                  LEA       $00017D24,A0         L0728
00BB0C  B0280001                  CMP.B     1(A0),D0            
00BB10  6C000006                  BGE       6(PC)                __L008B
00BB14  6100000A                  BSR       10(PC)               L008C
00BB18  4A01                __L008B:
                                  TST.B     D1                  
00BB1A  4CDF0103                  MOVEM.L   (A7)+,A0/D0-D1      
00BB1E  4E75                      RTS                           
00BB20  082800010002        L008C:BTST      #1,2(A0)            
00BB26  67000004                  BEQ       4(PC)                __L008D
00BB2A  4201                      CLR.B     D1                  
00BB2C  4E75                __L008D:
                                  RTS                           
00BB2E  48E76080            L008E:MOVEM.L   A0/D1-D2,-(A7)      
00BB32  302F0010                  MOVE.W    16(A7),D0           
00BB36  322F0012                  MOVE.W    18(A7),D1           
00BB3A  41F9FFFF8800              LEA       $FFFF8800.L,A0       ; YM2149 Sound Chip - Read data/Register select
00BB40  1401                      MOVE.B    D1,D2               
00BB42  0201000F                  ANDI.B    #$F,D1              
00BB46  1081                      MOVE.B    D1,(A0)             
00BB48  B23C000D                  CMP.B     #$D,D1              
00BB4C  6200000C                  BHI       12(PC)               L008F
00BB50  E302                      ASL.B     #1,D2               
00BB52  64000006                  BCC       6(PC)                L008F
00BB56  11400002                  MOVE.B    D0,2(A0)            
00BB5A  7000                L008F:MOVEQ     #0,D0               
00BB5C  1010                      MOVE.B    (A0),D0             
00BB5E  4CDF0106                  MOVEM.L   (A7)+,A0/D1-D2      
00BB62  4E75                      RTS                           
00BB64  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00BB68  302A0012                  MOVE.W    18(A2),D0           
00BB6C  5240                      ADDQ.W    #1,D0               
00BB6E  E848                      LSR.W     #4,D0               
00BB70  E548                      LSL.W     #2,D0               
00BB72  D040                      ADD.W     D0,D0               
00BB74  343900017220              MOVE.W    $00017220,D2         L0709
00BB7A  94790001721C              SUB.W     $0001721C,D2         L0707
00BB80  5242                      ADDQ.W    #1,D2               
00BB82  C0C2                      MULU      D2,D0               
00BB84  06400010                  ADDI.W    #$10,D0             
00BB88  4EB90000BBF8              JSR       $0000BBF8            L0092
00BB8E  4A80                      TST.L     D0                  
00BB90  6600000C                  BNE       12(PC)               __L0090
00BB94  42390001720A              CLR.B     $0001720A            L0700
00BB9A  60000056                  BRA       86(PC)               __L0091
00BB9E  322A0012            __L0090:
                                  MOVE.W    18(A2),D1           
00BBA2  5241                      ADDQ.W    #1,D1               
00BBA4  E849                      LSR.W     #4,D1               
00BBA6  E549                      LSL.W     #2,D1               
00BBA8  33C100017222              MOVE.W    D1,$00017222         L070A
00BBAE  23C000017210              MOVE.L    D0,$00017210         L0703
00BBB4  06800000000A              ADDI.L    #$A,D0              
00BBBA  23C000017214              MOVE.L    D0,$00017214         L0704
00BBC0  207900017210              MOVEA.L   $00017210,A0         L0703
00BBC6  20BC00000004              MOVE.L    #4,(A0)             
00BBCC  D1D0                      ADDA.L    (A0),A0             
00BBCE  30BC0200                  MOVE.W    #$200,(A0)          
00BBD2  30390001721E              MOVE.W    $0001721E,D0         L0708
00BBD8  90790001721A              SUB.W     $0001721A,D0         L0706
00BBDE  31400002                  MOVE.W    D0,2(A0)            
00BBE2  303900017220              MOVE.W    $00017220,D0         L0709
00BBE8  90790001721C              SUB.W     $0001721C,D0         L0707
00BBEE  31400004                  MOVE.W    D0,4(A0)            
00BBF2  4CDF7FFF            __L0091:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00BBF6  4E75                      RTS                           
00BBF8  48E74080            L0092:MOVEM.L   A0/D1,-(A7)         
00BBFC  5380                      SUBQ.L    #1,D0               
00BBFE  008000000003              ORI.L     #3,D0               
00BC04  5280                      ADDQ.L    #1,D0               
00BC06  223900017066              MOVE.L    $00017066,D1         L065D
00BC0C  5080                      ADDQ.L    #8,D0               
00BC0E  9280                      SUB.L     D0,D1               
00BC10  B2B90001709C              CMP.L     $0001709C,D1         L0676
00BC16  6F000014                  BLE       20(PC)               __L0093
00BC1A  2041                      MOVEA.L   D1,A0               
00BC1C  23C800017066              MOVE.L    A0,$00017066         L065D
00BC22  2080                      MOVE.L    D0,(A0)             
00BC24  5088                      ADDQ.L    #8,A0               
00BC26  2008                      MOVE.L    A0,D0               
00BC28  60000004                  BRA       4(PC)                __L0094
00BC2C  4280                __L0093:
                                  CLR.L     D0                  
00BC2E  4CDF0102            __L0094:
                                  MOVEM.L   (A7)+,A0/D1         
00BC32  4E75                      RTS                           
00BC34  48E78080            __L0095:
                                  MOVEM.L   A0/D0,-(A7)         
00BC38  5180                      SUBQ.L    #8,D0               
00BC3A  207900017066              MOVEA.L   $00017066,A0         L065D
00BC40  B1C0                      CMPA.L    D0,A0               
00BC42  6600000A                  BNE       10(PC)               __L0096
00BC46  D1D0                      ADDA.L    (A0),A0             
00BC48  23C800017066              MOVE.L    A0,$00017066         L065D
00BC4E  4CDF0101            __L0096:
                                  MOVEM.L   (A7)+,A0/D0         
00BC52  4E75                      RTS                           


; =============================================================================
; SYS_VT52_PUT_CHAR
; D0 character
00BC54  48E78080            SYS_VT52_PUT_CHAR:
                                  MOVEM.L   A0/D0,-(A7)         
00BC58  024000FF                  ANDI.W    #$FF,D0             
00BC5C  3F00                      MOVE.W    D0,-(A7)            
00BC5E  4217                      CLR.B     (A7)                
00BC60  3F3C0002                  MOVE.W    #2,-(A7)       ; Cconout - Write one character to the standard output device
00BC64  4E41                      TRAP      #1     				 ; GEMDOS                  
00BC66  588F                      ADDQ.L    #4,A7               
00BC68  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
00BC6C  4E75                      RTS


; =============================================================================                           
00BC6E  3F00                SYS_VT52_SET_CURSOR:
                                  MOVE.W    D0,-(A7)            
00BC70  3F01                      MOVE.W    D1,-(A7)            
00BC72  103C001B                  MOVE.B    #$1B,D0             
00BC76  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BC7C  103C0059                  MOVE.B    #$59,D0             ;(#"Y") 
00BC80  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BC86  301F                      MOVE.W    (A7)+,D0            
00BC88  06000020                  ADDI.B    #$20,D0             
00BC8C  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BC92  301F                      MOVE.W    (A7)+,D0            
00BC94  06000020                  ADDI.B    #$20,D0             
00BC98  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BC9E  4E75                      RTS


00BCA0  48E700602279        __L0099:
                                  DC.B      $48,$E7,$00,$60,$22,$79
00BCA6  0000AC6A                  DC.L      $0000AC6A            L0011
00BCAA  3280334100020C39          DC.B      $32,$80,$33,$41,$00,$02,$0C,$39 
00BCB2  0003                      DC.B      $00,$03 
00BCB4  000170DE                  DC.L      $000170DE            L0690
00BCB8  6600001A3011D151          DC.B      $66,$00,$00,$1A,$30,$11,$D1,$51 
00BCC0  0C390002                  DC.B      $0C,$39,$00,$02 
00BCC4  0000AC76                  DC.L      $0000AC76            L0015
00BCC8  6600000A30290002          DC.B      $66,$00,$00,$0A,$30,$29,$00,$02 
00BCD0  D1690002A0010C39          DC.B      $D1,$69,$00,$02,$A0,$01,$0C,$39 
00BCD8  0003                      DC.B      $00,$03 
00BCDA  000170DE                  DC.L      $000170DE            L0690
00BCDE  660000342279              DC.B      $66,$00,$00,$34,$22,$79 
00BCE4  0000AC6A                  DC.L      $0000AC6A            L0011
00BCE8  06510001A0010C39          DC.B      $06,$51,$00,$01,$A0,$01,$0C,$39 
00BCF0  0002                      DC.B      $00,$02 
00BCF2  0000AC76                  DC.L      $0000AC76            L0015
00BCF6  6600001C2279              DC.B      $66,$00,$00,$1C,$22,$79 
00BCFC  0000AC6A                  DC.L      $0000AC6A            L0011
00BD00  066900010002A001          DC.B      $06,$69,$00,$01,$00,$02,$A0,$01 
00BD08  2279                      DC.B      $22,$79 
00BD0A  0000AC6A                  DC.L      $0000AC6A            L0011
00BD0E  04510001A001              DC.B      $04,$51,$00,$01,$A0,$01 
00BD14  4CDF0600                  MOVEM.L   (A7)+,A1-A2         
00BD18  4E75                      RTS                           
00BD1A  48E7F0A02079        L009A:DC.B      $48,$E7,$F0,$A0,$20,$79 
00BD20  0000AC62                  DC.L      $0000AC62            L000F
00BD24  426800363179              DC.B      $42,$68,$00,$36,$31,$79 
00BD2A  0000AC72                  DC.L      $0000AC72            L0013
00BD2E  0024314000263141          DC.B      $00,$24,$31,$40,$00,$26,$31,$41 
00BD36  00283142002A3143          DC.B      $00,$28,$31,$42,$00,$2A,$31,$43 
00BD3E  002C0C390003              DC.B      $00,$2C,$0C,$39,$00,$03 
00BD44  000170DE                  DC.L      $000170DE            L0690
00BD48  6600002E30280026          DC.B      $66,$00,$00,$2E,$30,$28,$00,$26 
00BD50  D16800263028002A          DC.B      $D1,$68,$00,$26,$30,$28,$00,$2A 
00BD58  D168002A0C390002          DC.B      $D1,$68,$00,$2A,$0C,$39,$00,$02 
00BD60  0000AC76                  DC.L      $0000AC76            L0015
00BD64  6600001230280028          DC.B      $66,$00,$00,$12,$30,$28,$00,$28 
00BD6C  D16800283028002C          DC.B      $D1,$68,$00,$28,$30,$28,$00,$2C 
00BD74  D168002CA0030C39          DC.B      $D1,$68,$00,$2C,$A0,$03,$0C,$39 
00BD7C  0003                      DC.B      $00,$03 
00BD7E  000170DE                  DC.L      $000170DE            L0690
00BD82  6600003806680001          DC.B      $66,$00,$00,$38,$06,$68,$00,$01 
00BD8A  002606680001002A          DC.B      $00,$26,$06,$68,$00,$01,$00,$2A 
00BD92  A0030C390002              DC.B      $A0,$03,$0C,$39,$00,$02 
00BD98  0000AC76                  DC.L      $0000AC76            L0015
00BD9C  6600001E06680001          DC.B      $66,$00,$00,$1E,$06,$68,$00,$01 
00BDA4  002806680001002C          DC.B      $00,$28,$06,$68,$00,$01,$00,$2C 
00BDAC  A003046800010026          DC.B      $A0,$03,$04,$68,$00,$01,$00,$26 
00BDB4  04680001002AA003          DC.B      $04,$68,$00,$01,$00,$2A,$A0,$03 
00BDBC  4CDF050F                  MOVEM.L   (A7)+,A2/A0/D0-D3   
00BDC0  4E75                      RTS                           
00BDC2  48E700A02079        __L009B:
                                  DC.B      $48,$E7,$00,$A0,$20,$79 
00BDC8  0000AC62                  DC.L      $0000AC62            L000F
00BDCC  426800363179              DC.B      $42,$68,$00,$36,$31,$79 
00BDD2  0000AC72                  DC.L      $0000AC72            L0013
00BDD6  0024314000263141          DC.B      $00,$24,$31,$40,$00,$26,$31,$41 
00BDDE  00283142002A3143          DC.B      $00,$28,$31,$42,$00,$2A,$31,$43 
00BDE6  002CB46800266C00          DC.B      $00,$2C,$B4,$68,$00,$26,$6C,$00 
00BDEE  000C31680026002A          DC.B      $00,$0C,$31,$68,$00,$26,$00,$2A 
00BDF6  31420026B6680028          DC.B      $31,$42,$00,$26,$B6,$68,$00,$28 
00BDFE  6C00000C31680028          DC.B      $6C,$00,$00,$0C,$31,$68,$00,$28 
00BE06  002C314300282179          DC.B      $00,$2C,$31,$43,$00,$28,$21,$79 
00BE0E  0000AC6E                  DC.L      $0000AC6E            L0012
00BE12  002E317C00010032          DC.B      $00,$2E,$31,$7C,$00,$01,$00,$32 
00BE1A  0C390003                  DC.B      $0C,$39,$00,$03 
00BE1E  000170DE                  DC.L      $000170DE            L0690
00BE22  6600002E30280026          DC.B      $66,$00,$00,$2E,$30,$28,$00,$26 
00BE2A  D16800263028002A          DC.B      $D1,$68,$00,$26,$30,$28,$00,$2A 
00BE32  D168002A0C390002          DC.B      $D1,$68,$00,$2A,$0C,$39,$00,$02 
00BE3A  0000AC76                  DC.L      $0000AC76            L0015
00BE3E  6600001230280028          DC.B      $66,$00,$00,$12,$30,$28,$00,$28 
00BE46  D16800283028002C          DC.B      $D1,$68,$00,$28,$30,$28,$00,$2C 
00BE4E  D168002CA005              DC.B      $D1,$68,$00,$2C,$A0,$05 
00BE54  4CDF0500                  MOVEM.L   (A7)+,A2/A0         
00BE58  4E75                      RTS                           
00BE5A  FFFFFFFF            L009C:DC.B      $FF,$FF,$FF,$FF 
00BE5E  AAAAAAAA            __L009D:
                                  DC.B      $AA,$AA,$AA,$AA 
00BE62  48E70080            L009E:MOVEM.L   A0,-(A7)            
00BE66  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
00BE6E  66000006                  BNE       6(PC)                __L009F
00BE72  0040000C                  ORI.W     #$C,D0              
00BE76  0C390003000170DE    __L009F:
                                  CMPI.B    #3,$000170DE         L0690
00BE7E  66000028                  BNE       40(PC)               L00A1
00BE82  23FC0000BE5A0000AC6E      MOVE.L    #$0000BE5A,$0000AC6E L009C L0012
00BE8C  4A40                      TST.W     D0                  
00BE8E  67000018                  BEQ       24(PC)               L00A1
00BE92  B03C0003                  CMP.B     #3,D0               
00BE96  6700000C                  BEQ       12(PC)               __L00A0
00BE9A  23FC0000BE5E0000AC6E      MOVE.L    #$0000BE5E,$0000AC6E __L009D L0012
00BEA4  303C0001            __L00A0:
                                  MOVE.W    #1,D0               
00BEA8  20790000AC62        L00A1:MOVEA.L   $0000AC62,A0         L000F
00BEAE  08000000                  BTST      #0,D0               
00BEB2  56E80019                  SNE       25(A0)              
00BEB6  08000001                  BTST      #1,D0               
00BEBA  56E8001B                  SNE       27(A0)              
00BEBE  08000002                  BTST      #2,D0               
00BEC2  56E8001D                  SNE       29(A0)              
00BEC6  08000003                  BTST      #3,D0               
00BECA  56E8001F                  SNE       31(A0)              
00BECE  3F00                      MOVE.W    D0,-(A7)            
00BED0  103C001B                  MOVE.B    #$1B,D0             
00BED4  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BEDA  103C0062                  MOVE.B    #$62,D0             ;(#"b") 
00BEDE  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BEE4  301F                      MOVE.W    (A7)+,D0            
00BEE6  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BEEC  4CDF0100                  MOVEM.L   (A7)+,A0            
00BEF0  4E75                      RTS                           
00BEF2  3F00                __L00A2:
                                  MOVE.W    D0,-(A7)            
00BEF4  103C001B                  MOVE.B    #$1B,D0             
00BEF8  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BEFE  103C0063                  MOVE.B    #$63,D0             ;(#"c") 
00BF02  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BF08  301F                      MOVE.W    (A7)+,D0            
00BF0A  4EB90000BC54              JSR       $0000BC54            SYS_VT52_PUT_CHAR
00BF10  4E75                      RTS                           
00BF12  48E70080            __L00A3:
                                  MOVEM.L   A0,-(A7)            
00BF16  20790000AC62              MOVEA.L   $0000AC62,A0         L000F
00BF1C  33C70000AC72              MOVE.W    D7,$0000AC72         L0013
00BF22  31470024                  MOVE.W    D7,36(A0)           
00BF26  4CDF0100                  MOVEM.L   (A7)+,A0            
00BF2A  4E75                      RTS                           
00BF2C  4A39000170DF        L00A4:TST.B     $000170DF            L0691
00BF32  67000004                  BEQ       4(PC)                __L00A5
00BF36  4E75                      RTS                           
00BF38  48E7C040            __L00A5:
                                  MOVEM.L   A1/D0-D1,-(A7)      
00BF3C  227CFFFF8240              MOVEA.L   #$FFFF8240,A1        ; Video palette register 0
00BF42  303C000F                  MOVE.W    #$F,D0              
00BF46  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
00BF4E  6700000E                  BEQ       14(PC)               L00A7
00BF52  32D8                __L00A6:
                                  MOVE.W    (A0)+,(A1)+         
00BF54  51C8FFFC                  DBF       D0,-4(PC)            __L00A6
00BF58  4CDF0203                  MOVEM.L   (A7)+,A1/D0-D1      
00BF5C  4E75                      RTS                           
00BF5E  3218                L00A7:MOVE.W    (A0)+,D1            
00BF60  32C1                      MOVE.W    D1,(A1)+            
00BF62  51C8FFFA                  DBF       D0,-6(PC)            L00A7
00BF66  4CDF0203                  MOVEM.L   (A7)+,A1/D0-D1      
00BF6A  4E75                      RTS                           
00BF6C  48E7FFFE            L00A8:MOVEM.L   A0-A6/D0-D7,-(A7)   
00BF70  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
00BF78  6700002E                  BEQ       46(PC)               L00A9
00BF7C  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
00BF84  67000022                  BEQ       34(PC)               L00A9
00BF88  41F90000AC8E              LEA       $0000AC8E,A0         __L0020
00BF8E  BE7CFFFF                  CMP.W     #$FFFF,D7           
00BF92  67000022                  BEQ       34(PC)               L00AA
00BF96  41F90000ACAE              LEA       $0000ACAE,A0         __L0021
00BF9C  BE3C00FE                  CMP.B     #$FE,D7             
00BFA0  67000014                  BEQ       20(PC)               L00AA
00BFA4  6000001A                  BRA       26(PC)               L00AB
00BFA8  BE3C00F0            L00A9:CMP.B     #$F0,D7             
00BFAC  66000012                  BNE       18(PC)               L00AB
00BFB0  41F90000ACCE              LEA       $0000ACCE,A0         __L0022
00BFB6  303C000F            L00AA:MOVE.W    #$F,D0              
00BFBA  4EB900013F28              JSR       $00013F28            __L04C9
00BFC0  4CDF7FFF            L00AB:MOVEM.L   (A7)+,A0-A6/D0-D7   
00BFC4  4E75                      RTS                           
00BFC6  5C6269646F6E5C00          DC.B      $5C,$62,$69,$64,$6F,$6E,$5C,$00  ; '\bidon\'


; =============================================================================
; SYSTEM DISK ROUTINES
; =============================================================================
00BFCE  48E7FFFE            L00AC:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00BFD2  3F3C0019                  MOVE.W    #$19,-(A7)           ; Dgetdrv - Obtain the current drive
00BFD6  4E41                      TRAP      #1     				       ; GEMDOS                  
00BFD8  548F                      ADDQ.L    #2,A7               
00BFDA  48E78000            __L00AD:
                                  MOVEM.L   D0,-(A7)            
00BFDE  3F00                      MOVE.W    D0,-(A7)             ; d0 -> drive number
00BFE0  3F3C0007                  MOVE.W    #7,-(A7)             ; Getbpb - Get BIOS parameter block
00BFE4  4E4D                      TRAP      #$D                  ; BIOS                 
00BFE6  588F                      ADDQ.L    #4,A7               
00BFE8  4A80                      TST.L     D0                  
00BFEA  4CDF0001                  MOVEM.L   (A7)+,D0            
00BFEE  6700FFEA                  BEQ       -22(PC)              __L00AD
00BFF2  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00BFF6  48E70080            __L00AE:
                                  MOVEM.L   A0,-(A7)            
00BFFA  024000FF                  ANDI.W    #$FF,D0             
00BFFE  3F00                      MOVE.W    D0,-(A7)             ; mode
00C000  2F08                      MOVE.L    A0,-(A7)             ; file name
00C002  3F3C003D                  MOVE.W    #$3D,-(A7)           ; Fopen - Open a file
00C006  4E41                      TRAP      #1     				       ; GEMDOS                  
00C008  508F                      ADDQ.L    #8,A7               
00C00A  4CDF0100                  MOVEM.L   (A7)+,A0            
00C00E  4EB90000C1D2              JSR       $0000C1D2            FN_SHOW_UNPROTECT_DISK_MESSAGE
00C014  6600FFE0                  BNE       -32(PC)              __L00AE
00C018  4E75                      RTS                           


; =============================================================================
00C01A  48E70080            SYS_DISK_FSEEK:
                                  MOVEM.L   A0,-(A7)            
00C01E  3F3C0002                  MOVE.W    #2,-(A7)             ; seekmode (2->from end of file)
00C022  3F00                      MOVE.W    D0,-(A7)             ; handle
00C024  2F3C00000000              MOVE.L    #0,-(A7)             ; offset
00C02A  3F3C0042                  MOVE.W    #$42,-(A7)           ; Fseek - Move file position pointer
00C02E  4E41                      TRAP      #1     				       ; GEMDOS                  
00C030  DFFC0000000A              ADDA.L    #$A,A7              
00C036  4CDF0100                  MOVEM.L   (A7)+,A0            
00C03A  4E75                      RTS                           


; =============================================================================
; TODO: open then close ???
00C03C  48E7FFFE            SYS_DISK_FOPEN_R:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00C040  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00C044  3F3C0019                  MOVE.W    #$19,-(A7)           ; Dgetdrv - Obtain the current drive
00C048  4E41                      TRAP      #1     				       ; GEMDOS                  
00C04A  548F                      ADDQ.L    #2,A7               
00C04C  4EB90000C100              JSR       $0000C100            SYS_DISK_GET_FREE_SPACE
00C052  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00C056  4240                      CLR.W     D0                  
00C058  024000FF                  ANDI.W    #$FF,D0             
00C05C  3F00                      MOVE.W    D0,-(A7)             ; read-only mode
00C05E  2F08                      MOVE.L    A0,-(A7)             ; file name
00C060  3F3C003D                  MOVE.W    #$3D,-(A7)           ; Fopen - Open a file
00C064  4E41                      TRAP      #1     				       ; GEMDOS                  
00C066  508F                      ADDQ.L    #8,A7               
00C068  4A80                      TST.L     D0
00C06A  6B00000A                  BMI       10(PC)               __L00B1
00C06E  4EB90000C07C              JSR       $0000C07C            SYS_DISK_CLOSE_FILE ; WTF ?? on ferme direct ??
00C074  4280                      CLR.L     D0                  
00C076  4CDF7FFF            __L00B1:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00C07A  4E75                      RTS                           


; =============================================================================
; D0 - file handle
00C07C  48E70080            SYS_DISK_CLOSE_FILE:
                                  MOVEM.L   A0,-(A7)            
00C080  3F00                      MOVE.W    D0,-(A7)            
00C082  3F3C003E                  MOVE.W    #$3E,-(A7)           ; Fclose - Close a file
00C086  4E41                      TRAP      #1     				       ; GEMDOS                  
00C088  588F                      ADDQ.L    #4,A7               
00C08A  4CDF0100                  MOVEM.L   (A7)+,A0            
00C08E  4E75                      RTS                           


; =============================================================================                           
00C090  48E70080            SYS_DISK_CREATE_FILE:
                                  MOVEM.L   A0,-(A7)            
00C094  4267                      CLR.W     -(A7)                ; attrs
00C096  2F08                      MOVE.L    A0,-(A7)             ; name
00C098  3F3C003C                  MOVE.W    #$3C,-(A7)           ; Fcreate - Create a file
00C09C  4E41                      TRAP      #1     				       ; GEMDOS                  
00C09E  508F                      ADDQ.L    #8,A7               
00C0A0  4CDF0100                  MOVEM.L   (A7)+,A0            
00C0A4  4EB90000C1D2              JSR       $0000C1D2            FN_SHOW_UNPROTECT_DISK_MESSAGE
00C0AA  6600FFE4                  BNE       -28(PC)              SYS_DISK_CREATE_FILE
00C0AE  4E75                      RTS                           


; =============================================================================                           
; SYS_DISK_DELETE_FILE
00C0B0  48E70080            SYS_DISK_DELETE_FILE:
                                  MOVEM.L   A0,-(A7)            
00C0B4  2F08                      MOVE.L    A0,-(A7)            
00C0B6  3F3C0041                  MOVE.W    #$41,-(A7)           ; Fdelete - Delete a file
00C0BA  4E41                      TRAP      #1     				 ; GEMDOS                  
00C0BC  5C8F                      ADDQ.L    #6,A7               
00C0BE  4CDF0100                  MOVEM.L   (A7)+,A0            
00C0C2  4EB90000C1D2              JSR       $0000C1D2            FN_SHOW_UNPROTECT_DISK_MESSAGE
00C0C8  6600FFE6                  BNE       -26(PC)              SYS_DISK_DELETE_FILE
00C0CC  4E75                      RTS                           


; =============================================================================  
; SYS_DISK_READ_BYTES(word idfich, long nbre, char * buffer)
; D0 handle
; D1 num_bytes
; A1 dest buffer 
00C0CE  2F09                SYS_DISK_READ_BYTES:
                                  MOVE.L    A1,-(A7)            
00C0D0  2F01                      MOVE.L    D1,-(A7)            
00C0D2  3F00                      MOVE.W    D0,-(A7)            
00C0D4  3F3C003F                  MOVE.W    #$3F,-(A7)           ; Fread - Read from a file
00C0D8  4E41                      TRAP      #1     				       ; GEMDOS                  
00C0DA  DFFC0000000C              ADDA.L    #$C,A7              
00C0E0  4E75                      RTS                           


; =============================================================================     
; SYS_DISK_WRITE_BYTES
; D0 handle
; D1 num_bytes
; A1 buffer 
00C0E2  2F09                SYS_DISK_WRITE_BYTES:
                                  MOVE.L    A1,-(A7)            
00C0E4  2F01                      MOVE.L    D1,-(A7)            
00C0E6  3F00                      MOVE.W    D0,-(A7)            
00C0E8  3F3C0040                  MOVE.W    #$40,-(A7)           ; Fwrite - Write to a specified file
00C0EC  4E41                      TRAP      #1     				 ; GEMDOS                  
00C0EE  DFFC0000000C              ADDA.L    #$C,A7              
00C0F4  4EB90000C1D2              JSR       $0000C1D2            FN_SHOW_UNPROTECT_DISK_MESSAGE
00C0FA  6600FFE6                  BNE       -26(PC)              SYS_DISK_WRITE_BYTES
00C0FE  4E75                      RTS                           


; =============================================================================
; SYS_DISK_GET_FREE_SPACE             
00C100  48E74000            SYS_DISK_GET_FREE_SPACE:
                                  MOVEM.L   D1,-(A7)            
00C104  5240                      ADDQ.W    #1,D0               
00C106  3F00                      MOVE.W    D0,-(A7)            
00C108  2F3C00017484              MOVE.L    #$00017484,-(A7)     L071C
00C10E  3F3C0036                  MOVE.W    #$36,-(A7)           ; Dfree - Obtain free and total capacity of a drive
00C112  4E41                      TRAP      #1     				 ; GEMDOS                  
00C114  508F                      ADDQ.L    #8,A7               
00C116  203900017484              MOVE.L    $00017484,D0         L071C
00C11C  323C000A                  MOVE.W    #$A,D1              
00C120  E3A8                      LSL.L     D1,D0               
00C122  80FC03E8                  DIVU      #$3E8,D0            
00C126  4CDF0002                  MOVEM.L   (A7)+,D1            
00C12A  4E75                      RTS                           


; =============================================================================
; a0: pointer to path buffer
00C12C  48E77FFE            SYS_SET_CURRENT_PATH:
                                  MOVEM.L   A0-A6/D1-D7,-(A7)   
00C130  2F08                      MOVE.L    A0,-(A7)            
00C132  3F3C003B                  MOVE.W    #$3B,-(A7)           ; Dsetpath - Set the current directory
00C136  4E41                      TRAP      #1     				       ; GEMDOS                  
00C138  5C8F                      ADDQ.L    #6,A7               
00C13A  4CDF7FFE                  MOVEM.L   (A7)+,A0-A6/D1-D7   
00C13E  4E75                      RTS                           


; =============================================================================
00C140  48E77FFE            L00B9:MOVEM.L   A0-A6/D1-D7,-(A7)   
00C144  4A40                      TST.W     D0                  
00C146  6B00007E                  BMI       126(PC)              __L00C0
00C14A  B03C0001                  CMP.B     #1,D0               
00C14E  67000032                  BEQ       50(PC)               __L00BD
00C152  B03C0002                  CMP.B     #2,D0               
00C156  6700006A                  BEQ       106(PC)              __L00BF
00C15A  48E78000            __L00BA:
                                  MOVEM.L   D0,-(A7)            
00C15E  3F00                      MOVE.W    D0,-(A7)            
00C160  3F3C0007                  MOVE.W    #7,-(A7)             ; Getbpb - Get Bios parameter block
00C164  4E4D                      TRAP      #$D                  ; BIOS                 
00C166  588F                      ADDQ.L    #4,A7               
00C168  4A80                      TST.L     D0                  
00C16A  4CDF0001                  MOVEM.L   (A7)+,D0            
00C16E  6700004A                  BEQ       74(PC)               __L00BE
00C172  3F00                L00BB:MOVE.W    D0,-(A7)            
00C174  3F3C000E                  MOVE.W    #$E,-(A7)            ; Dsetdrv - Set the current drive and returns a bit-map of mounted drives
00C178  4E41                      TRAP      #1     				       ; GEMDOS                  
00C17A  588F                      ADDQ.L    #4,A7               
00C17C  4CDF7FFE            L00BC:MOVEM.L   (A7)+,A0-A6/D1-D7   
00C180  4E75                      RTS                           
00C182  2F00                __L00BD:
                                  MOVE.L    D0,-(A7)            
00C184  42A7                      CLR.L     -(A7)               
00C186  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00C18A  4E41                      TRAP      #1     				       ; GEMDOS                  
00C18C  DFFC00000006              ADDA.L    #6,A7               
00C192  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00C198  3A39000004A6              MOVE.W    $4A6.L,D5           
00C19E  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00C1A4  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00C1A8  4E41                      TRAP      #1     				       ; GEMDOS                  
00C1AA  DFFC00000006              ADDA.L    #6,A7               
00C1B0  201F                      MOVE.L    (A7)+,D0            
00C1B2  BA7C0002                  CMP.W     #2,D5               
00C1B6  6700FFA2                  BEQ       -94(PC)              __L00BA
00C1BA  303CFFFF            __L00BE:
                                  MOVE.W    #$FFFF,D0           
00C1BE  6000FFBC                  BRA       -68(PC)              L00BC
00C1C2  6000FFAE            __L00BF:
                                  BRA       -82(PC)              L00BB
00C1C6  3F3C0019            __L00C0:
                                  MOVE.W    #$19,-(A7)           ; Dgetdrv - Obtain the current drive
00C1CA  4E41                      TRAP      #1     				 ; GEMDOS                  
00C1CC  548F                      ADDQ.L    #2,A7               
00C1CE  6000FFAC                  BRA       -84(PC)              L00BC


; =============================================================================
00C1D2  48E7FFFE            FN_SHOW_UNPROTECT_DISK_MESSAGE:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00C1D6  B07CFFFF                  CMP.W     #$FFFF,D0           
00C1DA  6600003E                  BNE       62(PC)               __L00C3
00C1DE  4240                      CLR.W     D0                  
00C1E0  4241                      CLR.W     D1                  
00C1E2  4EB90000BC6E              JSR       $0000BC6E            SYS_VT52_SET_CURSOR
00C1E8  41F90000C222              LEA       $0000C222,A0         STR_UNPROTECT_DISK
00C1EE  48E78000                  MOVEM.L   D0,-(A7)            
00C1F2  2F08                      MOVE.L    A0,-(A7)            
00C1F4  3F3C0009                  MOVE.W    #9,-(A7)             ; Cconws - Write a string to the standard output device
00C1F8  4E41                      TRAP      #1     				 ; GEMDOS                  
00C1FA  5C8F                      ADDQ.L    #6,A7               
00C1FC  4CDF0001                  MOVEM.L   (A7)+,D0            
00C200  4EB90000B792        __L00C2:
                                  JSR       $0000B792            L0067
00C206  B03C000D                  CMP.B     #$D,D0              
00C20A  6600FFF4                  BNE       -12(PC)              __L00C2
00C20E  13FC0001000170FD          MOVE.B    #1,$000170FD         L069F
00C216  60000004                  BRA       4(PC)                __L00C4
00C21A  B040                __L00C3:
                                  CMP.W     D0,D0               
00C21C  4CDF7FFF            __L00C4:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00C220  4E75                      RTS                           
; =============================================================================
00C222  556E70726F746563    STR_UNPROTECT_DISK:
                                  DC.B      $55,$6E,$70,$72,$6F,$74,$65,$63 
00C22A  7420746865206469          DC.B      $74,$20,$74,$68,$65,$20,$64,$69 
00C232  736B2C207468656E          DC.B      $73,$6B,$2C,$20,$74,$68,$65,$6E 
00C23A  2070726573732052          DC.B      $20,$70,$72,$65,$73,$73,$20,$52 
00C242  455455524E2E0000          DC.B      $45,$54,$55,$52,$4E,$2E,$00,$00 
00C24A  0000                      DC.B      $00,$00 


; =============================================================================
; prepare keyboard config by going in supexec mode
00C24C  48E7E0E0            SYS_SUPEXEC_INIT_KEYBOARD:
                                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00C250  48790000C264              PEA       $0000C264            SYS_INIT_KEYBOARD
00C256  3F3C0026                  MOVE.W    #$26,-(A7)           ; Supexec - Start routine in supervisor-mode
00C25A  4E4E                      TRAP      #$E                  ; XBIOS                 
00C25C  5C8F                      ADDQ.L    #6,A7               
00C25E  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00C262  4E75                      RTS                           


; =============================================================================
; config keyboard
00C264  40E7                SYS_INIT_KEYBOARD:
                                  MOVE      SR,-(A7)            
00C266  007C0700                  ORI.W     #$700,SR            
00C26A  48E7E0E0                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00C26E  08B9000100000484          BCLR      #1,$484.L           
00C276  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00C27C  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00C282  2F3CFFFFFFFF              MOVE.L    #$FFFFFFFF,-(A7)    
00C288  3F3C0010                  MOVE.W    #$10,-(A7)           ; Keytbl - Define the mapping table for the keyboard scancodes to key-codes
00C28C  4E4E                      TRAP      #$E                  ; XBIOS                 
00C28E  DFFC0000000E              ADDA.L    #$E,A7              
00C294  23C00000CB40              MOVE.L    D0,$0000CB40         L0127
00C29A  3F3C0001                  MOVE.W    #1,-(A7)             ; Keyboard
00C29E  3F3C000E                  MOVE.W    #$E,-(A7)            ; Iorec - Obtain address of the input/output buffer of a serial device
00C2A2  4E4E                      TRAP      #$E                  ; XBIOS                 
00C2A4  588F                      ADDQ.L    #4,A7               
00C2A6  23C00000CB44              MOVE.L    D0,$0000CB44         __L0128
00C2AC  23FC0000CA8E0000CAA2      MOVE.L    #$0000CA8E,$0000CAA2 __L0116 L0117
00C2B6  23F9000001180000CA88      MOVE.L    $118.L,$0000CA88     L0114
00C2C0  23FC0000C2F600000118      MOVE.L    #$0000C2F6,$118.L    L00CA
00C2CA  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00C2CE  46DF                      MOVE      (A7)+,SR            
00C2D0  4E75                      RTS                           


; =============================================================================
; prepare keyboard restore by going in supexec mode
00C2D2  48790000C2E2        SYS_SUPEXEC_DEINIT_KEYBOARD:
                                  PEA       $0000C2E2            SYS_DEINIT_KEYBOARD
00C2D8  3F3C0026                  MOVE.W    #$26,-(A7)           ; Supexec - Start routine in supervisor-mode
00C2DC  4E4E                      TRAP      #$E                  ; XBIOS                 
00C2DE  5C8F                      ADDQ.L    #6,A7               
00C2E0  4E75                      RTS                           


; =============================================================================
; restore keyboard config
00C2E2  23F90000CA8800000118 SYS_DEINIT_KEYBOARD:
                                  MOVE.L    $0000CA88,$118.L     L0114 ; User Assigned Interrupt Vectors - ST-MFP-6 - Keyboard/MIDI (ACIA)
00C2EC  08F9000100000484          BSET      #1,$484.L            ; Attribute vector for console output - Key repeat on
00C2F4  4E75                      RTS                           


00C2F6  61000018            L00CA:BSR       24(PC)               L00CB
00C2FA  08390004FFFFFA01          BTST      #4,$FFFFFA01.L       ; MFP 68901 - Multi Function Peripheral Chip
00C302  6700FFF2                  BEQ       -14(PC)              L00CA
00C306  08B90006FFFFFA11          BCLR      #6,$FFFFFA11.L       ; Interrupt In-service B
00C30E  4E73                      RTE                           
00C310  3F00                L00CB:MOVE.W    D0,-(A7)            
00C312  1039FFFFFC00              MOVE.B    $FFFFFC00.L,D0       ; Keyboard ACIA control
00C318  08000007                  BTST      #7,D0               
00C31C  67000020                  BEQ       32(PC)               L00CD
00C320  08000000                  BTST      #0,D0               
00C324  67000018                  BEQ       24(PC)               L00CD
00C328  08000005                  BTST      #5,D0               
00C32C  6700000C                  BEQ       12(PC)               __L00CC
00C330  1039FFFFFC02              MOVE.B    $FFFFFC02.L,D0       ; Keyboard ACIA data
00C336  60000006                  BRA       6(PC)                L00CD
00C33A  61000006            __L00CC:
                                  BSR       6(PC)                L00CE
00C33E  301F                L00CD:MOVE.W    (A7)+,D0            
00C340  4E75                      RTS                           
00C342  3F00                L00CE:MOVE.W    D0,-(A7)            
00C344  1039FFFFFC02              MOVE.B    $FFFFFC02.L,D0       ; Keyboard ACIA data
00C34A  4A390000CA85              TST.B     $0000CA85            L0112
00C350  6600001A                  BNE       26(PC)               __L00D0
00C354  0C0000F6                  CMPI.B    #$F6,D0             
00C358  6200000A                  BHI       10(PC)               __L00CF
00C35C  610000F2                  BSR       242(PC)              __L00DD
00C360  60000036                  BRA       54(PC)               L00D3
00C364  610004B4            __L00CF:
                                  BSR       1204(PC)             L0107
00C368  6000002E                  BRA       46(PC)               L00D3
00C36C  0C3900FD0000CA84    __L00D0:
                                  CMPI.B    #$FD,$0000CA84       L0111
00C374  6200001E                  BHI       30(PC)               __L00D2
00C378  0C3900FC0000CA84          CMPI.B    #$FC,$0000CA84       __L0111
00C380  6700000A                  BEQ       10(PC)               __L00D1
00C384  61000422                  BSR       1058(PC)             __L0101
00C388  6000000E                  BRA       14(PC)               L00D3
00C38C  6100000E            __L00D1:
                                  BSR       14(PC)               L00D4
00C390  60000006                  BRA       6(PC)                L00D3
00C394  610003C6            __L00D2:
                                  BSR       966(PC)              L00FE
00C398  301F                L00D3:MOVE.W    (A7)+,D0            
00C39A  4E75                      RTS                           
00C39C  3F00                L00D4:MOVE.W    D0,-(A7)            
00C39E  53390000CA85              SUBQ.B    #1,$0000CA85         L0112
00C3A4  0C3900050000CA85          CMPI.B    #5,$0000CA85         L0112
00C3AC  67000036                  BEQ       54(PC)               __L00D5
00C3B0  0C3900040000CA85          CMPI.B    #4,$0000CA85         L0112
00C3B8  67000038                  BEQ       56(PC)               __L00D6
00C3BC  0C3900030000CA85          CMPI.B    #3,$0000CA85         L0112
00C3C4  6700003A                  BEQ       58(PC)               __L00D7
00C3C8  0C3900020000CA85          CMPI.B    #2,$0000CA85         L0112
00C3D0  6700003C                  BEQ       60(PC)               __L00D8
00C3D4  0C3900010000CA85          CMPI.B    #1,$0000CA85         L0112
00C3DC  6700003E                  BEQ       62(PC)               __L00D9
00C3E0  60000048                  BRA       72(PC)               __L00DA
00C3E4  61000056            __L00D5:
                                  BSR       86(PC)               L00DC
00C3E8  13C00000CB39              MOVE.B    D0,$0000CB39         __L0121
00C3EE  60000048                  BRA       72(PC)               L00DB
00C3F2  61000048            __L00D6:
                                  BSR       72(PC)               L00DC
00C3F6  13C00000CB3A              MOVE.B    D0,$0000CB3A         __L0122
00C3FC  6000003A                  BRA       58(PC)               L00DB
00C400  6100003A            __L00D7:
                                  BSR       58(PC)               L00DC
00C404  13C00000CB3B              MOVE.B    D0,$0000CB3B         __L0123
00C40A  6000002C                  BRA       44(PC)               L00DB
00C40E  6100002C            __L00D8:
                                  BSR       44(PC)               L00DC
00C412  13C00000CB3C              MOVE.B    D0,$0000CB3C         L0124
00C418  6000001E                  BRA       30(PC)               L00DB
00C41C  6100001E            __L00D9:
                                  BSR       30(PC)               L00DC
00C420  13C00000CB3D              MOVE.B    D0,$0000CB3D         L0125
00C426  60000010                  BRA       16(PC)               L00DB
00C42A  61000010            __L00DA:
                                  BSR       16(PC)               L00DC
00C42E  13C00000CB3E              MOVE.B    D0,$0000CB3E         L0126
00C434  60000002                  BRA       2(PC)                L00DB
00C438  301F                L00DB:MOVE.W    (A7)+,D0            
00C43A  4E75                      RTS                           
00C43C  2F01                L00DC:MOVE.L    D1,-(A7)            
00C43E  1200                      MOVE.B    D0,D1               
00C440  E809                      LSR.B     #4,D1               
00C442  C2FC000A                  MULU      #$A,D1              
00C446  0200000F                  ANDI.B    #$F,D0              
00C44A  D001                      ADD.B     D1,D0               
00C44C  221F                      MOVE.L    (A7)+,D1            
00C44E  4E75                      RTS                           
00C450  61000010            __L00DD:
                                  BSR       16(PC)               L00DE
00C454  6100003C                  BSR       60(PC)               __L00E0
00C458  6100015A                  BSR       346(PC)              __L00E9
00C45C  61000228                  BSR       552(PC)              __L00F5
00C460  4E75                      RTS                           
00C462  48E78080            L00DE:MOVEM.L   A0/D0,-(A7)         
00C466  41F90000CAB7              LEA       $0000CAB7,A0         __L011E
00C46C  4A00                      TST.B     D0                  
00C46E  6B000010                  BMI       16(PC)               __L00DF
00C472  4880                      EXT.W     D0                  
00C474  11BC00010000              MOVE.B    #1,0(A0,D0.W)       
00C47A  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
00C47E  4E75                      RTS                           
00C480  08800007            __L00DF:
                                  BCLR      #7,D0               
00C484  4880                      EXT.W     D0                  
00C486  11BC00000000              MOVE.B    #0,0(A0,D0.W)       
00C48C  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
00C490  4E75                      RTS                           
00C492  48E7F080            __L00E0:
                                  MOVEM.L   A0/D0-D3,-(A7)      
00C496  0C000036                  CMPI.B    #$36,D0             ;(#"6") 
00C49A  67000112                  BEQ       274(PC)              L00E8
00C49E  0C00002A                  CMPI.B    #$2A,D0             ;(#"*") 
00C4A2  6700010A                  BEQ       266(PC)              L00E8
00C4A6  0C00001D                  CMPI.B    #$1D,D0             
00C4AA  67000102                  BEQ       258(PC)              L00E8
00C4AE  0C000038                  CMPI.B    #$38,D0             ;(#"8") 
00C4B2  670000FA                  BEQ       250(PC)              L00E8
00C4B6  0C00003A                  CMPI.B    #$3A,D0             ;(#":") 
00C4BA  670000F2                  BEQ       242(PC)              L00E8
00C4BE  0C0000B6                  CMPI.B    #$B6,D0             
00C4C2  670000EA                  BEQ       234(PC)              L00E8
00C4C6  0C0000AA                  CMPI.B    #$AA,D0             
00C4CA  670000E2                  BEQ       226(PC)              L00E8
00C4CE  0C00009D                  CMPI.B    #$9D,D0             
00C4D2  670000DA                  BEQ       218(PC)              L00E8
00C4D6  0C0000B8                  CMPI.B    #$B8,D0             
00C4DA  670000D2                  BEQ       210(PC)              L00E8
00C4DE  0C0000BA                  CMPI.B    #$BA,D0             
00C4E2  670000CA                  BEQ       202(PC)              L00E8
00C4E6  4A00                      TST.B     D0                  
00C4E8  6B00008C                  BMI       140(PC)              __L00E6
00C4EC  083900000000CB37          BTST      #0,$0000CB37         L011F
00C4F4  66000012                  BNE       18(PC)               L00E1
00C4F8  083900010000CB37          BTST      #1,$0000CB37         L011F
00C500  66000006                  BNE       6(PC)                L00E1
00C504  60000010                  BRA       16(PC)               __L00E2
00C508  20790000CB40        L00E1:MOVEA.L   $0000CB40,A0         L0127
00C50E  20680004                  MOVEA.L   4(A0),A0            
00C512  60000024                  BRA       36(PC)               L00E4
00C516  083900040000CB37    __L00E2:
                                  BTST      #4,$0000CB37         L011F
00C51E  67000010                  BEQ       16(PC)               __L00E3
00C522  20790000CB40              MOVEA.L   $0000CB40,A0         L0127
00C528  20680008                  MOVEA.L   8(A0),A0            
00C52C  6000000A                  BRA       10(PC)               L00E4
00C530  20790000CB40        __L00E3:
                                  MOVEA.L   $0000CB40,A0         L0127
00C536  2050                      MOVEA.L   (A0),A0             
00C538  4880                L00E4:EXT.W     D0                  
00C53A  13F000000000CB38          MOVE.B    0(A0,D0.W),$0000CB38 L0120
00C542  4A390000CB38              TST.B     $0000CB38            L0120
00C548  66000010                  BNE       16(PC)               __L00E5
00C54C  13C00000CB38              MOVE.B    D0,$0000CB38         L0120
00C552  063900800000CB38          ADDI.B    #$80,$0000CB38       L0120
00C55A  20790000CAA2        __L00E5:
                                  MOVEA.L   $0000CAA2,A0         L0117
00C560  12390000CB38              MOVE.B    $0000CB38,D1         L0120
00C566  E149                      LSL.W     #8,D1               
00C568  1200                      MOVE.B    D0,D1               
00C56A  30C1                      MOVE.W    D1,(A0)+            
00C56C  23C80000CAA2              MOVE.L    A0,$0000CAA2         L0117
00C572  6000003A                  BRA       58(PC)               L00E8
00C576  08800007            __L00E6:
                                  BCLR      #7,D0               
00C57A  20790000CAA2              MOVEA.L   $0000CAA2,A0         L0117
00C580  720A                      MOVEQ     #$A,D1              
00C582  B1FC0000CA8C        __L00E7:
                                  CMPA.L    #$0000CA8C,A0        L0115
00C588  63000024                  BLS       36(PC)               L00E8
00C58C  3420                      MOVE.W    -(A0),D2            
00C58E  3083                      MOVE.W    D3,(A0)             
00C590  3602                      MOVE.W    D2,D3               
00C592  B400                      CMP.B     D0,D2               
00C594  57C9FFEC                  DBEQ      D1,-20(PC)           __L00E7
00C598  20790000CAA2              MOVEA.L   $0000CAA2,A0         L0117
00C59E  5548                      SUBQ.W    #2,A0               
00C5A0  13E8FFFE0000CB38          MOVE.B    -2(A0),$0000CB38     L0120
00C5A8  23C80000CAA2              MOVE.L    A0,$0000CAA2         L0117
00C5AE  4CDF010F            L00E8:MOVEM.L   (A7)+,A0/D0-D3      
00C5B2  4E75                      RTS                           
00C5B4  0C000036            __L00E9:
                                  CMPI.B    #$36,D0             ;(#"6") 
00C5B8  6600000A                  BNE       10(PC)               __L00EA
00C5BC  08F900000000CB37          BSET      #0,$0000CB37         L011F
00C5C4  0C00002A            __L00EA:
                                  CMPI.B    #$2A,D0             ;(#"*") 
00C5C8  6600000A                  BNE       10(PC)               __L00EB
00C5CC  08F900010000CB37          BSET      #1,$0000CB37         L011F
00C5D4  0C00001D            __L00EB:
                                  CMPI.B    #$1D,D0             
00C5D8  6600000A                  BNE       10(PC)               __L00EC
00C5DC  08F900020000CB37          BSET      #2,$0000CB37         L011F
00C5E4  0C000038            __L00EC:
                                  CMPI.B    #$38,D0             ;(#"8") 
00C5E8  6600000A                  BNE       10(PC)               __L00ED
00C5EC  08F900030000CB37          BSET      #3,$0000CB37         L011F
00C5F4  0C00003A            __L00ED:
                                  CMPI.B    #$3A,D0             ;(#":") 
00C5F8  6600000A                  BNE       10(PC)               __L00EE
00C5FC  087900040000CB37          BCHG      #4,$0000CB37         L011F
00C604  0C0000B6            __L00EE:
                                  CMPI.B    #$B6,D0             
00C608  6600000A                  BNE       10(PC)               __L00EF
00C60C  08B900000000CB37          BCLR      #0,$0000CB37         L011F
00C614  0C0000AA            __L00EF:
                                  CMPI.B    #$AA,D0             
00C618  6600000A                  BNE       10(PC)               __L00F0
00C61C  08B900010000CB37          BCLR      #1,$0000CB37         L011F
00C624  0C00009D            __L00F0:
                                  CMPI.B    #$9D,D0             
00C628  6600000A                  BNE       10(PC)               __L00F1
00C62C  08B900020000CB37          BCLR      #2,$0000CB37         L011F
00C634  0C0000B8            __L00F1:
                                  CMPI.B    #$B8,D0             
00C638  6600000A                  BNE       10(PC)               __L00F2
00C63C  08B900030000CB37          BCLR      #3,$0000CB37         L011F
00C644  61000004            __L00F2:
                                  BSR       4(PC)                L00F3
00C648  4E75                      RTS                           
00C64A  4A390000AC77        L00F3:TST.B     $0000AC77            L0016
00C650  66000004                  BNE       4(PC)                __L00F4
00C654  4E75                      RTS                           
00C656  48E7E0E0            __L00F4:
                                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00C65A  04B900000046000004A2      SUBI.L    #$46,$4A2.L         
00C664  10390000CB37              MOVE.B    $0000CB37,D0         L011F
00C66A  4880                      EXT.W     D0                  
00C66C  3F00                      MOVE.W    D0,-(A7)            
00C66E  3F3C000B                  MOVE.W    #$B,-(A7)            ; Kbshift - Keyboard shift status of the 'special' keys
00C672  4E4D                      TRAP      #$D                  ; BIOS                 
00C674  584F                      ADDQ.W    #4,A7               
00C676  06B900000046000004A2      ADDI.L    #$46,$4A2.L         
00C680  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00C684  4E75                      RTS                           
00C686  0C39000D0000CB38    __L00F5:
                                  CMPI.B    #$D,$0000CB38        L0120
00C68E  6700000E                  BEQ       14(PC)               L00F6
00C692  4A390000AC77              TST.B     $0000AC77            L0016
00C698  66000004                  BNE       4(PC)                L00F6
00C69C  4E75                      RTS                           
00C69E  48E7C080            L00F6:MOVEM.L   A0/D0-D1,-(A7)      
00C6A2  4A00                      TST.B     D0                  
00C6A4  6B0000B0                  BMI       176(PC)              L00FD
00C6A8  0C000036                  CMPI.B    #$36,D0             ;(#"6") 
00C6AC  670000A8                  BEQ       168(PC)              L00FD
00C6B0  0C00002A                  CMPI.B    #$2A,D0             ;(#"*") 
00C6B4  670000A0                  BEQ       160(PC)              L00FD
00C6B8  0C00001D                  CMPI.B    #$1D,D0             
00C6BC  67000098                  BEQ       152(PC)              L00FD
00C6C0  0C000038                  CMPI.B    #$38,D0             ;(#"8") 
00C6C4  67000090                  BEQ       144(PC)              L00FD
00C6C8  0C00003A                  CMPI.B    #$3A,D0             ;(#":") 
00C6CC  67000088                  BEQ       136(PC)              L00FD
00C6D0  0280000000FF              ANDI.L    #$FF,D0             
00C6D6  083900020000CB37          BTST      #2,$0000CB37         L011F
00C6DE  66000012                  BNE       18(PC)               __L00F7
00C6E2  083900030000CB37          BTST      #3,$0000CB37         L011F
00C6EA  66000010                  BNE       16(PC)               __L00F8
00C6EE  6000002E                  BRA       46(PC)               __L00FA
00C6F2  41F90000C8B8        __L00F7:
                                  LEA       $0000C8B8,A0         L010F
00C6F8  60000008                  BRA       8(PC)                __L00F9
00C6FC  41F90000C99E        __L00F8:
                                  LEA       $0000C99E,A0         L0110
00C702  D040                __L00F9:
                                  ADD.W     D0,D0               
00C704  30300000                  MOVE.W    0(A0,D0.W),D0       
00C708  3200                      MOVE.W    D0,D1               
00C70A  0280000000FF              ANDI.L    #$FF,D0             
00C710  02810000FF00              ANDI.L    #$FF00,D1           
00C716  E189                      LSL.L     #8,D1               
00C718  D081                      ADD.L     D1,D0               
00C71A  60000012                  BRA       18(PC)               L00FB
00C71E  4840                __L00FA:
                                  SWAP      D0                  
00C720  10390000CB38              MOVE.B    $0000CB38,D0         L0120
00C726  4A00                      TST.B     D0                  
00C728  6A000004                  BPL       4(PC)                L00FB
00C72C  4200                      CLR.B     D0                  
00C72E  20790000CB44        L00FB:MOVEA.L   $0000CB44,A0         L0128
00C734  32280008                  MOVE.W    8(A0),D1            
00C738  5841                      ADDQ.W    #4,D1               
00C73A  B2680004                  CMP.W     4(A0),D1            
00C73E  65000004                  BCS       4(PC)                __L00FC
00C742  4241                      CLR.W     D1                  
00C744  B2680006            __L00FC:
                                  CMP.W     6(A0),D1            
00C748  6700000C                  BEQ       12(PC)               L00FD
00C74C  31410008                  MOVE.W    D1,8(A0)            
00C750  2050                      MOVEA.L   (A0),A0             
00C752  21801000                  MOVE.L    D0,0(A0,D1.W)       
00C756  4CDF0103            L00FD:MOVEM.L   (A7)+,A0/D0-D1      
00C75A  4E75                      RTS                           
00C75C  4A390000CA86        L00FE:TST.B     $0000CA86            L0113
00C762  66000010                  BNE       16(PC)               __L00FF
00C766  13C00000CAAF              MOVE.B    D0,$0000CAAF         L0119
00C76C  42390000CA85              CLR.B     $0000CA85            L0112
00C772  4E75                      RTS                           
00C774  0C3900020000CAAE    __L00FF:
                                  CMPI.B    #2,$0000CAAE         L0118
00C77C  6700001C                  BEQ       28(PC)               __L0100
00C780  0200000F                  ANDI.B    #$F,D0              
00C784  023900F00000CAB0          ANDI.B    #$F0,$0000CAB0       L011A
00C78C  81390000CAB0              OR.B      D0,$0000CAB0         L011A
00C792  42390000CA85              CLR.B     $0000CA85            L0112
00C798  4E75                      RTS                           
00C79A  13C00000CAB0        __L0100:
                                  MOVE.B    D0,$0000CAB0         L011A
00C7A0  42390000CA85              CLR.B     $0000CA85            L0112
00C7A6  4E75                      RTS                           
00C7A8  4880                __L0101:
                                  EXT.W     D0                  
00C7AA  0C3900010000CA85          CMPI.B    #1,$0000CA85         L0112
00C7B2  67000034                  BEQ       52(PC)               __L0104
00C7B6  D1790000CAB2              ADD.W     D0,$0000CAB2         L011B
00C7BC  4A790000CAB2              TST.W     $0000CAB2            L011B
00C7C2  6A000008                  BPL       8(PC)                __L0102
00C7C6  42790000CAB2              CLR.W     $0000CAB2            L011B
00C7CC  0C79013F0000CAB2    __L0102:
                                  CMPI.W    #$13F,$0000CAB2      L011B
00C7D4  6300000A                  BLS       10(PC)               __L0103
00C7D8  33FC013F0000CAB2          MOVE.W    #$13F,$0000CAB2      L011B
00C7E0  53390000CA85        __L0103:
                                  SUBQ.B    #1,$0000CA85         L0112
00C7E6  4E75                      RTS                           
00C7E8  D1790000CAB4        __L0104:
                                  ADD.W     D0,$0000CAB4         L011C
00C7EE  4A790000CAB4              TST.W     $0000CAB4            L011C
00C7F4  6A000008                  BPL       8(PC)                __L0105
00C7F8  42790000CAB4              CLR.W     $0000CAB4            L011C
00C7FE  0C7900C70000CAB4    __L0105:
                                  CMPI.W    #$C7,$0000CAB4       L011C
00C806  6300000A                  BLS       10(PC)               __L0106
00C80A  33FC00C70000CAB4          MOVE.W    #$C7,$0000CAB4       L011C
00C812  42390000CA85        __L0106:
                                  CLR.B     $0000CA85            L0112
00C818  4E75                      RTS                           
00C81A  13C00000CA84        L0107:MOVE.B    D0,$0000CA84         L0111
00C820  0C0000FD                  CMPI.B    #$FD,D0             
00C824  6200005C                  BHI       92(PC)               __L010C
00C828  0C0000FC                  CMPI.B    #$FC,D0             
00C82C  67000080                  BEQ       128(PC)              __L010E
00C830  08000000                  BTST      #0,D0               
00C834  67000016                  BEQ       22(PC)               __L0108
00C838  08F900010000CAB6          BSET      #1,$0000CAB6         L011D
00C840  08F900070000CAB0          BSET      #7,$0000CAB0         L011A
00C848  60000012                  BRA       18(PC)               __L0109
00C84C  08B900010000CAB6    __L0108:
                                  BCLR      #1,$0000CAB6         L011D
00C854  08B900070000CAB0          BCLR      #7,$0000CAB0         L011A
00C85C  08000001            __L0109:
                                  BTST      #1,D0               
00C860  6700000E                  BEQ       14(PC)               __L010A
00C864  08F900000000CAB6          BSET      #0,$0000CAB6         L011D
00C86C  6000000A                  BRA       10(PC)               __L010B
00C870  08B900000000CAB6    __L010A:
                                  BCLR      #0,$0000CAB6         L011D
00C878  13FC00020000CA85    __L010B:
                                  MOVE.B    #2,$0000CA85         L0112
00C880  4E75                      RTS                           
00C882  08000000            __L010C:
                                  BTST      #0,D0               
00C886  66000014                  BNE       20(PC)               __L010D
00C88A  13FC00000000CA86          MOVE.B    #0,$0000CA86         __L0113
00C892  13FC00010000CA85          MOVE.B    #1,$0000CA85         L0112
00C89A  4E75                      RTS                           
00C89C  13FC00010000CA86    __L010D:
                                  MOVE.B    #1,$0000CA86         L0113
00C8A4  13FC00010000CA85          MOVE.B    #1,$0000CA85         L0112
00C8AC  4E75                      RTS                           
00C8AE  13FC00060000CA85    __L010E:
                                  MOVE.B    #6,$0000CA85         L0112
00C8B6  4E75                      RTS                           
00C8B8  0000011B02060302    L010F:DC.B      $00,$00,$01,$1B,$02,$06,$03,$02 
00C8C0  040205070608071D          DC.B      $04,$02,$05,$07,$06,$08,$07,$1D 
00C8C8  080A09010A070B05          DC.B      $08,$0A,$09,$01,$0A,$07,$0B,$05 
00C8D0  0C090D1F0E080F09          DC.B      $0C,$09,$0D,$1F,$0E,$08,$0F,$09 
00C8D8  1001111A12051312          DC.B      $10,$01,$11,$1A,$12,$05,$13,$12 
00C8E0  1414151916151709          DC.B      $14,$14,$15,$19,$16,$15,$17,$09 
00C8E8  180F19101A1E1B04          DC.B      $18,$0F,$19,$10,$1A,$1E,$1B,$04 
00C8F0  1C0A00001E111F13          DC.B      $1C,$0A,$00,$00,$1E,$11,$1F,$13 
00C8F8  2004210622072308          DC.B      $20,$04,$21,$06,$22,$07,$23,$08 
00C900  240A250B260C270D          DC.B      $24,$0A,$25,$0B,$26,$0C,$27,$0D 
00C908  2817290000002B03          DC.B      $28,$17,$29,$00,$00,$00,$2B,$03 
00C910  2C172D182E032F16          DC.B      $2C,$17,$2D,$18,$2E,$03,$2F,$16 
00C918  3002310E320C331B          DC.B      $30,$02,$31,$0E,$32,$0C,$33,$1B 
00C920  341A351D00000000          DC.B      $34,$1A,$35,$1D,$00,$00,$00,$00 
00C928  0000390000003B00          DC.B      $00,$00,$39,$00,$00,$00,$3B,$00 
00C930  3C003D003E003F00          DC.B      $3C,$00,$3D,$00,$3E,$00,$3F,$00 
00C938  4000410042004300          DC.B      $40,$00,$41,$00,$42,$00,$43,$00 
00C940  4400000000007700          DC.B      $44,$00,$00,$00,$00,$00,$77,$00 
00C948  480000004A1F7300          DC.B      $48,$00,$00,$00,$4A,$1F,$73,$00 
00C950  000074004E0B0000          DC.B      $00,$00,$74,$00,$4E,$0B,$00,$00 
00C958  500000005200531F          DC.B      $50,$00,$00,$00,$52,$00,$53,$1F 
00C960                            DCB.W     12,0
00C978  601C                      DC.B      $60,$1C 
00C97A  61006200                  BSR       25088(PC)            __L0418
00C97E  63086409650F660A          DC.B      $63,$08,$64,$09,$65,$0F,$66,$0A 
00C986  6717681869196A14          DC.B      $67,$17,$68,$18,$69,$19,$6A,$14 
00C98E  6B156C136D116E00          DC.B      $6B,$15,$6C,$13,$6D,$11,$6E,$00 
00C996  6F137010710E720A          DC.B      $6F,$13,$70,$10,$71,$0E,$72,$0A 
00C99E  0000011B78007900    L0110:DC.B      $00,$00,$01,$1B,$78,$00,$79,$00 
00C9A6  7A007B007C007D00          DC.B      $7A,$00,$7B,$00,$7C,$00,$7D,$00 
00C9AE  7E007F0080008100          DC.B      $7E,$00,$7F,$00,$80,$00,$81,$00 
00C9B6  830083000E080F09          DC.B      $83,$00,$83,$00,$0E,$08,$0F,$09 
00C9BE  1000110012001300          DC.B      $10,$00,$11,$00,$12,$00,$13,$00 
00C9C6  1400150016001700          DC.B      $14,$00,$15,$00,$16,$00,$17,$00 
00C9CE  180019001A5B1B5D          DC.B      $18,$00,$19,$00,$1A,$5B,$1B,$5D 
00C9D6  1C0D00001E001F00          DC.B      $1C,$0D,$00,$00,$1E,$00,$1F,$00 
00C9DE  2000210022002300          DC.B      $20,$00,$21,$00,$22,$00,$23,$00 
00C9E6  2400250026002700          DC.B      $24,$00,$25,$00,$26,$00,$27,$00 
00C9EE  285C296000002B40          DC.B      $28,$5C,$29,$60,$00,$00,$2B,$40 
00C9F6  2C002D002E002F00          DC.B      $2C,$00,$2D,$00,$2E,$00,$2F,$00 
00C9FE  30003100322C333B          DC.B      $30,$00,$31,$00,$32,$2C,$33,$3B 
00CA06  343A353D00000000          DC.B      $34,$3A,$35,$3D,$00,$00,$00,$00 
00CA0E  0000392000003B00          DC.B      $00,$00,$39,$20,$00,$00,$3B,$00 
00CA16  3C003D003E003F00          DC.B      $3C,$00,$3D,$00,$3E,$00,$3F,$00 
00CA1E  4000410042004300          DC.B      $40,$00,$41,$00,$42,$00,$43,$00 
00CA26  4400000000004700          DC.B      $44,$00,$00,$00,$00,$00,$47,$00 
00CA2E  480000004A2D4B00          DC.B      $48,$00,$00,$00,$4A,$2D,$4B,$00 
00CA36  00004D004E2B0000          DC.B      $00,$00,$4D,$00,$4E,$2B,$00,$00 
00CA3E  500000005200537F          DC.B      $50,$00,$00,$00,$52,$00,$53,$7F 
00CA46                            DCB.W     12,0
00CA5E  603C610062006328          DC.B      $60,$3C,$61,$00,$62,$00,$63,$28 
00CA66  6429652F662A6737          DC.B      $64,$29,$65,$2F,$66,$2A,$67,$37 
00CA6E  683869396A346B35          DC.B      $68,$38,$69,$39,$6A,$34,$6B,$35 
00CA76  6C366D316E326F33          DC.B      $6C,$36,$6D,$31,$6E,$32,$6F,$33 
00CA7E  7030712E720D              DC.B      $70,$30,$71,$2E,$72,$0D 
00CA84  00                  __L0111:
                                  DC.B      $00 
00CA85  00                  L0112:DC.B      $00 
00CA86  0000                __L0113:
                                  DC.B      $00,$00 
00CA88                      L0114:DCB.W     2,0 
00CA8C  0000                L0115:DC.B      $00,$00 
00CA8E                      __L0116:
                                  DCB.W     10,0
00CAA2                      L0117:DCB.W     6,0 
00CAAE  01                  L0118:DC.B      $01 
00CAAF  00                  L0119:DC.B      $00 
00CAB0  0000                L011A:DC.B      $00,$00 
00CAB2  00A0                L011B:DC.B      $00,$A0 
00CAB4  0064                L011C:DC.B      $00,$64 
00CAB6  00                  L011D:DC.B      $00 
00CAB7  000000000000000000  __L011E:
                                  DC.B      $00,$00,$00,$00,$00,$00,$00,$00,$00 
00CAC0                            DCB.W     59,0
00CB36  00                        DC.B      $00 
00CB37  00                  L011F:DC.B      $00 
00CB38  00                  L0120:DC.B      $00 
00CB39  00                  __L0121:
                                  DC.B      $00 
00CB3A  00                  __L0122:
                                  DC.B      $00 
00CB3B  00                  __L0123:
                                  DC.B      $00 
00CB3C  00                  L0124:DC.B      $00 
00CB3D  00                  L0125:DC.B      $00 
00CB3E  0000                L0126:DC.B      $00,$00 
00CB40                      L0127:DCB.W     2,0 
00CB44                      __L0128:
                                  DCB.W     2,0 
00CB48  48E7E0E0                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00CB4C  48790000CB64              PEA       $0000CB64            __L0129
00CB52  3F3C0000                  MOVE.W    #0,-(A7)            
00CB56  3F3C0019                  MOVE.W    #$19,-(A7)           ; Ikbdws - Program the keyboard chip
00CB5A  4E4E                      TRAP      #$E                  ; XBIOS                 
00CB5C  508F                      ADDQ.L    #8,A7               
00CB5E  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00CB62  4E75                      RTS                           
00CB64  1C00                __L0129:
                                  MOVE.B    D0,D6               
00CB66  48E7E0E0            __L012A:
                                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00CB6A  13FC00020000CAAE          MOVE.B    #2,$0000CAAE         L0118
00CB72  48790000CB8A              PEA       $0000CB8A            __L012B
00CB78  3F3C0000                  MOVE.W    #0,-(A7)            
00CB7C  3F3C0019                  MOVE.W    #$19,-(A7)           ; Ikbdws - Program the keyboard chip
00CB80  4E4E                      TRAP      #$E                  ; XBIOS                 
00CB82  508F                      ADDQ.L    #8,A7               
00CB84  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00CB88  4E75                      RTS                           
00CB8A  1400                __L012B:
                                  MOVE.B    D0,D2               
00CB8C  48E7E0E0            __L012C:
                                  MOVEM.L   A0-A2/D0-D2,-(A7)   
00CB90  13FC00010000CAAE          MOVE.B    #1,$0000CAAE         L0118
00CB98  48790000CBB0              PEA       $0000CBB0            __L012D
00CB9E  3F3C0000                  MOVE.W    #0,-(A7)            
00CBA2  3F3C0019                  MOVE.W    #$19,-(A7)           ; Ikbdws - Program the keyboard chip
00CBA6  4E4E                      TRAP      #$E                  ; XBIOS                 
00CBA8  508F                      ADDQ.L    #8,A7               
00CBAA  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00CBAE  4E75                      RTS                           
00CBB0  0801                __L012D:
                                  DC.B      $08,$01 
00CBB2  4E75                      RTS                           
00CBB4  0839000000017080    __L012E:
                                  BTST      #0,$00017080         L066C
00CBBC  66000004                  BNE       4(PC)                __L012F
00CBC0  4E75                      RTS                           
00CBC2  48E7FFFE            __L012F:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00CBC6  2449                      MOVEA.L   A1,A2               
00CBC8  D5EA000E                  ADDA.L    14(A2),A2           
00CBCC  342A0010                  MOVE.W    16(A2),D2           
00CBD0  670000F2                  BEQ       242(PC)              __L013A
00CBD4  D5EA000C                  ADDA.L    12(A2),A2           
00CBD8  5342                      SUBQ.W    #1,D2               
00CBDA  264A                L0130:MOVEA.L   A2,A3               
00CBDC  D7DA                      ADDA.L    (A2)+,A3            
00CBDE  0C130001                  CMPI.B    #1,(A3)             
00CBE2  6700000E                  BEQ       14(PC)               L0131
00CBE6  0C130002                  CMPI.B    #2,(A3)             
00CBEA  67000006                  BEQ       6(PC)                L0131
00CBEE  600000D0                  BRA       208(PC)              L0139
00CBF2  0C2B00010006        L0131:CMPI.B    #1,6(A3)            
00CBF8  660000C6                  BNE       198(PC)              L0139
00CBFC  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00CC00  45F90000CCCA              LEA       $0000CCCA,A2         __L013B
00CC06  43EB0010                  LEA       16(A3),A1           
00CC0A  20790000AC5E              MOVEA.L   $0000AC5E,A0         L000E
00CC10  282B0002                  MOVE.L    2(A3),D4            
00CC14  048400000010              SUBI.L    #$10,D4             
00CC1A  E28C                      LSR.L     #1,D4               
00CC1C  2604                      MOVE.L    D4,D3               
00CC1E  48E700C0                  MOVEM.L   A0-A1,-(A7)         
00CC22  E28B                      LSR.L     #1,D3               
00CC24  048300000001              SUBI.L    #1,D3               
00CC2A  30D9                __L0132:
                                  MOVE.W    (A1)+,(A0)+         
00CC2C  51CBFFFC                  DBF       D3,-4(PC)            __L0132
00CC30  4CDF0300                  MOVEM.L   (A7)+,A0-A1         
00CC34  2604                      MOVE.L    D4,D3               
00CC36  048300000002              SUBI.L    #2,D3               
00CC3C  1418                      MOVE.B    (A0)+,D2            
00CC3E  12C2                      MOVE.B    D2,(A1)+            
00CC40  4241                      CLR.W     D1                  
00CC42  4240                      CLR.W     D0                  
00CC44  1018                __L0133:
                                  MOVE.B    (A0)+,D0            
00CC46  1200                      MOVE.B    D0,D1               
00CC48  E809                      LSR.B     #4,D1               
00CC4A  D4321000                  ADD.B     0(A2,D1.W),D2       
00CC4E  12C2                      MOVE.B    D2,(A1)+            
00CC50  0200000F                  ANDI.B    #$F,D0              
00CC54  D4320000                  ADD.B     0(A2,D0.W),D2       
00CC58  12C2                      MOVE.B    D2,(A1)+            
00CC5A  51CBFFE8                  DBF       D3,-24(PC)           __L0133
00CC5E  4229FFFE                  CLR.B     -2(A1)              
00CC62  4229FFFF                  CLR.B     -1(A1)              
00CC66  04AB000000020002          SUBI.L    #2,2(A3)            
00CC6E  0C130002                  CMPI.B    #2,(A3)             
00CC72  66000048                  BNE       72(PC)               __L0138
00CC76  4AAB000C                  TST.L     12(A3)              
00CC7A  6700001A                  BEQ       26(PC)               __L0135
00CC7E  204B                      MOVEA.L   A3,A0               
00CC80  D1FC00000010              ADDA.L    #$10,A0             
00CC86  D1EB000C                  ADDA.L    12(A3),A0           
00CC8A  303C0007                  MOVE.W    #7,D0               
00CC8E  10FC00FF            __L0134:
                                  MOVE.B    #$FF,(A0)+          
00CC92  51C8FFFA                  DBF       D0,-6(PC)            __L0134
00CC96  204B                __L0135:
                                  MOVEA.L   A3,A0               
00CC98  D1EB0002                  ADDA.L    2(A3),A0            
00CC9C  91FC00000010              SUBA.L    #$10,A0             
00CCA2  4AAB000C                  TST.L     12(A3)              
00CCA6  67000008                  BEQ       8(PC)                __L0136
00CCAA  D1FC00000008              ADDA.L    #8,A0               
00CCB0  303C0007            __L0136:
                                  MOVE.W    #7,D0               
00CCB4  10FC00FF            __L0137:
                                  MOVE.B    #$FF,(A0)+          
00CCB8  51C8FFFA                  DBF       D0,-6(PC)            __L0137
00CCBC  4CDF7FFF            __L0138:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00CCC0  51CAFF18            L0139:DBF       D2,-232(PC)          L0130
00CCC4  4CDF7FFF            __L013A:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00CCC8  4E75                      RTS                           
00CCCA  DEEBF3F8FBFDFEFF    __L013B:
                                  DC.B      $DE,$EB,$F3,$F8,$FB,$FD,$FE,$FF 
00CCD2  01020305080D1522          DC.B      $01,$02,$03,$05,$08,$0D,$15,$22 
00CCDA  0839000000017080    __L013C:
                                  BTST      #0,$00017080         L066C
00CCE2  66000004                  BNE       4(PC)                __L013D
00CCE6  4E75                      RTS                           
00CCE8  48E7FFFE            __L013D:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00CCEC  2449                      MOVEA.L   A1,A2               
00CCEE  D5EA000E                  ADDA.L    14(A2),A2           
00CCF2  342A0004                  MOVE.W    4(A2),D2            
00CCF6  6700009A                  BEQ       154(PC)              __L0144
00CCFA  D5EA0000                  ADDA.L    0(A2),A2            
00CCFE  5342                      SUBQ.W    #1,D2               
00CD00  264A                L013E:MOVEA.L   A2,A3               
00CD02  D7DA                      ADDA.L    (A2)+,A3            
00CD04  0C130000                  CMPI.B    #0,(A3)             
00CD08  6700000E                  BEQ       14(PC)               L013F
00CD0C  0C130002                  CMPI.B    #2,(A3)             
00CD10  67000006                  BEQ       6(PC)                L013F
00CD14  60000078                  BRA       120(PC)              __L0143
00CD18  48E72000            L013F:MOVEM.L   D2,-(A7)            
00CD1C  3C2B0002                  MOVE.W    2(A3),D6            
00CD20  3E2B0004                  MOVE.W    4(A3),D7            
00CD24  5246                      ADDQ.W    #1,D6               
00CD26  E84E                      LSR.W     #4,D6               
00CD28  5346                      SUBQ.W    #1,D6               
00CD2A  4BEB0006                  LEA       6(A3),A5            
00CD2E  3C46                      MOVEA.W   D6,A6               
00CD30  3C0E                __L0140:
                                  MOVE.W    A6,D6               
00CD32  3A3C0003            __L0141:
                                  MOVE.W    #3,D5               
00CD36  381D                __L0142:
                                  MOVE.W    (A5)+,D4            
00CD38  E34C                      LSL.W     #1,D4               
00CD3A  E353                      ROXL.W    #1,D3               
00CD3C  E34C                      LSL.W     #1,D4               
00CD3E  E352                      ROXL.W    #1,D2               
00CD40  E34C                      LSL.W     #1,D4               
00CD42  E351                      ROXL.W    #1,D1               
00CD44  E34C                      LSL.W     #1,D4               
00CD46  E350                      ROXL.W    #1,D0               
00CD48  E34C                      LSL.W     #1,D4               
00CD4A  E353                      ROXL.W    #1,D3               
00CD4C  E34C                      LSL.W     #1,D4               
00CD4E  E352                      ROXL.W    #1,D2               
00CD50  E34C                      LSL.W     #1,D4               
00CD52  E351                      ROXL.W    #1,D1               
00CD54  E34C                      LSL.W     #1,D4               
00CD56  E350                      ROXL.W    #1,D0               
00CD58  E34C                      LSL.W     #1,D4               
00CD5A  E353                      ROXL.W    #1,D3               
00CD5C  E34C                      LSL.W     #1,D4               
00CD5E  E352                      ROXL.W    #1,D2               
00CD60  E34C                      LSL.W     #1,D4               
00CD62  E351                      ROXL.W    #1,D1               
00CD64  E34C                      LSL.W     #1,D4               
00CD66  E350                      ROXL.W    #1,D0               
00CD68  E34C                      LSL.W     #1,D4               
00CD6A  E353                      ROXL.W    #1,D3               
00CD6C  E34C                      LSL.W     #1,D4               
00CD6E  E352                      ROXL.W    #1,D2               
00CD70  E34C                      LSL.W     #1,D4               
00CD72  E351                      ROXL.W    #1,D1               
00CD74  E34C                      LSL.W     #1,D4               
00CD76  E350                      ROXL.W    #1,D0               
00CD78  51CDFFBC                  DBF       D5,-68(PC)           __L0142
00CD7C  48AD000FFFF8              MOVEM.W   D0-D3,-8(A5)        
00CD82  51CEFFAE                  DBF       D6,-82(PC)           __L0141
00CD86  51CFFFA8                  DBF       D7,-88(PC)           __L0140
00CD8A  4CDF0004                  MOVEM.L   (A7)+,D2            
00CD8E  51CAFF70            __L0143:
                                  DBF       D2,-144(PC)          L013E
00CD92  4CDF7FFF            __L0144:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00CD96  4E75                      RTS                           
00CD98  48E7FFFE            L0145:MOVEM.L   A0-A6/D0-D7,-(A7)   
00CD9C  2449                      MOVEA.L   A1,A2               
00CD9E  D5EA000E                  ADDA.L    14(A2),A2           
00CDA2  342A0004                  MOVE.W    4(A2),D2            
00CDA6  6700017A                  BEQ       378(PC)              __L015A
00CDAA  D5EA0000                  ADDA.L    0(A2),A2            
00CDAE  5342                      SUBQ.W    #1,D2               
00CDB0  2849                      MOVEA.L   A1,A4               
00CDB2  D9EC000E                  ADDA.L    14(A4),A4           
00CDB6  D9EC0012                  ADDA.L    18(A4),A4           
00CDBA  D9FC00000020              ADDA.L    #$20,A4             
00CDC0  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
00CDC8  67000026                  BEQ       38(PC)               L0147
00CDCC  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
00CDD4  6700001A                  BEQ       26(PC)               L0147
00CDD8  303C000F                  MOVE.W    #$F,D0              
00CDDC  224C                      MOVEA.L   A4,A1               
00CDDE  3211                __L0146:
                                  MOVE.W    (A1),D1             
00CDE0  E849                      LSR.W     #4,D1               
00CDE2  02410F0F                  ANDI.W    #$F0F,D1            
00CDE6  32C1                      MOVE.W    D1,(A1)+            
00CDE8  51C8FFF4                  DBF       D0,-12(PC)           __L0146
00CDEC  60000018                  BRA       24(PC)               __L0149
00CDF0  303C000F            L0147:MOVE.W    #$F,D0              
00CDF4  224C                      MOVEA.L   A4,A1               
00CDF6  3211                __L0148:
                                  MOVE.W    (A1),D1             
00CDF8  02410F0F                  ANDI.W    #$F0F,D1            
00CDFC  00410C0C                  ORI.W     #$C0C,D1            
00CE00  32C1                      MOVE.W    D1,(A1)+            
00CE02  51C8FFF2                  DBF       D0,-14(PC)           __L0148
00CE06  3054                __L0149:
                                  MOVEA.W   (A4),A0             
00CE08  264A                L014A:MOVEA.L   A2,A3               
00CE0A  D7DA                      ADDA.L    (A2)+,A3            
00CE0C  0C130000                  CMPI.B    #0,(A3)             
00CE10  67000016                  BEQ       22(PC)               __L014B
00CE14  0C130002                  CMPI.B    #2,(A3)             
00CE18  6700002C                  BEQ       44(PC)               __L014C
00CE1C  0C130001                  CMPI.B    #1,(A3)             
00CE20  67000106                  BEQ       262(PC)              __L015B
00CE24  600000F8                  BRA       248(PC)              L0159
00CE28  4254                __L014B:
                                  CLR.W     (A4)                
00CE2A  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
00CE32  66000026                  BNE       38(PC)               L014D
00CE36  08AB00040001              BCLR      #4,1(A3)            
00CE3C  08EB00050001              BSET      #5,1(A3)            
00CE42  60000016                  BRA       22(PC)               L014D
00CE46  3888                __L014C:
                                  MOVE.W    A0,(A4)             
00CE48  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
00CE50  66000008                  BNE       8(PC)                L014D
00CE54  08EB00040001              BSET      #4,1(A3)            
00CE5A  0C390001000170DE    L014D:CMPI.B    #1,$000170DE         L0690
00CE62  6700001A                  BEQ       26(PC)               L014E
00CE66  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
00CE6E  6700000E                  BEQ       14(PC)               L014E
00CE72  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
00CE7A  66000006                  BNE       6(PC)                __L014F
00CE7E  422B0001            L014E:CLR.B     1(A3)               
00CE82  48E7FF00            __L014F:
                                  MOVEM.L   D0-D7,-(A7)         
00CE86  3C2B0002                  MOVE.W    2(A3),D6            
00CE8A  3E2B0004                  MOVE.W    4(A3),D7            
00CE8E  5246                      ADDQ.W    #1,D6               
00CE90  E84E                      LSR.W     #4,D6               
00CE92  5346                      SUBQ.W    #1,D6               
00CE94  4BEB0006                  LEA       6(A3),A5            
00CE98  3C46                      MOVEA.W   D6,A6               
00CE9A  3C0E                __L0150:
                                  MOVE.W    A6,D6               
00CE9C  4C9D000F            __L0151:
                                  MOVEM.W   (A5)+,D0-D3         
00CEA0  383C000F                  MOVE.W    #$F,D4              
00CEA4  4245                L0152:CLR.W     D5                  
00CEA6  E24B                      LSR.W     #1,D3               
00CEA8  E355                      ROXL.W    #1,D5               
00CEAA  E24A                      LSR.W     #1,D2               
00CEAC  E355                      ROXL.W    #1,D5               
00CEAE  E249                      LSR.W     #1,D1               
00CEB0  E355                      ROXL.W    #1,D5               
00CEB2  E248                      LSR.W     #1,D0               
00CEB4  E355                      ROXL.W    #1,D5               
00CEB6  E345                      ASL.W     #1,D5               
00CEB8  67000036                  BEQ       54(PC)               L0156
00CEBC  08040000                  BTST      #0,D4               
00CEC0  67000006                  BEQ       6(PC)                __L0153
00CEC4  08450000                  BCHG      #0,D5               
00CEC8  08070000            __L0153:
                                  BTST      #0,D7               
00CECC  67000006                  BEQ       6(PC)                __L0154
00CED0  08450000                  BCHG      #0,D5               
00CED4  1A345000            __L0154:
                                  MOVE.B    0(A4,D5.W),D5       
00CED8  08050000                  BTST      #0,D5               
00CEDC  67000006                  BEQ       6(PC)                __L0155
00CEE0  08C0000F                  BSET      #15,D0              
00CEE4  08050001            __L0155:
                                  BTST      #1,D5               
00CEE8  67000006                  BEQ       6(PC)                L0156
00CEEC  08C1000F                  BSET      #15,D1              
00CEF0  08050002            L0156:BTST      #2,D5               
00CEF4  67000006                  BEQ       6(PC)                __L0157
00CEF8  08C2000F                  BSET      #15,D2              
00CEFC  08050003            __L0157:
                                  BTST      #3,D5               
00CF00  67000006                  BEQ       6(PC)                __L0158
00CF04  08C3000F                  BSET      #15,D3              
00CF08  51CCFF9A            __L0158:
                                  DBF       D4,-102(PC)          L0152
00CF0C  48AD000FFFF8              MOVEM.W   D0-D3,-8(A5)        
00CF12  51CEFF88                  DBF       D6,-120(PC)          __L0151
00CF16  51CFFF82                  DBF       D7,-126(PC)          __L0150
00CF1A  4CDF00FF                  MOVEM.L   (A7)+,D0-D7         
00CF1E  51CAFEE8            L0159:DBF       D2,-280(PC)          L014A
00CF22  4CDF7FFF            __L015A:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00CF26  4E75                      RTS                           
00CF28  4240                __L015B:
                                  CLR.W     D0                  
00CF2A  102B0001                  MOVE.B    1(A3),D0            
00CF2E  D040                      ADD.W     D0,D0               
00CF30  177400000001              MOVE.B    0(A4,D0.W),1(A3)    
00CF36  6000FFE6                  BRA       -26(PC)              L0159
00CF3A  4200                L015C:CLR.B     D0                  
00CF3C  6000FFFC                  BRA       -4(PC)               L015C
00CF40  00                  L015D:DC.B      $00 
00CF41  00                  L015E:DC.B      $00 
00CF42  4A390000AC76        L015F:TST.B     $0000AC76            L0015
00CF48  67000004                  BEQ       4(PC)                __L0160
00CF4C  4E75                      RTS                           
00CF4E  48E7FFFE            __L0160:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00CF52  26790000AC52              MOVEA.L   $0000AC52,A3         L000A
00CF58  383900017108              MOVE.W    $00017108,D4         L06A8
00CF5E  0244FFF0                  ANDI.W    #$FFF0,D4           
00CF62  3A390001710A              MOVE.W    $0001710A,D5         L06A9
00CF68  3C390001710C              MOVE.W    $0001710C,D6         L06AA
00CF6E  0046000F                  ORI.W     #$F,D6              
00CF72  3E390001710E              MOVE.W    $0001710E,D7         L06AB
00CF78  61000024                  BSR       36(PC)               L0162
00CF7C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00CF80  4E75                      RTS                           
00CF82  26790000AC4E        __L0161:
                                  MOVEA.L   $0000AC4E,A3         L0009
00CF88  383C0000                  MOVE.W    #0,D4               
00CF8C  3A3C0000                  MOVE.W    #0,D5               
00CF90  3C3C013F                  MOVE.W    #$13F,D6            
00CF94  3E3C00C7                  MOVE.W    #$C7,D7             
00CF98  61000004                  BSR       4(PC)                L0162
00CF9C  4E75                      RTS                           
00CF9E  4A390000AC76        L0162:TST.B     $0000AC76            L0015
00CFA4  66000192                  BNE       402(PC)              __L0170
00CFA8  3039000170D6              MOVE.W    $000170D6,D0         L068D
00CFAE  B046                      CMP.W     D6,D0               
00CFB0  6F000004                  BLE       4(PC)                __L0163
00CFB4  4E75                      RTS                           
00CFB6  3239000170D8        __L0163:
                                  MOVE.W    $000170D8,D1         L068E
00CFBC  B247                      CMP.W     D7,D1               
00CFBE  6F000004                  BLE       4(PC)                __L0164
00CFC2  4E75                      RTS                           
00CFC4  3400                __L0164:
                                  MOVE.W    D0,D2               
00CFC6  0642000F                  ADDI.W    #$F,D2              
00CFCA  B444                      CMP.W     D4,D2               
00CFCC  6C000004                  BGE       4(PC)                __L0165
00CFD0  4E75                      RTS                           
00CFD2  3601                __L0165:
                                  MOVE.W    D1,D3               
00CFD4  0643000F                  ADDI.W    #$F,D3              
00CFD8  B645                      CMP.W     D5,D3               
00CFDA  6C000004                  BGE       4(PC)                __L0166
00CFDE  4E75                      RTS                           
00CFE0  13FC00010000CF40    __L0166:
                                  MOVE.B    #1,$0000CF40         L015D
00CFE8  13FC00010000CF41          MOVE.B    #1,$0000CF41         L015E
00CFF0  B044                      CMP.W     D4,D0               
00CFF2  6C000008                  BGE       8(PC)                __L0167
00CFF6  42390000CF40              CLR.B     $0000CF40            L015D
00CFFC  B446                __L0167:
                                  CMP.W     D6,D2               
00CFFE  6F000008                  BLE       8(PC)                __L0168
00D002  42390000CF41              CLR.B     $0000CF41            L015E
00D008  2079000170DA        __L0168:
                                  MOVEA.L   $000170DA,A0         L068F
00D00E  41E80006                  LEA       6(A0),A0            
00D012  45F90000AE4E              LEA       $0000AE4E,A2         L0030
00D018  24F9000170D6              MOVE.L    $000170D6,(A2)+      L068D
00D01E  4240                      CLR.W     D0                  
00D020  343C000F                  MOVE.W    #$F,D2              
00D024  9A41                      SUB.W     D1,D5               
00D026  6F000006                  BLE       6(PC)                __L0169
00D02A  9445                      SUB.W     D5,D2               
00D02C  3005                      MOVE.W    D5,D0               
00D02E  9647                __L0169:
                                  SUB.W     D7,D3               
00D030  6F000004                  BLE       4(PC)                __L016A
00D034  9443                      SUB.W     D3,D2               
00D036  3E02                __L016A:
                                  MOVE.W    D2,D7               
00D038  3400                      MOVE.W    D0,D2               
00D03A  D079000170D8              ADD.W     $000170D8,D0         L068E
00D040  C0FC00A0                  MULU      #$A0,D0             
00D044  2240                      MOVEA.L   D0,A1               
00D046  E74A                      LSL.W     #3,D2               
00D048  D0C2                      ADDA.W    D2,A0               
00D04A  D4C2                      ADDA.W    D2,A2               
00D04C  D4C2                      ADDA.W    D2,A2               
00D04E  3039000170D6              MOVE.W    $000170D6,D0         L068D
00D054  3A00                      MOVE.W    D0,D5               
00D056  0245000F                  ANDI.W    #$F,D5              
00D05A  E848                      LSR.W     #4,D0               
00D05C  E748                      LSL.W     #3,D0               
00D05E  D2C0                      ADDA.W    D0,A1               
00D060  387C0000                  MOVEA.W   #0,A4               
00D064  4A390000CF40              TST.B     $0000CF40            L015D
00D06A  6600000E                  BNE       14(PC)               __L016B
00D06E  D2FC0008                  ADDA.W    #8,A1               
00D072  D4FC0008                  ADDA.W    #8,A2               
00D076  D8FC0008                  ADDA.W    #8,A4               
00D07A  4A390000CF41        __L016B:
                                  TST.B     $0000CF41            L015E
00D080  66000006                  BNE       6(PC)                __L016C
00D084  D8FC0008                  ADDA.W    #8,A4               
00D088  3A4C                __L016C:
                                  MOVEA.W   A4,A5               
00D08A  D8FC0090                  ADDA.W    #$90,A4             
00D08E  D3CB                      ADDA.L    A3,A1               
00D090  2018                __L016D:
                                  MOVE.L    (A0)+,D0            
00D092  2418                      MOVE.L    (A0)+,D2            
00D094  3200                      MOVE.W    D0,D1               
00D096  4841                      SWAP      D1                  
00D098  3602                      MOVE.W    D2,D3               
00D09A  4843                      SWAP      D3                  
00D09C  4240                      CLR.W     D0                  
00D09E  4241                      CLR.W     D1                  
00D0A0  4242                      CLR.W     D2                  
00D0A2  4243                      CLR.W     D3                  
00D0A4  EAA8                      LSR.L     D5,D0               
00D0A6  EAA9                      LSR.L     D5,D1               
00D0A8  EAAA                      LSR.L     D5,D2               
00D0AA  EAAB                      LSR.L     D5,D3               
00D0AC  2800                      MOVE.L    D0,D4               
00D0AE  8881                      OR.L      D1,D4               
00D0B0  8882                      OR.L      D2,D4               
00D0B2  8883                      OR.L      D3,D4               
00D0B4  4684                      NOT.L     D4                  
00D0B6  4840                      SWAP      D0                  
00D0B8  4841                      SWAP      D1                  
00D0BA  4842                      SWAP      D2                  
00D0BC  4843                      SWAP      D3                  
00D0BE  4844                      SWAP      D4                  
00D0C0  4A390000CF40              TST.B     $0000CF40            L015D
00D0C6  6700002A                  BEQ       42(PC)               __L016E
00D0CA  3C11                      MOVE.W    (A1),D6             
00D0CC  34C6                      MOVE.W    D6,(A2)+            
00D0CE  CC44                      AND.W     D4,D6               
00D0D0  8C40                      OR.W      D0,D6               
00D0D2  32C6                      MOVE.W    D6,(A1)+            
00D0D4  3C11                      MOVE.W    (A1),D6             
00D0D6  34C6                      MOVE.W    D6,(A2)+            
00D0D8  CC44                      AND.W     D4,D6               
00D0DA  8C41                      OR.W      D1,D6               
00D0DC  32C6                      MOVE.W    D6,(A1)+            
00D0DE  3C11                      MOVE.W    (A1),D6             
00D0E0  34C6                      MOVE.W    D6,(A2)+            
00D0E2  CC44                      AND.W     D4,D6               
00D0E4  8C42                      OR.W      D2,D6               
00D0E6  32C6                      MOVE.W    D6,(A1)+            
00D0E8  3C11                      MOVE.W    (A1),D6             
00D0EA  34C6                      MOVE.W    D6,(A2)+            
00D0EC  CC44                      AND.W     D4,D6               
00D0EE  8C43                      OR.W      D3,D6               
00D0F0  32C6                      MOVE.W    D6,(A1)+            
00D0F2  4840                __L016E:
                                  SWAP      D0                  
00D0F4  4841                      SWAP      D1                  
00D0F6  4842                      SWAP      D2                  
00D0F8  4843                      SWAP      D3                  
00D0FA  4844                      SWAP      D4                  
00D0FC  4A390000CF41              TST.B     $0000CF41            L015E
00D102  6700002A                  BEQ       42(PC)               __L016F
00D106  3C11                      MOVE.W    (A1),D6             
00D108  34C6                      MOVE.W    D6,(A2)+            
00D10A  CC44                      AND.W     D4,D6               
00D10C  8C40                      OR.W      D0,D6               
00D10E  32C6                      MOVE.W    D6,(A1)+            
00D110  3C11                      MOVE.W    (A1),D6             
00D112  34C6                      MOVE.W    D6,(A2)+            
00D114  CC44                      AND.W     D4,D6               
00D116  8C41                      OR.W      D1,D6               
00D118  32C6                      MOVE.W    D6,(A1)+            
00D11A  3C11                      MOVE.W    (A1),D6             
00D11C  34C6                      MOVE.W    D6,(A2)+            
00D11E  CC44                      AND.W     D4,D6               
00D120  8C42                      OR.W      D2,D6               
00D122  32C6                      MOVE.W    D6,(A1)+            
00D124  3C11                      MOVE.W    (A1),D6             
00D126  34C6                      MOVE.W    D6,(A2)+            
00D128  CC44                      AND.W     D4,D6               
00D12A  8C43                      OR.W      D3,D6               
00D12C  32C6                      MOVE.W    D6,(A1)+            
00D12E  D4CD                __L016F:
                                  ADDA.W    A5,A2               
00D130  D2CC                      ADDA.W    A4,A1               
00D132  51CFFF5C                  DBF       D7,-164(PC)          __L016D
00D136  4E75                      RTS                           
00D138  D844                __L0170:
                                  ADD.W     D4,D4               
00D13A  DA45                      ADD.W     D5,D5               
00D13C  DC46                      ADD.W     D6,D6               
00D13E  5246                      ADDQ.W    #1,D6               
00D140  DE47                      ADD.W     D7,D7               
00D142  5247                      ADDQ.W    #1,D7               
00D144  3039000170D6              MOVE.W    $000170D6,D0         L068D
00D14A  D040                      ADD.W     D0,D0               
00D14C  B046                      CMP.W     D6,D0               
00D14E  6F000004                  BLE       4(PC)                __L0171
00D152  4E75                      RTS                           
00D154  3239000170D8        __L0171:
                                  MOVE.W    $000170D8,D1         L068E
00D15A  D241                      ADD.W     D1,D1               
00D15C  B247                      CMP.W     D7,D1               
00D15E  6F000004                  BLE       4(PC)                __L0172
00D162  4E75                      RTS                           
00D164  3400                __L0172:
                                  MOVE.W    D0,D2               
00D166  0642000F                  ADDI.W    #$F,D2              
00D16A  B444                      CMP.W     D4,D2               
00D16C  6C000004                  BGE       4(PC)                __L0173
00D170  4E75                      RTS                           
00D172  3601                __L0173:
                                  MOVE.W    D1,D3               
00D174  0643000F                  ADDI.W    #$F,D3              
00D178  B645                      CMP.W     D5,D3               
00D17A  6C000004                  BGE       4(PC)                __L0174
00D17E  4E75                      RTS                           
00D180  13FC00010000CF40    __L0174:
                                  MOVE.B    #1,$0000CF40         L015D
00D188  13FC00010000CF41          MOVE.B    #1,$0000CF41         L015E
00D190  B044                      CMP.W     D4,D0               
00D192  6C000008                  BGE       8(PC)                __L0175
00D196  42390000CF40              CLR.B     $0000CF40            L015D
00D19C  B446                __L0175:
                                  CMP.W     D6,D2               
00D19E  6F000008                  BLE       8(PC)                __L0176
00D1A2  42390000CF41              CLR.B     $0000CF41            L015E
00D1A8  20790000D286        __L0176:
                                  MOVEA.L   $0000D286,A0         L017E
00D1AE  45F90000AE4E              LEA       $0000AE4E,A2         L0030
00D1B4  2039000170D6              MOVE.L    $000170D6,D0         L068D
00D1BA  D080                      ADD.L     D0,D0               
00D1BC  24C0                      MOVE.L    D0,(A2)+            
00D1BE  4240                      CLR.W     D0                  
00D1C0  343C000F                  MOVE.W    #$F,D2              
00D1C4  9A41                      SUB.W     D1,D5               
00D1C6  6F000006                  BLE       6(PC)                __L0177
00D1CA  9445                      SUB.W     D5,D2               
00D1CC  3005                      MOVE.W    D5,D0               
00D1CE  9647                __L0177:
                                  SUB.W     D7,D3               
00D1D0  6F000004                  BLE       4(PC)                __L0178
00D1D4  9443                      SUB.W     D3,D2               
00D1D6  3E02                __L0178:
                                  MOVE.W    D2,D7               
00D1D8  3400                      MOVE.W    D0,D2               
00D1DA  D079000170D8              ADD.W     $000170D8,D0         L068E
00D1E0  D079000170D8              ADD.W     $000170D8,D0         L068E
00D1E6  C0FC0050                  MULU      #$50,D0             
00D1EA  2240                      MOVEA.L   D0,A1               
00D1EC  E34A                      LSL.W     #1,D2               
00D1EE  D0C2                      ADDA.W    D2,A0               
00D1F0  D4C2                      ADDA.W    D2,A2               
00D1F2  D4C2                      ADDA.W    D2,A2               
00D1F4  3039000170D6              MOVE.W    $000170D6,D0         L068D
00D1FA  D040                      ADD.W     D0,D0               
00D1FC  3A00                      MOVE.W    D0,D5               
00D1FE  0245000F                  ANDI.W    #$F,D5              
00D202  E848                      LSR.W     #4,D0               
00D204  E348                      LSL.W     #1,D0               
00D206  D2C0                      ADDA.W    D0,A1               
00D208  387C0000                  MOVEA.W   #0,A4               
00D20C  4A390000CF40              TST.B     $0000CF40            L015D
00D212  6600000E                  BNE       14(PC)               __L0179
00D216  D2FC0002                  ADDA.W    #2,A1               
00D21A  D4FC0002                  ADDA.W    #2,A2               
00D21E  D8FC0002                  ADDA.W    #2,A4               
00D222  4A390000CF41        __L0179:
                                  TST.B     $0000CF41            L015E
00D228  66000006                  BNE       6(PC)                __L017A
00D22C  D8FC0002                  ADDA.W    #2,A4               
00D230  3A4C                __L017A:
                                  MOVEA.W   A4,A5               
00D232  D8FC004C                  ADDA.W    #$4C,A4             
00D236  D3CB                      ADDA.L    A3,A1               
00D238  32280020            __L017B:
                                  MOVE.W    32(A0),D1           
00D23C  3018                      MOVE.W    (A0)+,D0            
00D23E  4840                      SWAP      D0                  
00D240  4841                      SWAP      D1                  
00D242  4240                      CLR.W     D0                  
00D244  4241                      CLR.W     D1                  
00D246  EAA8                      LSR.L     D5,D0               
00D248  EAA9                      LSR.L     D5,D1               
00D24A  4681                      NOT.L     D1                  
00D24C  4840                      SWAP      D0                  
00D24E  4841                      SWAP      D1                  
00D250  4A390000CF40              TST.B     $0000CF40            L015D
00D256  6700000C                  BEQ       12(PC)               __L017C
00D25A  3C11                      MOVE.W    (A1),D6             
00D25C  34C6                      MOVE.W    D6,(A2)+            
00D25E  CC41                      AND.W     D1,D6               
00D260  8C40                      OR.W      D0,D6               
00D262  32C6                      MOVE.W    D6,(A1)+            
00D264  4840                __L017C:
                                  SWAP      D0                  
00D266  4841                      SWAP      D1                  
00D268  4A390000CF41              TST.B     $0000CF41            L015E
00D26E  6700000C                  BEQ       12(PC)               __L017D
00D272  3C11                      MOVE.W    (A1),D6             
00D274  34C6                      MOVE.W    D6,(A2)+            
00D276  CC41                      AND.W     D1,D6               
00D278  8C40                      OR.W      D0,D6               
00D27A  32C6                      MOVE.W    D6,(A1)+            
00D27C  D4CD                __L017D:
                                  ADDA.W    A5,A2               
00D27E  D2CC                      ADDA.W    A4,A1               
00D280  51CFFFB6                  DBF       D7,-74(PC)           __L017B
00D284  4E75                      RTS                           
00D286  0000D28A            L017E:DC.L      $0000D28A            L017F
00D28A  0000400060007000    L017F:DC.B      $00,$00,$40,$00,$60,$00,$70,$00 
00D292  78007C007E007F00          DC.B      $78,$00,$7C,$00,$7E,$00,$7F,$00 
00D29A  7F807C006C004600          DC.B      $7F,$80,$7C,$00,$6C,$00,$46,$00 
00D2A2  0600030003000000          DC.B      $06,$00,$03,$00,$03,$00,$00,$00 
00D2AA  C000E000F000F800          DC.B      $C0,$00,$E0,$00,$F0,$00,$F8,$00 
00D2B2  FC00FE00FF00FF80          DC.B      $FC,$00,$FE,$00,$FF,$00,$FF,$80 
00D2BA  FFC0FFE0FF00              DC.B      $FF,$C0,$FF,$E0,$FF,$00 
00D2C0  EF00                      ASL.B     #7,D0               
00D2C2  CF00                      ABCD      D0,D7               
00D2C4  0780                      BCLR      D3,D0               
00D2C6  0780                      BCLR      D3,D0               
00D2C8  03C0                      BSET      D1,D0               
00D2CA  4A390000AC76        L0180:TST.B     $0000AC76            L0015
00D2D0  66000076                  BNE       118(PC)              __L0187
00D2D4  22790000AC4E              MOVEA.L   $0000AC4E,A1         L0009
00D2DA  4A39000170D1              TST.B     $000170D1            L068A
00D2E0  67000008                  BEQ       8(PC)                __L0181
00D2E4  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00D2EA  41F90000AE4E        __L0181:
                                  LEA       $0000AE4E,A0         L0030
00D2F0  4240                      CLR.W     D0                  
00D2F2  3E3C000F                  MOVE.W    #$F,D7              
00D2F6  323C0090                  MOVE.W    #$90,D1             
00D2FA  4202                      CLR.B     D2                  
00D2FC  3618                      MOVE.W    (A0)+,D3            
00D2FE  6B000046                  BMI       70(PC)               __L0186
00D302  B67C0130                  CMP.W     #$130,D3            
00D306  6D00000E                  BLT       14(PC)               __L0182
00D30A  143C0001                  MOVE.B    #1,D2               
00D30E  06400008                  ADDI.W    #8,D0               
00D312  06410008                  ADDI.W    #8,D1               
00D316  E84B                __L0182:
                                  LSR.W     #4,D3               
00D318  E74B                      LSL.W     #3,D3               
00D31A  D2C3                      ADDA.W    D3,A1               
00D31C  363C00B8                  MOVE.W    #$B8,D3             
00D320  3818                      MOVE.W    (A0)+,D4            
00D322  9644                      SUB.W     D4,D3               
00D324  6A000004                  BPL       4(PC)                __L0183
00D328  DE43                      ADD.W     D3,D7               
00D32A  C8FC00A0            __L0183:
                                  MULU      #$A0,D4             
00D32E  D2C4                      ADDA.W    D4,A1               
00D330  4A02                __L0184:
                                  TST.B     D2                  
00D332  66000006                  BNE       6(PC)                __L0185
00D336  22D8                      MOVE.L    (A0)+,(A1)+         
00D338  22D8                      MOVE.L    (A0)+,(A1)+         
00D33A  22D8                __L0185:
                                  MOVE.L    (A0)+,(A1)+         
00D33C  22D8                      MOVE.L    (A0)+,(A1)+         
00D33E  D2C1                      ADDA.W    D1,A1               
00D340  D0C0                      ADDA.W    D0,A0               
00D342  51CFFFEC                  DBF       D7,-20(PC)           __L0184
00D346  4E75                __L0186:
                                  RTS                           
00D348  22790000AC4E        __L0187:
                                  MOVEA.L   $0000AC4E,A1         L0009
00D34E  4A39000170D1              TST.B     $000170D1            L068A
00D354  67000008                  BEQ       8(PC)                __L0188
00D358  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00D35E  41F90000AE4E        __L0188:
                                  LEA       $0000AE4E,A0         L0030
00D364  4240                      CLR.W     D0                  
00D366  3E3C000F                  MOVE.W    #$F,D7              
00D36A  323C004C                  MOVE.W    #$4C,D1             ;(#"L") 
00D36E  4202                      CLR.B     D2                  
00D370  3618                      MOVE.W    (A0)+,D3            
00D372  6B000042                  BMI       66(PC)               __L018D
00D376  B67C0270                  CMP.W     #$270,D3            
00D37A  6D00000E                  BLT       14(PC)               __L0189
00D37E  143C0001                  MOVE.B    #1,D2               
00D382  06400002                  ADDI.W    #2,D0               
00D386  06410002                  ADDI.W    #2,D1               
00D38A  E84B                __L0189:
                                  LSR.W     #4,D3               
00D38C  E34B                      LSL.W     #1,D3               
00D38E  D2C3                      ADDA.W    D3,A1               
00D390  363C0180                  MOVE.W    #$180,D3            
00D394  3818                      MOVE.W    (A0)+,D4            
00D396  9644                      SUB.W     D4,D3               
00D398  6A000004                  BPL       4(PC)                __L018A
00D39C  DE43                      ADD.W     D3,D7               
00D39E  C8FC0050            __L018A:
                                  MULU      #$50,D4             
00D3A2  D2C4                      ADDA.W    D4,A1               
00D3A4  4A02                __L018B:
                                  TST.B     D2                  
00D3A6  66000004                  BNE       4(PC)                __L018C
00D3AA  32D8                      MOVE.W    (A0)+,(A1)+         
00D3AC  32D8                __L018C:
                                  MOVE.W    (A0)+,(A1)+         
00D3AE  D2C1                      ADDA.W    D1,A1               
00D3B0  D0C0                      ADDA.W    D0,A0               
00D3B2  51CFFFF0                  DBF       D7,-16(PC)           __L018B
00D3B6  4E75                __L018D:
                                  RTS                           
00D3B8  0000                L018E:DC.B      $00,$00 
00D3BA  0000                L018F:DC.B      $00,$00 
00D3BC  0000                L0190:DC.B      $00,$00 
00D3BE  0000                L0191:DC.B      $00,$00 
00D3C0  24280008            L0192:MOVE.L    8(A0),D2            
00D3C4  028200FFFFFF              ANDI.L    #$FFFFFF,D2         
00D3CA  2442                      MOVEA.L   D2,A2               
00D3CC  D5D2                      ADDA.L    (A2),A2             
00D3CE  4A2A0000                  TST.B     0(A2)               
00D3D2  6A000004                  BPL       4(PC)                __L0193
00D3D6  4E75                      RTS                           
00D3D8  3028000C            __L0193:
                                  MOVE.W    12(A0),D0           
00D3DC  B07900017128              CMP.W     $00017128,D0         L06B7
00D3E2  6F000004                  BLE       4(PC)                __L0194
00D3E6  4E75                      RTS                           
00D3E8  3228000E            __L0194:
                                  MOVE.W    14(A0),D1           
00D3EC  B2790001712A              CMP.W     $0001712A,D1         L06B8
00D3F2  6F000004                  BLE       4(PC)                __L0195
00D3F6  4E75                      RTS                           
00D3F8  3400                __L0195:
                                  MOVE.W    D0,D2               
00D3FA  D46A0002                  ADD.W     2(A2),D2            
00D3FE  B47900017124              CMP.W     $00017124,D2         L06B5
00D404  6C000004                  BGE       4(PC)                __L0196
00D408  4E75                      RTS                           
00D40A  3601                __L0196:
                                  MOVE.W    D1,D3               
00D40C  D66A0004                  ADD.W     4(A2),D3            
00D410  B67900017126              CMP.W     $00017126,D3         L06B6
00D416  6C000004                  BGE       4(PC)                __L0197
00D41A  4E75                      RTS                           
00D41C  3800                __L0197:
                                  MOVE.W    D0,D4               
00D41E  B07900017124              CMP.W     $00017124,D0         L06B5
00D424  6C000008                  BGE       8(PC)                __L0198
00D428  383900017124              MOVE.W    $00017124,D4         L06B5
00D42E  3A01                __L0198:
                                  MOVE.W    D1,D5               
00D430  B27900017126              CMP.W     $00017126,D1         L06B6
00D436  6C000008                  BGE       8(PC)                __L0199
00D43A  3A3900017126              MOVE.W    $00017126,D5         L06B6
00D440  3C02                __L0199:
                                  MOVE.W    D2,D6               
00D442  B47900017128              CMP.W     $00017128,D2         L06B7
00D448  6F000008                  BLE       8(PC)                __L019A
00D44C  3C3900017128              MOVE.W    $00017128,D6         L06B7
00D452  3E03                __L019A:
                                  MOVE.W    D3,D7               
00D454  B6790001712A              CMP.W     $0001712A,D3         L06B8
00D45A  6F000008                  BLE       8(PC)                __L019B
00D45E  3E390001712A              MOVE.W    $0001712A,D7         L06B8
00D464  33C400017116        __L019B:
                                  MOVE.W    D4,$00017116         L06AE
00D46A  33C500017118              MOVE.W    D5,$00017118         L06AF
00D470  33C60001711A              MOVE.W    D6,$0001711A         L06B0
00D476  33C70001711C              MOVE.W    D7,$0001711C         L06B1
00D47C  9E45                      SUB.W     D5,D7               
00D47E  0244FFF0                  ANDI.W    #$FFF0,D4           
00D482  0046000F                  ORI.W     #$F,D6              
00D486  3A06                      MOVE.W    D6,D5               
00D488  9C44                      SUB.W     D4,D6               
00D48A  E84E                      LSR.W     #4,D6               
00D48C  3206                      MOVE.W    D6,D1               
00D48E  5241                      ADDQ.W    #1,D1               
00D490  E541                      ASL.W     #2,D1               
00D492  367900017230              MOVEA.W   $00017230,A3         L070F
00D498  96C1                      SUBA.W    D1,A3               
00D49A  D7CB                      ADDA.L    A3,A3               
00D49C  302A0002                  MOVE.W    2(A2),D0            
00D4A0  5240                      ADDQ.W    #1,D0               
00D4A2  E448                      LSR.W     #2,D0               
00D4A4  33C000017134              MOVE.W    D0,$00017134         L06BD
00D4AA  9041                      SUB.W     D1,D0               
00D4AC  D040                      ADD.W     D0,D0               
00D4AE  3C40                      MOVEA.W   D0,A6               
00D4B0  227900017224              MOVEA.L   $00017224,A1         L070B
00D4B6  303900017118              MOVE.W    $00017118,D0         L06AF
00D4BC  90790001722A              SUB.W     $0001722A,D0         L070D
00D4C2  C0F900017230              MULU      $00017230,D0         L070F
00D4C8  D080                      ADD.L     D0,D0               
00D4CA  D3C0                      ADDA.L    D0,A1               
00D4CC  303900017116              MOVE.W    $00017116,D0         L06AE
00D4D2  907900017228              SUB.W     $00017228,D0         L070C
00D4D8  0240FFF0                  ANDI.W    #$FFF0,D0           
00D4DC  E248                      LSR.W     #1,D0               
00D4DE  D2C0                      ADDA.W    D0,A1               
00D4E0  1012                      MOVE.B    (A2),D0             
00D4E2  B03C0001                  CMP.B     #1,D0               
00D4E6  6700068A                  BEQ       1674(PC)             __L01CF
00D4EA  49EA0006                  LEA       6(A2),A4            
00D4EE  303900017118              MOVE.W    $00017118,D0         L06AF
00D4F4  9068000E                  SUB.W     14(A0),D0           
00D4F8  343900017134              MOVE.W    $00017134,D2         L06BD
00D4FE  C0C2                      MULU      D2,D0               
00D500  D080                      ADD.L     D0,D0               
00D502  D9C0                      ADDA.L    D0,A4               
00D504  303900017116              MOVE.W    $00017116,D0         L06AE
00D50A  9068000C                  SUB.W     12(A0),D0           
00D50E  5340                      SUBQ.W    #1,D0               
00D510  E848                      LSR.W     #4,D0               
00D512  E740                      ASL.W     #3,D0               
00D514  082800000008              BTST      #0,8(A0)            
00D51A  66001088                  BNE       4232(PC)             __L021F
00D51E  3228000C                  MOVE.W    12(A0),D1           
00D522  0241000F                  ANDI.W    #$F,D1              
00D526  6600000E                  BNE       14(PC)               __L019C
00D52A  0C120002                  CMPI.B    #2,(A2)             
00D52E  6700098C                  BEQ       2444(PC)             __L01F3
00D532  60000810                  BRA       2064(PC)             __L01DE
00D536  9CFC0008            __L019C:
                                  SUBA.W    #8,A6               
00D53A  42B90001711E              CLR.L     $0001711E            L06B2
00D540  B868000C                  CMP.W     12(A0),D4           
00D544  6E000010                  BGT       16(PC)               __L019D
00D548  33FC00010001711E          MOVE.W    #1,$0001711E         L06B2
00D550  4240                      CLR.W     D0                  
00D552  DCFC0008                  ADDA.W    #8,A6               
00D556  D8C0                __L019D:
                                  ADDA.W    D0,A4               
00D558  302A0002                  MOVE.W    2(A2),D0            
00D55C  0040000F                  ORI.W     #$F,D0              
00D560  D068000C                  ADD.W     12(A0),D0           
00D564  BA40                      CMP.W     D0,D5               
00D566  6F00001E                  BLE       30(PC)               __L019F
00D56A  33FC000100017120          MOVE.W    #1,$00017120         L06B3
00D572  5346                      SUBQ.W    #1,D6               
00D574  6A00000C                  BPL       12(PC)               __L019E
00D578  5246                      ADDQ.W    #1,D6               
00D57A  33FCFFFF00017120          MOVE.W    #$FFFF,$00017120     L06B3
00D582  DCFC0008            __L019E:
                                  ADDA.W    #8,A6               
00D586  0C120002            __L019F:
                                  CMPI.B    #2,(A2)             
00D58A  6700010E                  BEQ       270(PC)              __L01A8
00D58E  082A00040001              BTST      #4,1(A2)            
00D594  66000430                  BNE       1072(PC)             __L01BF
00D598  082A00050001              BTST      #5,1(A2)            
00D59E  66000236                  BNE       566(PC)              __L01AF
00D5A2  3A28000C                  MOVE.W    12(A0),D5           
00D5A6  0245000F                  ANDI.W    #$F,D5              
00D5AA  3A45                      MOVEA.W   D5,A5               
00D5AC  344E                      MOVEA.W   A6,A2               
00D5AE  204C                      MOVEA.L   A4,A0               
00D5B0  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00D5B6  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00D5BC  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00D5C6  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00D5D0  3F3AFDE6                  MOVE.W    -538(PC),-(A7)       L018E
00D5D4  303AFDE4            __L01A0:
                                  MOVE.W    -540(PC),D0          L018F
00D5D8  387AFDE4                  MOVEA.W   -540(PC),A4          L0191
00D5DC  323AFDDE                  MOVE.W    -546(PC),D1          L0190
00D5E0  661A                      BNE.S     26(PC)               __L01A1
00D5E2  3818                      MOVE.W    (A0)+,D4            
00D5E4  3A18                      MOVE.W    (A0)+,D5            
00D5E6  3C18                      MOVE.W    (A0)+,D6            
00D5E8  3E18                      MOVE.W    (A0)+,D7            
00D5EA  4844                      SWAP      D4                  
00D5EC  4845                      SWAP      D5                  
00D5EE  4846                      SWAP      D6                  
00D5F0  4847                      SWAP      D7                  
00D5F2  320C                      MOVE.W    A4,D1               
00D5F4  6A0E                      BPL.S     14(PC)               L01A2
00D5F6  3C40                      MOVEA.W   D0,A6               
00D5F8  6000008C                  BRA       140(PC)              L01A7
00D5FC  7800                __L01A1:
                                  MOVEQ     #0,D4               
00D5FE  7A00                      MOVEQ     #0,D5               
00D600  7C00                      MOVEQ     #0,D6               
00D602  7E00                      MOVEQ     #0,D7               
00D604  3C40                L01A2:MOVEA.W   D0,A6               
00D606  3818                      MOVE.W    (A0)+,D4            
00D608  3A18                      MOVE.W    (A0)+,D5            
00D60A  3C18                      MOVE.W    (A0)+,D6            
00D60C  3E18                      MOVE.W    (A0)+,D7            
00D60E  2004                      MOVE.L    D4,D0               
00D610  2205                      MOVE.L    D5,D1               
00D612  2406                      MOVE.L    D6,D2               
00D614  2607                      MOVE.L    D7,D3               
00D616  4844                      SWAP      D4                  
00D618  4845                      SWAP      D5                  
00D61A  4846                      SWAP      D6                  
00D61C  4847                      SWAP      D7                  
00D61E  380D                __L01A3:
                                  MOVE.W    A5,D4               
00D620  E8A8                      LSR.L     D4,D0               
00D622  E8A9                      LSR.L     D4,D1               
00D624  E8AA                      LSR.L     D4,D2               
00D626  E8AB                      LSR.L     D4,D3               
00D628  3E00                      MOVE.W    D0,D7               
00D62A  8041                      OR.W      D1,D0               
00D62C  8042                      OR.W      D2,D0               
00D62E  8043                      OR.W      D3,D0               
00D630  674A                      BEQ.S     74(PC)               __L01A6
00D632  4640                      NOT.W     D0                  
00D634  6736                      BEQ.S     54(PC)               __L01A5
00D636  3819                      MOVE.W    (A1)+,D4            
00D638  C840                      AND.W     D0,D4               
00D63A  8847                      OR.W      D7,D4               
00D63C  3A19                      MOVE.W    (A1)+,D5            
00D63E  CA40                      AND.W     D0,D5               
00D640  8A41                      OR.W      D1,D5               
00D642  3C19                      MOVE.W    (A1)+,D6            
00D644  CC40                      AND.W     D0,D6               
00D646  8C42                      OR.W      D2,D6               
00D648  3E19                      MOVE.W    (A1)+,D7            
00D64A  CE40                      AND.W     D0,D7               
00D64C  8E43                      OR.W      D3,D7               
00D64E  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00D654  300E                      MOVE.W    A6,D0               
00D656  51C8FFAC                  DBF       D0,-84(PC)           L01A2
00D65A  320C                L01A4:MOVE.W    A4,D1               
00D65C  6628                      BNE.S     40(PC)               L01A7
00D65E  D0CA                      ADDA.W    A2,A0               
00D660  D2CB                      ADDA.W    A3,A1               
00D662  5357                      SUBQ.W    #1,(A7)             
00D664  6A00FF6E                  BPL       -146(PC)             __L01A0
00D668  544F                      ADDQ.W    #2,A7               
00D66A  4E75                      RTS                           
00D66C  32C7                __L01A5:
                                  MOVE.W    D7,(A1)+            
00D66E  32C1                      MOVE.W    D1,(A1)+            
00D670  32C2                      MOVE.W    D2,(A1)+            
00D672  32C3                      MOVE.W    D3,(A1)+            
00D674  300E                      MOVE.W    A6,D0               
00D676  51C8FF8C                  DBF       D0,-116(PC)          L01A2
00D67A  60DE                      BRA.S     -34(PC)              L01A4
00D67C  5049                __L01A6:
                                  ADDQ.W    #8,A1               
00D67E  300E                      MOVE.W    A6,D0               
00D680  51C8FF82                  DBF       D0,-126(PC)          L01A2
00D684  60D4                      BRA.S     -44(PC)              L01A4
00D686  4244                L01A7:CLR.W     D4                  
00D688  4245                      CLR.W     D5                  
00D68A  4246                      CLR.W     D6                  
00D68C  4247                      CLR.W     D7                  
00D68E  3844                      MOVEA.W   D4,A4               
00D690  2004                      MOVE.L    D4,D0               
00D692  2205                      MOVE.L    D5,D1               
00D694  2406                      MOVE.L    D6,D2               
00D696  2607                      MOVE.L    D7,D3               
00D698  6084                      BRA.S     -124(PC)             __L01A3
00D69A  082A00040001        __L01A8:
                                  BTST      #4,1(A2)            
00D6A0  660003EE                  BNE       1006(PC)             __L01C8
00D6A4  082A00050001              BTST      #5,1(A2)            
00D6AA  6600020E                  BNE       526(PC)              __L01B8
00D6AE  3A28000C                  MOVE.W    12(A0),D5           
00D6B2  0245000F                  ANDI.W    #$F,D5              
00D6B6  3A45                      MOVEA.W   D5,A5               
00D6B8  344E                      MOVEA.W   A6,A2               
00D6BA  204C                      MOVEA.L   A4,A0               
00D6BC  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00D6C2  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00D6C8  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00D6D2  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00D6DC  3F3AFCDA                  MOVE.W    -806(PC),-(A7)       L018E
00D6E0  387AFCDC                  MOVEA.W   -804(PC),A4          L0191
00D6E4  303AFCD4            __L01A9:
                                  MOVE.W    -812(PC),D0          L018F
00D6E8  323AFCD2                  MOVE.W    -814(PC),D1          L0190
00D6EC  6758                      BEQ.S     88(PC)               __L01AA
00D6EE  3C40                      MOVEA.W   D0,A6               
00D6F0  7800                      MOVEQ     #0,D4               
00D6F2  7A00                      MOVEQ     #0,D5               
00D6F4  7C00                      MOVEQ     #0,D6               
00D6F6  7E00                      MOVEQ     #0,D7               
00D6F8  3818                      MOVE.W    (A0)+,D4            
00D6FA  3A18                      MOVE.W    (A0)+,D5            
00D6FC  3C18                      MOVE.W    (A0)+,D6            
00D6FE  3E18                      MOVE.W    (A0)+,D7            
00D700  2004                      MOVE.L    D4,D0               
00D702  2205                      MOVE.L    D5,D1               
00D704  2406                      MOVE.L    D6,D2               
00D706  2607                      MOVE.L    D7,D3               
00D708  4844                      SWAP      D4                  
00D70A  4845                      SWAP      D5                  
00D70C  4846                      SWAP      D6                  
00D70E  4847                      SWAP      D7                  
00D710  380D                      MOVE.W    A5,D4               
00D712  E8A8                      LSR.L     D4,D0               
00D714  E8A9                      LSR.L     D4,D1               
00D716  E8AA                      LSR.L     D4,D2               
00D718  E8AB                      LSR.L     D4,D3               
00D71A  3E00                      MOVE.W    D0,D7               
00D71C  8041                      OR.W      D1,D0               
00D71E  8042                      OR.W      D2,D0               
00D720  8043                      OR.W      D3,D0               
00D722  4640                      NOT.W     D0                  
00D724  3819                      MOVE.W    (A1)+,D4            
00D726  C840                      AND.W     D0,D4               
00D728  8847                      OR.W      D7,D4               
00D72A  3A19                      MOVE.W    (A1)+,D5            
00D72C  CA40                      AND.W     D0,D5               
00D72E  8A41                      OR.W      D1,D5               
00D730  3C19                      MOVE.W    (A1)+,D6            
00D732  CC40                      AND.W     D0,D6               
00D734  8C42                      OR.W      D2,D6               
00D736  3E19                      MOVE.W    (A1)+,D7            
00D738  CE40                      AND.W     D0,D7               
00D73A  8E43                      OR.W      D3,D7               
00D73C  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00D742  60000040                  BRA       64(PC)               __L01AC
00D746  3818                __L01AA:
                                  MOVE.W    (A0)+,D4            
00D748  3A18                      MOVE.W    (A0)+,D5            
00D74A  3C18                      MOVE.W    (A0)+,D6            
00D74C  3E18                      MOVE.W    (A0)+,D7            
00D74E  4844                      SWAP      D4                  
00D750  4845                      SWAP      D5                  
00D752  4846                      SWAP      D6                  
00D754  4847                      SWAP      D7                  
00D756  320C                      MOVE.W    A4,D1               
00D758  6B34                      BMI.S     52(PC)               __L01AD
00D75A  3C40                __L01AB:
                                  MOVEA.W   D0,A6               
00D75C  3818                      MOVE.W    (A0)+,D4            
00D75E  3A18                      MOVE.W    (A0)+,D5            
00D760  3C18                      MOVE.W    (A0)+,D6            
00D762  3E18                      MOVE.W    (A0)+,D7            
00D764  2004                      MOVE.L    D4,D0               
00D766  2205                      MOVE.L    D5,D1               
00D768  2406                      MOVE.L    D6,D2               
00D76A  2607                      MOVE.L    D7,D3               
00D76C  4844                      SWAP      D4                  
00D76E  4845                      SWAP      D5                  
00D770  4846                      SWAP      D6                  
00D772  4847                      SWAP      D7                  
00D774  380D                      MOVE.W    A5,D4               
00D776  E8A8                      LSR.L     D4,D0               
00D778  E8A9                      LSR.L     D4,D1               
00D77A  E8AA                      LSR.L     D4,D2               
00D77C  E8AB                      LSR.L     D4,D3               
00D77E  4891000F                  MOVEM.W   D0-D3,(A1)          
00D782  5049                      ADDQ.W    #8,A1               
00D784  300E                __L01AC:
                                  MOVE.W    A6,D0               
00D786  51C8FFD2                  DBF       D0,-46(PC)           __L01AB
00D78A  320C                      MOVE.W    A4,D1               
00D78C  673A                      BEQ.S     58(PC)               __L01AE
00D78E  4244                __L01AD:
                                  CLR.W     D4                  
00D790  4245                      CLR.W     D5                  
00D792  4246                      CLR.W     D6                  
00D794  4247                      CLR.W     D7                  
00D796  300D                      MOVE.W    A5,D0               
00D798  E0AC                      LSR.L     D0,D4               
00D79A  E0AD                      LSR.L     D0,D5               
00D79C  E0AE                      LSR.L     D0,D6               
00D79E  E0AF                      LSR.L     D0,D7               
00D7A0  3604                      MOVE.W    D4,D3               
00D7A2  8845                      OR.W      D5,D4               
00D7A4  8846                      OR.W      D6,D4               
00D7A6  8847                      OR.W      D7,D4               
00D7A8  4644                      NOT.W     D4                  
00D7AA  3019                      MOVE.W    (A1)+,D0            
00D7AC  C044                      AND.W     D4,D0               
00D7AE  8043                      OR.W      D3,D0               
00D7B0  3219                      MOVE.W    (A1)+,D1            
00D7B2  C244                      AND.W     D4,D1               
00D7B4  8245                      OR.W      D5,D1               
00D7B6  3419                      MOVE.W    (A1)+,D2            
00D7B8  C444                      AND.W     D4,D2               
00D7BA  8446                      OR.W      D6,D2               
00D7BC  3619                      MOVE.W    (A1)+,D3            
00D7BE  C644                      AND.W     D4,D3               
00D7C0  8647                      OR.W      D7,D3               
00D7C2  48A9000FFFF8              MOVEM.W   D0-D3,-8(A1)        
00D7C8  D0CA                __L01AE:
                                  ADDA.W    A2,A0               
00D7CA  D2CB                      ADDA.W    A3,A1               
00D7CC  5357                      SUBQ.W    #1,(A7)             
00D7CE  6A00FF14                  BPL       -236(PC)             __L01A9
00D7D2  544F                      ADDQ.W    #2,A7               
00D7D4  4E75                      RTS                           
00D7D6  3A28000C            __L01AF:
                                  MOVE.W    12(A0),D5           
00D7DA  0245000F                  ANDI.W    #$F,D5              
00D7DE  3A45                      MOVEA.W   D5,A5               
00D7E0  344E                      MOVEA.W   A6,A2               
00D7E2  204C                      MOVEA.L   A4,A0               
00D7E4  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00D7EA  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00D7F0  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00D7FA  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00D804  3F3AFBB2                  MOVE.W    -1102(PC),-(A7)      L018E
00D808  363AFBB0            __L01B0:
                                  MOVE.W    -1104(PC),D3         L018F
00D80C  387AFBB0                  MOVEA.W   -1104(PC),A4         L0191
00D810  323AFBAA                  MOVE.W    -1110(PC),D1         L0190
00D814  6616                      BNE.S     22(PC)               __L01B1
00D816  3818                      MOVE.W    (A0)+,D4            
00D818  3A18                      MOVE.W    (A0)+,D5            
00D81A  3C18                      MOVE.W    (A0)+,D6            
00D81C  5448                      ADDQ.W    #2,A0               
00D81E  4844                      SWAP      D4                  
00D820  4845                      SWAP      D5                  
00D822  4846                      SWAP      D6                  
00D824  320C                      MOVE.W    A4,D1               
00D826  6A0A                      BPL.S     10(PC)               L01B2
00D828  6000007C                  BRA       124(PC)              L01B7
00D82C  7800                __L01B1:
                                  MOVEQ     #0,D4               
00D82E  7A00                      MOVEQ     #0,D5               
00D830  7C00                      MOVEQ     #0,D6               
00D832  3818                L01B2:MOVE.W    (A0)+,D4            
00D834  3A18                      MOVE.W    (A0)+,D5            
00D836  3C18                      MOVE.W    (A0)+,D6            
00D838  5448                      ADDQ.W    #2,A0               
00D83A  2004                      MOVE.L    D4,D0               
00D83C  2205                      MOVE.L    D5,D1               
00D83E  2406                      MOVE.L    D6,D2               
00D840  4844                      SWAP      D4                  
00D842  4845                      SWAP      D5                  
00D844  4846                      SWAP      D6                  
00D846  380D                __L01B3:
                                  MOVE.W    A5,D4               
00D848  E8A8                      LSR.L     D4,D0               
00D84A  E8A9                      LSR.L     D4,D1               
00D84C  E8AA                      LSR.L     D4,D2               
00D84E  3E00                      MOVE.W    D0,D7               
00D850  8041                      OR.W      D1,D0               
00D852  8042                      OR.W      D2,D0               
00D854  6746                      BEQ.S     70(PC)               __L01B6
00D856  4640                      NOT.W     D0                  
00D858  6732                      BEQ.S     50(PC)               __L01B5
00D85A  3819                      MOVE.W    (A1)+,D4            
00D85C  C840                      AND.W     D0,D4               
00D85E  8847                      OR.W      D7,D4               
00D860  3A19                      MOVE.W    (A1)+,D5            
00D862  CA40                      AND.W     D0,D5               
00D864  8A41                      OR.W      D1,D5               
00D866  3C19                      MOVE.W    (A1)+,D6            
00D868  CC40                      AND.W     D0,D6               
00D86A  8C42                      OR.W      D2,D6               
00D86C  3E19                      MOVE.W    (A1)+,D7            
00D86E  CE40                      AND.W     D0,D7               
00D870  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00D876  51CBFFBA                  DBF       D3,-70(PC)           L01B2
00D87A  320C                L01B4:MOVE.W    A4,D1               
00D87C  6628                      BNE.S     40(PC)               L01B7
00D87E  D0CA                      ADDA.W    A2,A0               
00D880  D2CB                      ADDA.W    A3,A1               
00D882  5357                      SUBQ.W    #1,(A7)             
00D884  6A00FF82                  BPL       -126(PC)             __L01B0
00D888  544F                      ADDQ.W    #2,A7               
00D88A  4E75                      RTS                           
00D88C  32C7                __L01B5:
                                  MOVE.W    D7,(A1)+            
00D88E  32C1                      MOVE.W    D1,(A1)+            
00D890  32C2                      MOVE.W    D2,(A1)+            
00D892  4259                      CLR.W     (A1)+               
00D894  51CBFF9C                  DBF       D3,-100(PC)          L01B2
00D898  6000FFE0                  BRA       -32(PC)              L01B4
00D89C  5049                __L01B6:
                                  ADDQ.W    #8,A1               
00D89E  51CBFF92                  DBF       D3,-110(PC)          L01B2
00D8A2  6000FFD6                  BRA       -42(PC)              L01B4
00D8A6  4244                L01B7:CLR.W     D4                  
00D8A8  4245                      CLR.W     D5                  
00D8AA  4246                      CLR.W     D6                  
00D8AC  4243                      CLR.W     D3                  
00D8AE  3844                      MOVEA.W   D4,A4               
00D8B0  2004                      MOVE.L    D4,D0               
00D8B2  2205                      MOVE.L    D5,D1               
00D8B4  2406                      MOVE.L    D6,D2               
00D8B6  6000FF8E                  BRA       -114(PC)             __L01B3
00D8BA  3A28000C            __L01B8:
                                  MOVE.W    12(A0),D5           
00D8BE  0245000F                  ANDI.W    #$F,D5              
00D8C2  3A45                      MOVEA.W   D5,A5               
00D8C4  344E                      MOVEA.W   A6,A2               
00D8C6  204C                      MOVEA.L   A4,A0               
00D8C8  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00D8CE  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00D8D4  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00D8DE  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00D8E8  3F3AFACE                  MOVE.W    -1330(PC),-(A7)      L018E
00D8EC  387AFAD0                  MOVEA.W   -1328(PC),A4         L0191
00D8F0  363AFAC8            __L01B9:
                                  MOVE.W    -1336(PC),D3         L018F
00D8F4  323AFAC6                  MOVE.W    -1338(PC),D1         L0190
00D8F8  674C                      BEQ.S     76(PC)               __L01BA
00D8FA  7800                      MOVEQ     #0,D4               
00D8FC  7A00                      MOVEQ     #0,D5               
00D8FE  7C00                      MOVEQ     #0,D6               
00D900  3818                      MOVE.W    (A0)+,D4            
00D902  3A18                      MOVE.W    (A0)+,D5            
00D904  3C18                      MOVE.W    (A0)+,D6            
00D906  5448                      ADDQ.W    #2,A0               
00D908  2004                      MOVE.L    D4,D0               
00D90A  2205                      MOVE.L    D5,D1               
00D90C  2406                      MOVE.L    D6,D2               
00D90E  4844                      SWAP      D4                  
00D910  4845                      SWAP      D5                  
00D912  4846                      SWAP      D6                  
00D914  380D                      MOVE.W    A5,D4               
00D916  E8A8                      LSR.L     D4,D0               
00D918  E8A9                      LSR.L     D4,D1               
00D91A  E8AA                      LSR.L     D4,D2               
00D91C  3E00                      MOVE.W    D0,D7               
00D91E  8041                      OR.W      D1,D0               
00D920  8042                      OR.W      D2,D0               
00D922  4640                      NOT.W     D0                  
00D924  3819                      MOVE.W    (A1)+,D4            
00D926  C840                      AND.W     D0,D4               
00D928  8847                      OR.W      D7,D4               
00D92A  3A19                      MOVE.W    (A1)+,D5            
00D92C  CA40                      AND.W     D0,D5               
00D92E  8A41                      OR.W      D1,D5               
00D930  3C19                      MOVE.W    (A1)+,D6            
00D932  CC40                      AND.W     D0,D6               
00D934  8C42                      OR.W      D2,D6               
00D936  3E19                      MOVE.W    (A1)+,D7            
00D938  CE40                      AND.W     D0,D7               
00D93A  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00D940  7E00                      MOVEQ     #0,D7               
00D942  6000003A                  BRA       58(PC)               __L01BC
00D946  7E00                __L01BA:
                                  MOVEQ     #0,D7               
00D948  3818                      MOVE.W    (A0)+,D4            
00D94A  3A18                      MOVE.W    (A0)+,D5            
00D94C  3C18                      MOVE.W    (A0)+,D6            
00D94E  5448                      ADDQ.W    #2,A0               
00D950  4844                      SWAP      D4                  
00D952  4845                      SWAP      D5                  
00D954  4846                      SWAP      D6                  
00D956  320C                      MOVE.W    A4,D1               
00D958  6B2C                      BMI.S     44(PC)               __L01BD
00D95A  3818                L01BB:MOVE.W    (A0)+,D4            
00D95C  3A18                      MOVE.W    (A0)+,D5            
00D95E  3C18                      MOVE.W    (A0)+,D6            
00D960  5448                      ADDQ.W    #2,A0               
00D962  2004                      MOVE.L    D4,D0               
00D964  2205                      MOVE.L    D5,D1               
00D966  2406                      MOVE.L    D6,D2               
00D968  4844                      SWAP      D4                  
00D96A  4845                      SWAP      D5                  
00D96C  4846                      SWAP      D6                  
00D96E  380D                      MOVE.W    A5,D4               
00D970  E8A8                      LSR.L     D4,D0               
00D972  E8A9                      LSR.L     D4,D1               
00D974  E8AA                      LSR.L     D4,D2               
00D976  32C0                      MOVE.W    D0,(A1)+            
00D978  32C1                      MOVE.W    D1,(A1)+            
00D97A  32C2                      MOVE.W    D2,(A1)+            
00D97C  32C7                      MOVE.W    D7,(A1)+            
00D97E  51CBFFDA            __L01BC:
                                  DBF       D3,-38(PC)           L01BB
00D982  320C                      MOVE.W    A4,D1               
00D984  6732                      BEQ.S     50(PC)               __L01BE
00D986  4244                __L01BD:
                                  CLR.W     D4                  
00D988  4245                      CLR.W     D5                  
00D98A  4246                      CLR.W     D6                  
00D98C  300D                      MOVE.W    A5,D0               
00D98E  E0AC                      LSR.L     D0,D4               
00D990  E0AD                      LSR.L     D0,D5               
00D992  E0AE                      LSR.L     D0,D6               
00D994  3404                      MOVE.W    D4,D2               
00D996  8845                      OR.W      D5,D4               
00D998  8846                      OR.W      D6,D4               
00D99A  4644                      NOT.W     D4                  
00D99C  3019                      MOVE.W    (A1)+,D0            
00D99E  C044                      AND.W     D4,D0               
00D9A0  8042                      OR.W      D2,D0               
00D9A2  3219                      MOVE.W    (A1)+,D1            
00D9A4  C244                      AND.W     D4,D1               
00D9A6  8245                      OR.W      D5,D1               
00D9A8  3419                      MOVE.W    (A1)+,D2            
00D9AA  C444                      AND.W     D4,D2               
00D9AC  8446                      OR.W      D6,D2               
00D9AE  3A19                      MOVE.W    (A1)+,D5            
00D9B0  CA44                      AND.W     D4,D5               
00D9B2  48A90027FFF8              MOVEM.W   D5/D0-D2,-8(A1)     
00D9B8  D0CA                __L01BE:
                                  ADDA.W    A2,A0               
00D9BA  D2CB                      ADDA.W    A3,A1               
00D9BC  5357                      SUBQ.W    #1,(A7)             
00D9BE  6A00FF30                  BPL       -208(PC)             __L01B9
00D9C2  544F                      ADDQ.W    #2,A7               
00D9C4  4E75                      RTS                           
00D9C6  3428000C            __L01BF:
                                  MOVE.W    12(A0),D2           
00D9CA  0242000F                  ANDI.W    #$F,D2              
00D9CE  3A45                      MOVEA.W   D5,A5               
00D9D0  344E                      MOVEA.W   A6,A2               
00D9D2  204C                      MOVEA.L   A4,A0               
00D9D4  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00D9DA  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00D9E0  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00D9EA  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00D9F4  3F3AF9C2                  MOVE.W    -1598(PC),-(A7)      L018E
00D9F8  363AF9C0            __L01C0:
                                  MOVE.W    -1600(PC),D3         L018F
00D9FC  387AF9C0                  MOVEA.W   -1600(PC),A4         L0191
00DA00  323AF9BA                  MOVE.W    -1606(PC),D1         L0190
00DA04  6612                      BNE.S     18(PC)               __L01C1
00DA06  3818                      MOVE.W    (A0)+,D4            
00DA08  3A18                      MOVE.W    (A0)+,D5            
00DA0A  5848                      ADDQ.W    #4,A0               
00DA0C  4844                      SWAP      D4                  
00DA0E  4845                      SWAP      D5                  
00DA10  320C                      MOVE.W    A4,D1               
00DA12  6A08                      BPL.S     8(PC)                L01C2
00DA14  6000006A                  BRA       106(PC)              L01C7
00DA18  7800                __L01C1:
                                  MOVEQ     #0,D4               
00DA1A  7A00                      MOVEQ     #0,D5               
00DA1C  3818                L01C2:MOVE.W    (A0)+,D4            
00DA1E  3A18                      MOVE.W    (A0)+,D5            
00DA20  5848                      ADDQ.W    #4,A0               
00DA22  2004                      MOVE.L    D4,D0               
00DA24  2205                      MOVE.L    D5,D1               
00DA26  4844                      SWAP      D4                  
00DA28  4845                      SWAP      D5                  
00DA2A  E4A8                __L01C3:
                                  LSR.L     D2,D0               
00DA2C  E4A9                      LSR.L     D2,D1               
00DA2E  3E00                      MOVE.W    D0,D7               
00DA30  8041                      OR.W      D1,D0               
00DA32  6742                      BEQ.S     66(PC)               __L01C6
00DA34  4640                      NOT.W     D0                  
00DA36  6730                      BEQ.S     48(PC)               __L01C5
00DA38  3819                      MOVE.W    (A1)+,D4            
00DA3A  C840                      AND.W     D0,D4               
00DA3C  8847                      OR.W      D7,D4               
00DA3E  3A19                      MOVE.W    (A1)+,D5            
00DA40  CA40                      AND.W     D0,D5               
00DA42  8A41                      OR.W      D1,D5               
00DA44  3C19                      MOVE.W    (A1)+,D6            
00DA46  CC40                      AND.W     D0,D6               
00DA48  3E19                      MOVE.W    (A1)+,D7            
00DA4A  CE40                      AND.W     D0,D7               
00DA4C  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00DA52  51CBFFC8                  DBF       D3,-56(PC)           L01C2
00DA56  320C                L01C4:MOVE.W    A4,D1               
00DA58  6626                      BNE.S     38(PC)               L01C7
00DA5A  D0CA                      ADDA.W    A2,A0               
00DA5C  D2CB                      ADDA.W    A3,A1               
00DA5E  5357                      SUBQ.W    #1,(A7)             
00DA60  6A00FF96                  BPL       -106(PC)             __L01C0
00DA64  544F                      ADDQ.W    #2,A7               
00DA66  4E75                      RTS                           
00DA68  32C7                __L01C5:
                                  MOVE.W    D7,(A1)+            
00DA6A  32C1                      MOVE.W    D1,(A1)+            
00DA6C  4299                      CLR.L     (A1)+               
00DA6E  51CBFFAC                  DBF       D3,-84(PC)           L01C2
00DA72  6000FFE2                  BRA       -30(PC)              L01C4
00DA76  5049                __L01C6:
                                  ADDQ.W    #8,A1               
00DA78  51CBFFA2                  DBF       D3,-94(PC)           L01C2
00DA7C  6000FFD8                  BRA       -40(PC)              L01C4
00DA80  4244                L01C7:CLR.W     D4                  
00DA82  4245                      CLR.W     D5                  
00DA84  4243                      CLR.W     D3                  
00DA86  3844                      MOVEA.W   D4,A4               
00DA88  2004                      MOVE.L    D4,D0               
00DA8A  2205                      MOVE.L    D5,D1               
00DA8C  6000FF9C                  BRA       -100(PC)             __L01C3
00DA90  3428000C            __L01C8:
                                  MOVE.W    12(A0),D2           
00DA94  0242000F                  ANDI.W    #$F,D2              
00DA98  3A45                      MOVEA.W   D5,A5               
00DA9A  344E                      MOVEA.W   A6,A2               
00DA9C  204C                      MOVEA.L   A4,A0               
00DA9E  33C70000D3B8              MOVE.W    D7,$0000D3B8         L018E
00DAA4  33C60000D3BA              MOVE.W    D6,$0000D3BA         L018F
00DAAA  33F9000171200000D3BE      MOVE.W    $00017120,$0000D3BE  L06B3 L0191
00DAB4  33F90001711E0000D3BC      MOVE.W    $0001711E,$0000D3BC  L06B2 L0190
00DABE  3F3AF8F8                  MOVE.W    -1800(PC),-(A7)      L018E
00DAC2  387AF8FA                  MOVEA.W   -1798(PC),A4         L0191
00DAC6  363AF8F2            __L01C9:
                                  MOVE.W    -1806(PC),D3         L018F
00DACA  323AF8F0                  MOVE.W    -1808(PC),D1         L0190
00DACE  673C                      BEQ.S     60(PC)               __L01CA
00DAD0  7800                      MOVEQ     #0,D4               
00DAD2  7A00                      MOVEQ     #0,D5               
00DAD4  3818                      MOVE.W    (A0)+,D4            
00DAD6  3A18                      MOVE.W    (A0)+,D5            
00DAD8  5848                      ADDQ.W    #4,A0               
00DADA  2004                      MOVE.L    D4,D0               
00DADC  2205                      MOVE.L    D5,D1               
00DADE  4844                      SWAP      D4                  
00DAE0  4845                      SWAP      D5                  
00DAE2  E4A8                      LSR.L     D2,D0               
00DAE4  E4A9                      LSR.L     D2,D1               
00DAE6  3E00                      MOVE.W    D0,D7               
00DAE8  8041                      OR.W      D1,D0               
00DAEA  4640                      NOT.W     D0                  
00DAEC  3819                      MOVE.W    (A1)+,D4            
00DAEE  C840                      AND.W     D0,D4               
00DAF0  8847                      OR.W      D7,D4               
00DAF2  3A19                      MOVE.W    (A1)+,D5            
00DAF4  CA40                      AND.W     D0,D5               
00DAF6  8A41                      OR.W      D1,D5               
00DAF8  3C19                      MOVE.W    (A1)+,D6            
00DAFA  CC40                      AND.W     D0,D6               
00DAFC  3E19                      MOVE.W    (A1)+,D7            
00DAFE  CE40                      AND.W     D0,D7               
00DB00  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00DB06  7E00                      MOVEQ     #0,D7               
00DB08  6000002A                  BRA       42(PC)               __L01CC
00DB0C  7E00                __L01CA:
                                  MOVEQ     #0,D7               
00DB0E  3818                      MOVE.W    (A0)+,D4            
00DB10  3A18                      MOVE.W    (A0)+,D5            
00DB12  5848                      ADDQ.W    #4,A0               
00DB14  4844                      SWAP      D4                  
00DB16  4845                      SWAP      D5                  
00DB18  320C                      MOVE.W    A4,D1               
00DB1A  6B20                      BMI.S     32(PC)               __L01CD
00DB1C  3818                L01CB:MOVE.W    (A0)+,D4            
00DB1E  3A18                      MOVE.W    (A0)+,D5            
00DB20  5848                      ADDQ.W    #4,A0               
00DB22  2004                      MOVE.L    D4,D0               
00DB24  2205                      MOVE.L    D5,D1               
00DB26  4844                      SWAP      D4                  
00DB28  4845                      SWAP      D5                  
00DB2A  E4A8                      LSR.L     D2,D0               
00DB2C  E4A9                      LSR.L     D2,D1               
00DB2E  32C0                      MOVE.W    D0,(A1)+            
00DB30  32C1                      MOVE.W    D1,(A1)+            
00DB32  22C7                      MOVE.L    D7,(A1)+            
00DB34  51CBFFE6            __L01CC:
                                  DBF       D3,-26(PC)           L01CB
00DB38  320C                      MOVE.W    A4,D1               
00DB3A  6728                      BEQ.S     40(PC)               __L01CE
00DB3C  4244                __L01CD:
                                  CLR.W     D4                  
00DB3E  4245                      CLR.W     D5                  
00DB40  E4AC                      LSR.L     D2,D4               
00DB42  E4AD                      LSR.L     D2,D5               
00DB44  3C04                      MOVE.W    D4,D6               
00DB46  8845                      OR.W      D5,D4               
00DB48  4644                      NOT.W     D4                  
00DB4A  3019                      MOVE.W    (A1)+,D0            
00DB4C  C044                      AND.W     D4,D0               
00DB4E  8046                      OR.W      D6,D0               
00DB50  3219                      MOVE.W    (A1)+,D1            
00DB52  C244                      AND.W     D4,D1               
00DB54  8245                      OR.W      D5,D1               
00DB56  3A19                      MOVE.W    (A1)+,D5            
00DB58  CA44                      AND.W     D4,D5               
00DB5A  3C19                      MOVE.W    (A1)+,D6            
00DB5C  CC44                      AND.W     D4,D6               
00DB5E  48A90063FFF8              MOVEM.W   D5-D6/D0-D1,-8(A1)  
00DB64  D0CA                __L01CE:
                                  ADDA.W    A2,A0               
00DB66  D2CB                      ADDA.W    A3,A1               
00DB68  5357                      SUBQ.W    #1,(A7)             
00DB6A  6A00FF5A                  BPL       -166(PC)             __L01C9
00DB6E  544F                      ADDQ.W    #2,A7               
00DB70  4E75                      RTS                           
00DB72  42790001711E        __L01CF:
                                  CLR.W     $0001711E            L06B2
00DB78  427900017120              CLR.W     $00017120            L06B3
00DB7E  B868000C                  CMP.W     12(A0),D4           
00DB82  6E00000A                  BGT       10(PC)               __L01D0
00DB86  33FC00010001711E          MOVE.W    #1,$0001711E         L06B2
00DB8E  302A0002            __L01D0:
                                  MOVE.W    2(A2),D0            
00DB92  D068000C                  ADD.W     12(A0),D0           
00DB96  BA40                      CMP.W     D0,D5               
00DB98  6F00001A                  BLE       26(PC)               L01D1
00DB9C  33FC000100017120          MOVE.W    #1,$00017120         L06B3
00DBA4  5346                      SUBQ.W    #1,D6               
00DBA6  6A00000C                  BPL       12(PC)               L01D1
00DBAA  5246                      ADDQ.W    #1,D6               
00DBAC  33FCFFFF00017120          MOVE.W    #$FFFF,$00017120     L06B3
00DBB4  4846                L01D1:SWAP      D6                  
00DBB6  4847                      SWAP      D7                  
00DBB8  4244                      CLR.W     D4                  
00DBBA  4245                      CLR.W     D5                  
00DBBC  4246                      CLR.W     D6                  
00DBBE  4247                      CLR.W     D7                  
00DBC0  082A00000001              BTST      #0,1(A2)            
00DBC6  67000004                  BEQ       4(PC)                __L01D2
00DBCA  4644                      NOT.W     D4                  
00DBCC  082A00010001        __L01D2:
                                  BTST      #1,1(A2)            
00DBD2  67000004                  BEQ       4(PC)                __L01D3
00DBD6  4645                      NOT.W     D5                  
00DBD8  082A00020001        __L01D3:
                                  BTST      #2,1(A2)            
00DBDE  67000004                  BEQ       4(PC)                __L01D4
00DBE2  4646                      NOT.W     D6                  
00DBE4  082A00030001        __L01D4:
                                  BTST      #3,1(A2)            
00DBEA  67000004                  BEQ       4(PC)                __L01D5
00DBEE  4647                      NOT.W     D7                  
00DBF0  49F90000DD34        __L01D5:
                                  LEA       $0000DD34,A4         L01DD
00DBF6  489400F0                  MOVEM.W   D4-D7,(A4)          
00DBFA  48AC00F00008              MOVEM.W   D4-D7,8(A4)         
00DC00  4844                      SWAP      D4                  
00DC02  4845                      SWAP      D5                  
00DC04  4846                      SWAP      D6                  
00DC06  4847                      SWAP      D7                  
00DC08  3028000C                  MOVE.W    12(A0),D0           
00DC0C  0240000F                  ANDI.W    #$F,D0              
00DC10  223C00010000              MOVE.L    #$10000,D1          
00DC16  E0A9                      LSR.L     D0,D1               
00DC18  5341                      SUBQ.W    #1,D1               
00DC1A  3801                      MOVE.W    D1,D4               
00DC1C  C954                      AND.W     D4,(A4)             
00DC1E  C96C0002                  AND.W     D4,2(A4)            
00DC22  C96C0004                  AND.W     D4,4(A4)            
00DC26  C96C0006                  AND.W     D4,6(A4)            
00DC2A  3028000C                  MOVE.W    12(A0),D0           
00DC2E  D06A0002                  ADD.W     2(A2),D0            
00DC32  0240000F                  ANDI.W    #$F,D0              
00DC36  223C00008000              MOVE.L    #$8000,D1           
00DC3C  E0A9                      LSR.L     D0,D1               
00DC3E  5341                      SUBQ.W    #1,D1               
00DC40  4641                      NOT.W     D1                  
00DC42  3A01                      MOVE.W    D1,D5               
00DC44  CB6C0008                  AND.W     D5,8(A4)            
00DC48  CB6C000A                  AND.W     D5,10(A4)           
00DC4C  CB6C000C                  AND.W     D5,12(A4)           
00DC50  CB6C000E                  AND.W     D5,14(A4)           
00DC54  4A790001711E              TST.W     $0001711E            L06B2
00DC5A  67000024                  BEQ       36(PC)               L01D6
00DC5E  4A7900017120              TST.W     $00017120            L06B3
00DC64  6A00001A                  BPL       26(PC)               L01D6
00DC68  42790001711E              CLR.W     $0001711E            L06B2
00DC6E  CA44                      AND.W     D4,D5               
00DC70  C96C0008                  AND.W     D4,8(A4)            
00DC74  C96C000A                  AND.W     D4,10(A4)           
00DC78  C96C000C                  AND.W     D4,12(A4)           
00DC7C  C96C000E                  AND.W     D4,14(A4)           
00DC80  4644                L01D6:NOT.W     D4                  
00DC82  4645                      NOT.W     D5                  
00DC84  3846                      MOVEA.W   D6,A4               
00DC86  41F90000DD34              LEA       $0000DD34,A0         __L01DD
00DC8C  3A790001711E              MOVEA.W   $0001711E,A5         L06B2
00DC92  3C7900017120              MOVEA.W   $00017120,A6         L06B3
00DC98  4844                      SWAP      D4                  
00DC9A  4845                      SWAP      D5                  
00DC9C  4846                __L01D7:
                                  SWAP      D6                  
00DC9E  4847                      SWAP      D7                  
00DCA0  300D                      MOVE.W    A5,D0               
00DCA2  6700003A                  BEQ       58(PC)               __L01D8
00DCA6  4844                      SWAP      D4                  
00DCA8  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00DCAC  C044                      AND.W     D4,D0               
00DCAE  C244                      AND.W     D4,D1               
00DCB0  C444                      AND.W     D4,D2               
00DCB2  C644                      AND.W     D4,D3               
00DCB4  4844                      SWAP      D4                  
00DCB6  8050                      OR.W      (A0),D0             
00DCB8  82680002                  OR.W      2(A0),D1            
00DCBC  84680004                  OR.W      4(A0),D2            
00DCC0  86680006                  OR.W      6(A0),D3            
00DCC4  48A9000FFFF8              MOVEM.W   D0-D3,-8(A1)        
00DCCA  300E                      MOVE.W    A6,D0               
00DCCC  6B000032                  BMI       50(PC)               L01DB
00DCD0  3004                      MOVE.W    D4,D0               
00DCD2  3205                      MOVE.W    D5,D1               
00DCD4  3406                      MOVE.W    D6,D2               
00DCD6  3607                      MOVE.W    D7,D3               
00DCD8  4846                      SWAP      D6                  
00DCDA  60000018                  BRA       24(PC)               __L01DA
00DCDE  300E                __L01D8:
                                  MOVE.W    A6,D0               
00DCE0  6B00001E                  BMI       30(PC)               L01DB
00DCE4  3004                      MOVE.W    D4,D0               
00DCE6  3205                      MOVE.W    D5,D1               
00DCE8  3406                      MOVE.W    D6,D2               
00DCEA  3607                      MOVE.W    D7,D3               
00DCEC  4846                      SWAP      D6                  
00DCEE  4891000F            L01D9:MOVEM.W   D0-D3,(A1)          
00DCF2  5089                      ADDQ.L    #8,A1               
00DCF4  51CEFFF8            __L01DA:
                                  DBF       D6,-8(PC)            L01D9
00DCF8  4846                      SWAP      D6                  
00DCFA  300E                      MOVE.W    A6,D0               
00DCFC  67000028                  BEQ       40(PC)               __L01DC
00DD00  4845                L01DB:SWAP      D5                  
00DD02  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00DD06  C045                      AND.W     D5,D0               
00DD08  C245                      AND.W     D5,D1               
00DD0A  C445                      AND.W     D5,D2               
00DD0C  C645                      AND.W     D5,D3               
00DD0E  4845                      SWAP      D5                  
00DD10  80680008                  OR.W      8(A0),D0            
00DD14  8268000A                  OR.W      10(A0),D1           
00DD18  8468000C                  OR.W      12(A0),D2           
00DD1C  8668000E                  OR.W      14(A0),D3           
00DD20  48A9000FFFF8              MOVEM.W   D0-D3,-8(A1)        
00DD26  D3CB                __L01DC:
                                  ADDA.L    A3,A1               
00DD28  4846                      SWAP      D6                  
00DD2A  4847                      SWAP      D7                  
00DD2C  3C0C                      MOVE.W    A4,D6               
00DD2E  51CFFF6C                  DBF       D7,-148(PC)          __L01D7
00DD32  4E75                      RTS                           
00DD34                      __L01DD:
                                  DCB.W     8,0 
00DD44  082A00040001        __L01DE:
                                  BTST      #4,1(A2)            
00DD4A  66000106                  BNE       262(PC)              __L01EC
00DD4E  082A00050001              BTST      #5,1(A2)            
00DD54  66000084                  BNE       132(PC)              __L01E5
00DD58  06400008                  ADDI.W    #8,D0               
00DD5C  B868000C                  CMP.W     12(A0),D4           
00DD60  6E000004                  BGT       4(PC)                __L01DF
00DD64  4240                      CLR.W     D0                  
00DD66  D8C0                __L01DF:
                                  ADDA.W    D0,A4               
00DD68  344E                      MOVEA.W   A6,A2               
00DD6A  204C                      MOVEA.L   A4,A0               
00DD6C  3846                      MOVEA.W   D6,A4               
00DD6E  4847                __L01E0:
                                  SWAP      D7                  
00DD70  4846                L01E1:SWAP      D6                  
00DD72  3818                      MOVE.W    (A0)+,D4            
00DD74  3A18                      MOVE.W    (A0)+,D5            
00DD76  3C18                      MOVE.W    (A0)+,D6            
00DD78  3E18                      MOVE.W    (A0)+,D7            
00DD7A  3004                      MOVE.W    D4,D0               
00DD7C  8045                      OR.W      D5,D0               
00DD7E  8046                      OR.W      D6,D0               
00DD80  8047                      OR.W      D7,D0               
00DD82  674A                      BEQ.S     74(PC)               __L01E4
00DD84  4640                      NOT.W     D0                  
00DD86  6734                      BEQ.S     52(PC)               __L01E3
00DD88  4844                      SWAP      D4                  
00DD8A  3800                      MOVE.W    D0,D4               
00DD8C  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00DD90  C044                      AND.W     D4,D0               
00DD92  C244                      AND.W     D4,D1               
00DD94  C444                      AND.W     D4,D2               
00DD96  C644                      AND.W     D4,D3               
00DD98  4844                      SWAP      D4                  
00DD9A  8840                      OR.W      D0,D4               
00DD9C  8A41                      OR.W      D1,D5               
00DD9E  8C42                      OR.W      D2,D6               
00DDA0  8E43                      OR.W      D3,D7               
00DDA2  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00DDA8  4846                      SWAP      D6                  
00DDAA  51CEFFC4                  DBF       D6,-60(PC)           L01E1
00DDAE  D0CA                L01E2:ADDA.W    A2,A0               
00DDB0  D2CB                      ADDA.W    A3,A1               
00DDB2  3C0C                      MOVE.W    A4,D6               
00DDB4  4847                      SWAP      D7                  
00DDB6  51CFFFB6                  DBF       D7,-74(PC)           __L01E0
00DDBA  4E75                      RTS                           
00DDBC  32C4                __L01E3:
                                  MOVE.W    D4,(A1)+            
00DDBE  32C5                      MOVE.W    D5,(A1)+            
00DDC0  32C6                      MOVE.W    D6,(A1)+            
00DDC2  32C7                      MOVE.W    D7,(A1)+            
00DDC4  4846                      SWAP      D6                  
00DDC6  51CEFFA8                  DBF       D6,-88(PC)           L01E1
00DDCA  6000FFE2                  BRA       -30(PC)              L01E2
00DDCE  5049                __L01E4:
                                  ADDQ.W    #8,A1               
00DDD0  4846                      SWAP      D6                  
00DDD2  51CEFF9C                  DBF       D6,-100(PC)          L01E1
00DDD6  6000FFD6                  BRA       -42(PC)              L01E2
00DDDA  06400008            __L01E5:
                                  ADDI.W    #8,D0               
00DDDE  B868000C                  CMP.W     12(A0),D4           
00DDE2  6E000004                  BGT       4(PC)                __L01E6
00DDE6  4240                      CLR.W     D0                  
00DDE8  D8C0                __L01E6:
                                  ADDA.W    D0,A4               
00DDEA  344E                      MOVEA.W   A6,A2               
00DDEC  204C                      MOVEA.L   A4,A0               
00DDEE  3846                      MOVEA.W   D6,A4               
00DDF0  4847                __L01E7:
                                  SWAP      D7                  
00DDF2  4846                L01E8:SWAP      D6                  
00DDF4  3818                      MOVE.W    (A0)+,D4            
00DDF6  3A18                      MOVE.W    (A0)+,D5            
00DDF8  3C18                      MOVE.W    (A0)+,D6            
00DDFA  5448                      ADDQ.W    #2,A0               
00DDFC  3E04                      MOVE.W    D4,D7               
00DDFE  8E45                      OR.W      D5,D7               
00DE00  8E46                      OR.W      D6,D7               
00DE02  6742                      BEQ.S     66(PC)               __L01EB
00DE04  4647                      NOT.W     D7                  
00DE06  672C                      BEQ.S     44(PC)               __L01EA
00DE08  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00DE0C  C047                      AND.W     D7,D0               
00DE0E  C247                      AND.W     D7,D1               
00DE10  C447                      AND.W     D7,D2               
00DE12  C647                      AND.W     D7,D3               
00DE14  8044                      OR.W      D4,D0               
00DE16  8245                      OR.W      D5,D1               
00DE18  8446                      OR.W      D6,D2               
00DE1A  48A9000FFFF8              MOVEM.W   D0-D3,-8(A1)        
00DE20  4846                      SWAP      D6                  
00DE22  51CEFFCE                  DBF       D6,-50(PC)           L01E8
00DE26  D0CA                L01E9:ADDA.W    A2,A0               
00DE28  D2CB                      ADDA.W    A3,A1               
00DE2A  3C0C                      MOVE.W    A4,D6               
00DE2C  4847                      SWAP      D7                  
00DE2E  51CFFFC0                  DBF       D7,-64(PC)           __L01E7
00DE32  4E75                      RTS                           
00DE34  32C4                __L01EA:
                                  MOVE.W    D4,(A1)+            
00DE36  32C5                      MOVE.W    D5,(A1)+            
00DE38  32C6                      MOVE.W    D6,(A1)+            
00DE3A  32C7                      MOVE.W    D7,(A1)+            
00DE3C  4846                      SWAP      D6                  
00DE3E  51CEFFB2                  DBF       D6,-78(PC)           L01E8
00DE42  6000FFE2                  BRA       -30(PC)              L01E9
00DE46  5049                __L01EB:
                                  ADDQ.W    #8,A1               
00DE48  4846                      SWAP      D6                  
00DE4A  51CEFFA6                  DBF       D6,-90(PC)           L01E8
00DE4E  6000FFD6                  BRA       -42(PC)              L01E9
00DE52  06400008            __L01EC:
                                  ADDI.W    #8,D0               
00DE56  B868000C                  CMP.W     12(A0),D4           
00DE5A  6E000004                  BGT       4(PC)                __L01ED
00DE5E  4240                      CLR.W     D0                  
00DE60  D8C0                __L01ED:
                                  ADDA.W    D0,A4               
00DE62  344E                      MOVEA.W   A6,A2               
00DE64  204C                      MOVEA.L   A4,A0               
00DE66  3846                      MOVEA.W   D6,A4               
00DE68  4847                __L01EE:
                                  SWAP      D7                  
00DE6A  3818                L01EF:MOVE.W    (A0)+,D4            
00DE6C  3A18                      MOVE.W    (A0)+,D5            
00DE6E  5848                      ADDQ.W    #4,A0               
00DE70  3E04                      MOVE.W    D4,D7               
00DE72  8E45                      OR.W      D5,D7               
00DE74  673C                      BEQ.S     60(PC)               __L01F2
00DE76  4647                      NOT.W     D7                  
00DE78  6728                      BEQ.S     40(PC)               __L01F1
00DE7A  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00DE7E  C047                      AND.W     D7,D0               
00DE80  C247                      AND.W     D7,D1               
00DE82  C447                      AND.W     D7,D2               
00DE84  C647                      AND.W     D7,D3               
00DE86  8044                      OR.W      D4,D0               
00DE88  8245                      OR.W      D5,D1               
00DE8A  48A9000FFFF8              MOVEM.W   D0-D3,-8(A1)        
00DE90  51CEFFD8                  DBF       D6,-40(PC)           L01EF
00DE94  D0CA                L01F0:ADDA.W    A2,A0               
00DE96  D2CB                      ADDA.W    A3,A1               
00DE98  3C0C                      MOVE.W    A4,D6               
00DE9A  4847                      SWAP      D7                  
00DE9C  51CFFFCA                  DBF       D7,-54(PC)           __L01EE
00DEA0  4E75                      RTS                           
00DEA2  32C4                __L01F1:
                                  MOVE.W    D4,(A1)+            
00DEA4  32C5                      MOVE.W    D5,(A1)+            
00DEA6  32C7                      MOVE.W    D7,(A1)+            
00DEA8  32C7                      MOVE.W    D7,(A1)+            
00DEAA  51CEFFBE                  DBF       D6,-66(PC)           L01EF
00DEAE  6000FFE4                  BRA       -28(PC)              L01F0
00DEB2  5049                __L01F2:
                                  ADDQ.W    #8,A1               
00DEB4  51CEFFB4                  DBF       D6,-76(PC)           L01EF
00DEB8  6000FFDA                  BRA       -38(PC)              L01F0
00DEBC  06400008            __L01F3:
                                  ADDI.W    #8,D0               
00DEC0  B868000C                  CMP.W     12(A0),D4           
00DEC4  6E000004                  BGT       4(PC)                __L01F4
00DEC8  4240                      CLR.W     D0                  
00DECA  D8C0                __L01F4:
                                  ADDA.W    D0,A4               
00DECC  344E                      MOVEA.W   A6,A2               
00DECE  204C                      MOVEA.L   A4,A0               
00DED0  60000002                  BRA       2(PC)                L01F5
00DED4  BC7C000B            L01F5:CMP.W     #$B,D6              
00DED8  6D000032                  BLT       50(PC)               __L01F7
00DEDC  0446000B                  SUBI.W    #$B,D6              
00DEE0  E546                      ASL.W     #2,D6               
00DEE2  2C7B6004                  MOVEA.L   4(PC,D6.W),A6        __L01F6
00DEE6  4ED6                      JMP       (A6)                
00DEE8  0000DF22            __L01F6:
                                  DC.L      $0000DF22            L01F9
00DEEC  0000DF50                  DC.L      $0000DF50            L01FA
00DEF0  0000DF7E                  DC.L      $0000DF7E            L01FB
00DEF4  0000DFAC                  DC.L      $0000DFAC            L01FC
00DEF8  0000DFDA                  DC.L      $0000DFDA            L01FD
00DEFC  0000E00C                  DC.L      $0000E00C            L01FE
00DF00  0000E046                  DC.L      $0000E046            L01FF
00DF04  0000E080                  DC.L      $0000E080            L0200
00DF08  0000E0BA                  DC.L      $0000E0BA            L0201
00DF0C  3846                __L01F7:
                                  MOVEA.W   D6,A4               
00DF0E  22D8                L01F8:MOVE.L    (A0)+,(A1)+         
00DF10  22D8                      MOVE.L    (A0)+,(A1)+         
00DF12  51CEFFFA                  DBF       D6,-6(PC)            L01F8
00DF16  D0CA                      ADDA.W    A2,A0               
00DF18  D2CB                      ADDA.W    A3,A1               
00DF1A  3C0C                      MOVE.W    A4,D6               
00DF1C  51CFFFF0                  DBF       D7,-16(PC)           L01F8
00DF20  4E75                      RTS                           
00DF22  4CD8707F            __L01F9:
                                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF26  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF2A  D2FC0028                  ADDA.W    #$28,A1             
00DF2E  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF32  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF36  43E90028                  LEA       40(A1),A1           
00DF3A  4CD8000F                  MOVEM.L   (A0)+,D0-D3         
00DF3E  48D1000F                  MOVEM.L   D0-D3,(A1)          
00DF42  D2FC0010                  ADDA.W    #$10,A1             
00DF46  D0CA                      ADDA.W    A2,A0               
00DF48  D2CB                      ADDA.W    A3,A1               
00DF4A  51CFFFD6                  DBF       D7,-42(PC)           __L01F9
00DF4E  4E75                      RTS                           
00DF50  4CD8707F            L01FA:MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF54  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF58  43E90028                  LEA       40(A1),A1           
00DF5C  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF60  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF64  43E90028                  LEA       40(A1),A1           
00DF68  4CD8003F                  MOVEM.L   (A0)+,D0-D5         
00DF6C  48D1003F                  MOVEM.L   D0-D5,(A1)          
00DF70  43E90018                  LEA       24(A1),A1           
00DF74  D0CA                      ADDA.W    A2,A0               
00DF76  D2CB                      ADDA.W    A3,A1               
00DF78  51CFFFD6                  DBF       D7,-42(PC)           L01FA
00DF7C  4E75                      RTS                           
00DF7E  4CD8707F            L01FB:MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF82  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF86  43E90028                  LEA       40(A1),A1           
00DF8A  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DF8E  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DF92  43E90028                  LEA       40(A1),A1           
00DF96  4CD8107F                  MOVEM.L   (A0)+,A4/D0-D6      
00DF9A  48D1107F                  MOVEM.L   A4/D0-D6,(A1)       
00DF9E  43E90020                  LEA       32(A1),A1           
00DFA2  D0CA                      ADDA.W    A2,A0               
00DFA4  D2CB                      ADDA.W    A3,A1               
00DFA6  51CFFFD6                  DBF       D7,-42(PC)           L01FB
00DFAA  4E75                      RTS                           
00DFAC  4CD8707F            L01FC:MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFB0  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFB4  43E90028                  LEA       40(A1),A1           
00DFB8  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFBC  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFC0  43E90028                  LEA       40(A1),A1           
00DFC4  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFC8  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFCC  43E90028                  LEA       40(A1),A1           
00DFD0  D0CA                      ADDA.W    A2,A0               
00DFD2  D2CB                      ADDA.W    A3,A1               
00DFD4  51CFFFD6                  DBF       D7,-42(PC)           L01FC
00DFD8  4E75                      RTS                           
00DFDA  4CD8707F            L01FD:MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFDE  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFE2  43E90028                  LEA       40(A1),A1           
00DFE6  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFEA  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFEE  43E90028                  LEA       40(A1),A1           
00DFF2  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00DFF6  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00DFFA  43E90028                  LEA       40(A1),A1           
00DFFE  22D8                      MOVE.L    (A0)+,(A1)+         
00E000  22D8                      MOVE.L    (A0)+,(A1)+         
00E002  D0CA                      ADDA.W    A2,A0               
00E004  D2CB                      ADDA.W    A3,A1               
00E006  51CFFFD2                  DBF       D7,-46(PC)           L01FD
00E00A  4E75                      RTS                           
00E00C  4CD8707F            L01FE:MOVEM.L   (A0)+,A4-A6/D0-D6   
00E010  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E014  43E90028                  LEA       40(A1),A1           
00E018  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E01C  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E020  43E90028                  LEA       40(A1),A1           
00E024  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E028  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E02C  43E90028                  LEA       40(A1),A1           
00E030  4CD8000F                  MOVEM.L   (A0)+,D0-D3         
00E034  48D1000F                  MOVEM.L   D0-D3,(A1)          
00E038  D2FC0010                  ADDA.W    #$10,A1             
00E03C  D0CA                      ADDA.W    A2,A0               
00E03E  D2CB                      ADDA.W    A3,A1               
00E040  51CFFFCA                  DBF       D7,-54(PC)           L01FE
00E044  4E75                      RTS                           
00E046  4CD8707F            L01FF:MOVEM.L   (A0)+,A4-A6/D0-D6   
00E04A  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E04E  43E90028                  LEA       40(A1),A1           
00E052  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E056  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E05A  43E90028                  LEA       40(A1),A1           
00E05E  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E062  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E066  43E90028                  LEA       40(A1),A1           
00E06A  4CD8003F                  MOVEM.L   (A0)+,D0-D5         
00E06E  48D1003F                  MOVEM.L   D0-D5,(A1)          
00E072  43E90018                  LEA       24(A1),A1           
00E076  D0CA                      ADDA.W    A2,A0               
00E078  D2CB                      ADDA.W    A3,A1               
00E07A  51CFFFCA                  DBF       D7,-54(PC)           L01FF
00E07E  4E75                      RTS                           
00E080  4CD8707F            L0200:MOVEM.L   (A0)+,A4-A6/D0-D6   
00E084  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E088  43E90028                  LEA       40(A1),A1           
00E08C  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E090  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E094  43E90028                  LEA       40(A1),A1           
00E098  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E09C  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E0A0  43E90028                  LEA       40(A1),A1           
00E0A4  4CD8107F                  MOVEM.L   (A0)+,A4/D0-D6      
00E0A8  48D1107F                  MOVEM.L   A4/D0-D6,(A1)       
00E0AC  43E90020                  LEA       32(A1),A1           
00E0B0  D0CA                      ADDA.W    A2,A0               
00E0B2  D2CB                      ADDA.W    A3,A1               
00E0B4  51CFFFCA                  DBF       D7,-54(PC)           L0200
00E0B8  4E75                      RTS                           
00E0BA  4CD8707F            L0201:MOVEM.L   (A0)+,A4-A6/D0-D6   
00E0BE  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E0C2  43E90028                  LEA       40(A1),A1           
00E0C6  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E0CA  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E0CE  43E90028                  LEA       40(A1),A1           
00E0D2  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E0D6  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E0DA  43E90028                  LEA       40(A1),A1           
00E0DE  4CD8707F                  MOVEM.L   (A0)+,A4-A6/D0-D6   
00E0E2  48D1707F                  MOVEM.L   A4-A6/D0-D6,(A1)    
00E0E6  43E90028                  LEA       40(A1),A1           
00E0EA  D0CA                      ADDA.W    A2,A0               
00E0EC  D2CB                      ADDA.W    A3,A1               
00E0EE  51CFFFCA                  DBF       D7,-54(PC)           L0201
00E0F2  4E75                      RTS                           
00E0F4  48E70080            __L0202:
                                  MOVEM.L   A0,-(A7)            
00E0F8  20790000AC4E              MOVEA.L   $0000AC4E,A0         L0009
00E0FE  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00E104  6100002C                  BSR       44(PC)               L0204
00E108  4CDF0100                  MOVEM.L   (A7)+,A0            
00E10C  4E75                      RTS                           
00E10E  4A390000AC76        L0203:TST.B     $0000AC76            L0015
00E114  6600006C                  BNE       108(PC)              __L0205
00E118  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00E11C  20790000AC52              MOVEA.L   $0000AC52,A0         L000A
00E122  22790000AC4E              MOVEA.L   $0000AC4E,A1         L0009
00E128  61000008                  BSR       8(PC)                L0204
00E12C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E130  4E75                      RTS                           
00E132  3E390001710E        L0204:MOVE.W    $0001710E,D7         L06AB
00E138  9E790001710A              SUB.W     $0001710A,D7         L06A9
00E13E  3C3900017110              MOVE.W    $00017110,D6         L06AC
00E144  E44E                      LSR.W     #2,D6               
00E146  5346                      SUBQ.W    #1,D6               
00E148  30390001710A              MOVE.W    $0001710A,D0         L06A9
00E14E  C0FC00A0                  MULU      #$A0,D0             
00E152  323900017108              MOVE.W    $00017108,D1         L06A8
00E158  0241FFF0                  ANDI.W    #$FFF0,D1           
00E15C  E249                      LSR.W     #1,D1               
00E15E  D041                      ADD.W     D1,D0               
00E160  D0C0                      ADDA.W    D0,A0               
00E162  D2C0                      ADDA.W    D0,A1               
00E164  383900017110              MOVE.W    $00017110,D4         L06AC
00E16A  D844                      ADD.W     D4,D4               
00E16C  4444                      NEG.W     D4                  
00E16E  48C4                      EXT.L     D4                  
00E170  0684000000A0              ADDI.L    #$A0,D4             
00E176  3444                      MOVEA.W   D4,A2               
00E178  3644                      MOVEA.W   D4,A3               
00E17A  4EB90000DED4              JSR       $0000DED4            L01F5
00E180  4E75                      RTS                           
00E182  0C3900010000AC76    __L0205:
                                  CMPI.B    #1,$0000AC76         L0015
00E18A  670000D4                  BEQ       212(PC)              __L0208
00E18E  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00E192  3E390001710E              MOVE.W    $0001710E,D7         L06AB
00E198  9E790001710A              SUB.W     $0001710A,D7         L06A9
00E19E  3C3900017110              MOVE.W    $00017110,D6         L06AC
00E1A4  E44E                      LSR.W     #2,D6               
00E1A6  5346                      SUBQ.W    #1,D6               
00E1A8  3C46                      MOVEA.W   D6,A6               
00E1AA  30390001710A              MOVE.W    $0001710A,D0         L06A9
00E1B0  C0FC00A0                  MULU      #$A0,D0             
00E1B4  323900017108              MOVE.W    $00017108,D1         L06A8
00E1BA  0241FFF0                  ANDI.W    #$FFF0,D1           
00E1BE  E249                      LSR.W     #1,D1               
00E1C0  D041                      ADD.W     D1,D0               
00E1C2  20790000AC52              MOVEA.L   $0000AC52,A0         L000A
00E1C8  D0C0                      ADDA.W    D0,A0               
00E1CA  3A3900017110              MOVE.W    $00017110,D5         L06AC
00E1D0  DA45                      ADD.W     D5,D5               
00E1D2  4445                      NEG.W     D5                  
00E1D4  48C5                      EXT.L     D5                  
00E1D6  0685000000A0              ADDI.L    #$A0,D5             
00E1DC  383900017110              MOVE.W    $00017110,D4         L06AC
00E1E2  4444                      NEG.W     D4                  
00E1E4  48C4                      EXT.L     D4                  
00E1E6  0684000000A0              ADDI.L    #$A0,D4             
00E1EC  30390001710A              MOVE.W    $0001710A,D0         L06A9
00E1F2  C0FC00A0                  MULU      #$A0,D0             
00E1F6  323900017108              MOVE.W    $00017108,D1         L06A8
00E1FC  E449                      LSR.W     #2,D1               
00E1FE  22790000AC4E              MOVEA.L   $0000AC4E,A1         L0009
00E204  D3C0                      ADDA.L    D0,A1               
00E206  D2C1                      ADDA.W    D1,A1               
00E208  2449                      MOVEA.L   A1,A2               
00E20A  D5FC00000050              ADDA.L    #$50,A2             
00E210  26790000AC78              MOVEA.L   $0000AC78,A3         L0017
00E216  3C0E                __L0206:
                                  MOVE.W    A6,D6               
00E218  4240                __L0207:
                                  CLR.W     D0                  
00E21A  4241                      CLR.W     D1                  
00E21C  4242                      CLR.W     D2                  
00E21E  4243                      CLR.W     D3                  
00E220  1018                      MOVE.B    (A0)+,D0            
00E222  1218                      MOVE.B    (A0)+,D1            
00E224  1418                      MOVE.B    (A0)+,D2            
00E226  1618                      MOVE.B    (A0)+,D3            
00E228  5848                      ADDQ.W    #4,A0               
00E22A  D040                      ADD.W     D0,D0               
00E22C  D241                      ADD.W     D1,D1               
00E22E  D442                      ADD.W     D2,D2               
00E230  D643                      ADD.W     D3,D3               
00E232  30330000                  MOVE.W    0(A3,D0.W),D0       
00E236  D040                      ADD.W     D0,D0               
00E238  80732000                  OR.W      0(A3,D2.W),D0       
00E23C  4840                      SWAP      D0                  
00E23E  30331000                  MOVE.W    0(A3,D1.W),D0       
00E242  D040                      ADD.W     D0,D0               
00E244  80733000                  OR.W      0(A3,D3.W),D0       
00E248  22C0                      MOVE.L    D0,(A1)+            
00E24A  24C0                      MOVE.L    D0,(A2)+            
00E24C  51CEFFCA                  DBF       D6,-54(PC)           __L0207
00E250  D2C4                      ADDA.W    D4,A1               
00E252  D4C4                      ADDA.W    D4,A2               
00E254  D0C5                      ADDA.W    D5,A0               
00E256  51CFFFBE                  DBF       D7,-66(PC)           __L0206
00E25A  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E25E  4E75                      RTS                           
00E260  48E7FFFE            __L0208:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00E264  3E390001710E              MOVE.W    $0001710E,D7         L06AB
00E26A  9E790001710A              SUB.W     $0001710A,D7         L06A9
00E270  3C3900017110              MOVE.W    $00017110,D6         L06AC
00E276  E44E                      LSR.W     #2,D6               
00E278  5346                      SUBQ.W    #1,D6               
00E27A  3C46                      MOVEA.W   D6,A6               
00E27C  30390001710A              MOVE.W    $0001710A,D0         L06A9
00E282  C0FC00A0                  MULU      #$A0,D0             
00E286  323900017108              MOVE.W    $00017108,D1         L06A8
00E28C  0241FFF0                  ANDI.W    #$FFF0,D1           
00E290  E249                      LSR.W     #1,D1               
00E292  D041                      ADD.W     D1,D0               
00E294  20790000AC52              MOVEA.L   $0000AC52,A0         L000A
00E29A  D0C0                      ADDA.W    D0,A0               
00E29C  3A3900017110              MOVE.W    $00017110,D5         L06AC
00E2A2  DA45                      ADD.W     D5,D5               
00E2A4  4445                      NEG.W     D5                  
00E2A6  48C5                      EXT.L     D5                  
00E2A8  0685000000A0              ADDI.L    #$A0,D5             
00E2AE  383900017110              MOVE.W    $00017110,D4         L06AC
00E2B4  D844                      ADD.W     D4,D4               
00E2B6  4444                      NEG.W     D4                  
00E2B8  48C4                      EXT.L     D4                  
00E2BA  0684000000A0              ADDI.L    #$A0,D4             
00E2C0  30390001710A              MOVE.W    $0001710A,D0         L06A9
00E2C6  C0FC00A0                  MULU      #$A0,D0             
00E2CA  323900017108              MOVE.W    $00017108,D1         L06A8
00E2D0  E249                      LSR.W     #1,D1               
00E2D2  22790000AC4E              MOVEA.L   $0000AC4E,A1         L0009
00E2D8  D3C0                      ADDA.L    D0,A1               
00E2DA  D2C1                      ADDA.W    D1,A1               
00E2DC  26790000AC78              MOVEA.L   $0000AC78,A3         L0017
00E2E2  3C0E                __L0209:
                                  MOVE.W    A6,D6               
00E2E4  4240                __L020A:
                                  CLR.W     D0                  
00E2E6  4241                      CLR.W     D1                  
00E2E8  4242                      CLR.W     D2                  
00E2EA  4243                      CLR.W     D3                  
00E2EC  1018                      MOVE.B    (A0)+,D0            
00E2EE  1218                      MOVE.B    (A0)+,D1            
00E2F0  1418                      MOVE.B    (A0)+,D2            
00E2F2  1618                      MOVE.B    (A0)+,D3            
00E2F4  5848                      ADDQ.W    #4,A0               
00E2F6  D040                      ADD.W     D0,D0               
00E2F8  D241                      ADD.W     D1,D1               
00E2FA  D442                      ADD.W     D2,D2               
00E2FC  D643                      ADD.W     D3,D3               
00E2FE  30330000                  MOVE.W    0(A3,D0.W),D0       
00E302  D040                      ADD.W     D0,D0               
00E304  80732000                  OR.W      0(A3,D2.W),D0       
00E308  32331000                  MOVE.W    0(A3,D1.W),D1       
00E30C  D241                      ADD.W     D1,D1               
00E30E  82733000                  OR.W      0(A3,D3.W),D1       
00E312  32C0                      MOVE.W    D0,(A1)+            
00E314  4259                      CLR.W     (A1)+               
00E316  32C1                      MOVE.W    D1,(A1)+            
00E318  4259                      CLR.W     (A1)+               
00E31A  51CEFFC8                  DBF       D6,-56(PC)           __L020A
00E31E  D3C4                      ADDA.L    D4,A1               
00E320  D1C5                      ADDA.L    D5,A0               
00E322  51CFFFBE                  DBF       D7,-66(PC)           __L0209
00E326  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E32A  4E75                      RTS                           
00E32C  48E7FFFE            __L020B:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00E330  20790000AC78              MOVEA.L   $0000AC78,A0         L0017
00E336  4244                      CLR.W     D4                  
00E338  323C00FF                  MOVE.W    #$FF,D1             
00E33C  1004                __L020C:
                                  MOVE.B    D4,D0               
00E33E  4242                      CLR.W     D2                  
00E340  363C0007                  MOVE.W    #7,D3               
00E344  E34A                __L020D:
                                  LSL.W     #1,D2               
00E346  E308                      LSL.B     #1,D0               
00E348  E352                      ROXL.W    #1,D2               
00E34A  51CBFFF8                  DBF       D3,-8(PC)            __L020D
00E34E  30C2                      MOVE.W    D2,(A0)+            
00E350  5204                      ADDQ.B    #1,D4               
00E352  51C9FFE8                  DBF       D1,-24(PC)           __L020C
00E356  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E35A  4E75                      RTS                           
00E35C  BC7C000B            __L020E:
                                  CMP.W     #$B,D6              
00E360  6D00004A                  BLT       74(PC)               __L0210
00E364  0446000B                  SUBI.W    #$B,D6              
00E368  E546                      ASL.W     #2,D6               
00E36A  2F3B601C                  MOVE.L    28(PC,D6.W),-(A7)    __L020F
00E36E  4280                      CLR.L     D0                  
00E370  2200                      MOVE.L    D0,D1               
00E372  2400                      MOVE.L    D0,D2               
00E374  2600                      MOVE.L    D0,D3               
00E376  2800                      MOVE.L    D0,D4               
00E378  2A00                      MOVE.L    D0,D5               
00E37A  2C00                      MOVE.L    D0,D6               
00E37C  2440                      MOVEA.L   D0,A2               
00E37E  2640                      MOVEA.L   D0,A3               
00E380  2840                      MOVEA.L   D0,A4               
00E382  2A40                      MOVEA.L   D0,A5               
00E384  2C40                      MOVEA.L   D0,A6               
00E386  4E75                      RTS                           
00E388  0000E3C4            __L020F:
                                  DC.L      $0000E3C4            L0212
00E38C  0000E3D4                  DC.L      $0000E3D4            L0213
00E390  0000E3E8                  DC.L      $0000E3E8            L0214
00E394  0000E3FC                  DC.L      $0000E3FC            L0215
00E398  0000E410                  DC.L      $0000E410            L0216
00E39C  0000E424                  DC.L      $0000E424            L0217
00E3A0  0000E438                  DC.L      $0000E438            L0218
00E3A4  0000E44C                  DC.L      $0000E44C            L0219
00E3A8  0000E464                  DC.L      $0000E464            L021A
00E3AC  3846                __L0210:
                                  MOVEA.W   D6,A4               
00E3AE  4280                      CLR.L     D0                  
00E3B0  4281                      CLR.L     D1                  
00E3B2  48E1C000            L0211:MOVEM.L   D0-D1,-(A1)         
00E3B6  51CEFFFA                  DBF       D6,-6(PC)            L0211
00E3BA  D3C8                      ADDA.L    A0,A1               
00E3BC  3C0C                      MOVE.W    A4,D6               
00E3BE  51CFFFF2                  DBF       D7,-14(PC)           L0211
00E3C2  4E75                      RTS                           
00E3C4  48E1FE3E            __L0212:
                                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3C8  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3CC  D2C8                      ADDA.W    A0,A1               
00E3CE  51CFFFF4                  DBF       D7,-12(PC)           __L0212
00E3D2  4E75                      RTS                           
00E3D4  48E1FE3E            L0213:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3D8  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3DC  2300                      MOVE.L    D0,-(A1)            
00E3DE  2300                      MOVE.L    D0,-(A1)            
00E3E0  D2C8                      ADDA.W    A0,A1               
00E3E2  51CFFFF0                  DBF       D7,-16(PC)           L0213
00E3E6  4E75                      RTS                           
00E3E8  48E1FE3E            L0214:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3EC  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E3F0  48E1F000                  MOVEM.L   D0-D3,-(A1)         
00E3F4  D2C8                      ADDA.W    A0,A1               
00E3F6  51CFFFF0                  DBF       D7,-16(PC)           L0214
00E3FA  4E75                      RTS                           
00E3FC  48E1FE3E            L0215:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E400  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E404  48E1FC00                  MOVEM.L   D0-D5,-(A1)         
00E408  D2C8                      ADDA.W    A0,A1               
00E40A  51CFFFF0                  DBF       D7,-16(PC)           L0215
00E40E  4E75                      RTS                           
00E410  48E1FE3E            L0216:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E414  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E418  48E1FE20                  MOVEM.L   A2/D0-D6,-(A1)      
00E41C  D2C8                      ADDA.W    A0,A1               
00E41E  51CFFFF0                  DBF       D7,-16(PC)           L0216
00E422  4E75                      RTS                           
00E424  48E1FE3E            L0217:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E428  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E42C  48E1FE38                  MOVEM.L   A2-A4/D0-D6,-(A1)   
00E430  D2C8                      ADDA.W    A0,A1               
00E432  51CFFFF0                  DBF       D7,-16(PC)           L0217
00E436  4E75                      RTS                           
00E438  48E1FE3E            L0218:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E43C  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E440  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E444  D2C8                      ADDA.W    A0,A1               
00E446  51CFFFF0                  DBF       D7,-16(PC)           L0218
00E44A  4E75                      RTS                           
00E44C  48E1FE3E            L0219:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E450  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E454  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E458  48E1C000                  MOVEM.L   D0-D1,-(A1)         
00E45C  D2C8                      ADDA.W    A0,A1               
00E45E  51CFFFEC                  DBF       D7,-20(PC)           L0219
00E462  4E75                      RTS                           
00E464  48E1FE3E            L021A:MOVEM.L   A2-A6/D0-D6,-(A1)   
00E468  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E46C  48E1FE3E                  MOVEM.L   A2-A6/D0-D6,-(A1)   
00E470  48E1F000                  MOVEM.L   D0-D3,-(A1)         
00E474  D2C8                      ADDA.W    A0,A1               
00E476  51CFFFEC                  DBF       D7,-20(PC)           L021A
00E47A  4E75                      RTS                           
00E47C  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00E480  36390001712A              MOVE.W    $0001712A,D3         L06B8
00E486  967900017126              SUB.W     $00017126,D3         L06B6
00E48C  3A3900017128              MOVE.W    $00017128,D5         L06B7
00E492  9A7900017124              SUB.W     $00017124,D5         L06B5
00E498  E84D                      LSR.W     #4,D5               
00E49A  227900017224              MOVEA.L   $00017224,A1         L070B
00E4A0  32390001712A              MOVE.W    $0001712A,D1         L06B8
00E4A6  C2FC00A0                  MULU      #$A0,D1             
00E4AA  D2C1                      ADDA.W    D1,A1               
00E4AC  323900017128              MOVE.W    $00017128,D1         L06B7
00E4B2  E849                      LSR.W     #4,D1               
00E4B4  E741                      ASL.W     #3,D1               
00E4B6  06410008                  ADDI.W    #8,D1               
00E4BA  D2C1                      ADDA.W    D1,A1               
00E4BC  383900017128              MOVE.W    $00017128,D4         L06B7
00E4C2  987900017124              SUB.W     $00017124,D4         L06B5
00E4C8  E84C                      LSR.W     #4,D4               
00E4CA  E744                      ASL.W     #3,D4               
00E4CC  06440008                  ADDI.W    #8,D4               
00E4D0  4444                      NEG.W     D4                  
00E4D2  48C4                      EXT.L     D4                  
00E4D4  0684000000A0              ADDI.L    #$A0,D4             
00E4DA  4280                      CLR.L     D0                  
00E4DC  4281                      CLR.L     D1                  
00E4DE  3405                __L021B:
                                  MOVE.W    D5,D2               
00E4E0  48E1C000            __L021C:
                                  MOVEM.L   D0-D1,-(A1)         
00E4E4  51CAFFFA                  DBF       D2,-6(PC)            __L021C
00E4E8  93C4                      SUBA.L    D4,A1               
00E4EA  51CBFFF2                  DBF       D3,-14(PC)           __L021B
00E4EE  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E4F2  4E75                      RTS                           
00E4F4  48E7FFFE            L021D:MOVEM.L   A0-A6/D0-D7,-(A7)   
00E4F8  3E390001710E              MOVE.W    $0001710E,D7         L06AB
00E4FE  9E790001710A              SUB.W     $0001710A,D7         L06A9
00E504  3C390001710C              MOVE.W    $0001710C,D6         L06AA
00E50A  9C7900017108              SUB.W     $00017108,D6         L06A8
00E510  E84E                      LSR.W     #4,D6               
00E512  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00E518  32390001710E              MOVE.W    $0001710E,D1         L06AB
00E51E  C2FC00A0                  MULU      #$A0,D1             
00E522  D2C1                      ADDA.W    D1,A1               
00E524  32390001710C              MOVE.W    $0001710C,D1         L06AA
00E52A  E849                      LSR.W     #4,D1               
00E52C  E741                      ASL.W     #3,D1               
00E52E  06410008                  ADDI.W    #8,D1               
00E532  D2C1                      ADDA.W    D1,A1               
00E534  38390001710C              MOVE.W    $0001710C,D4         L06AA
00E53A  987900017108              SUB.W     $00017108,D4         L06A8
00E540  E84C                      LSR.W     #4,D4               
00E542  E744                      ASL.W     #3,D4               
00E544  06440008                  ADDI.W    #8,D4               
00E548  4444                      NEG.W     D4                  
00E54A  48C4                      EXT.L     D4                  
00E54C  0684000000A0              ADDI.L    #$A0,D4             
00E552  4484                      NEG.L     D4                  
00E554  2044                      MOVEA.L   D4,A0               
00E556  4EB90000E35C              JSR       $0000E35C            __L020E
00E55C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00E560  4E75                      RTS                           
00E562  2F0E                __L021E:
                                  MOVE.L    A6,-(A7)            
00E564  2C790000AC52              MOVEA.L   $0000AC52,A6         L000A
00E56A  23F90000AC4E0000AC52      MOVE.L    $0000AC4E,$0000AC52  L0009 L000A
00E574  33FC000000017108          MOVE.W    #0,$00017108         L06A8
00E57C  33FC00000001710A          MOVE.W    #0,$0001710A         L06A9
00E584  33FC013F0001710C          MOVE.W    #$13F,$0001710C      L06AA
00E58C  33FC00C70001710E          MOVE.W    #$C7,$0001710E       L06AB
00E594  4EB90000E4F4              JSR       $0000E4F4            L021D
00E59A  23CE0000AC52              MOVE.L    A6,$0000AC52         L000A
00E5A0  2C5F                      MOVEA.L   (A7)+,A6            
00E5A2  4E75                      RTS                           
00E5A4  49EA0006            __L021F:
                                  LEA       6(A2),A4            
00E5A8  303900017118              MOVE.W    $00017118,D0         L06AF
00E5AE  9068000E                  SUB.W     14(A0),D0           
00E5B2  343900017134              MOVE.W    $00017134,D2         L06BD
00E5B8  D442                      ADD.W     D2,D2               
00E5BA  C0C2                      MULU      D2,D0               
00E5BC  D9C0                      ADDA.L    D0,A4               
00E5BE  D8C2                      ADDA.W    D2,A4               
00E5C0  99FC00000010              SUBA.L    #$10,A4             
00E5C6  303900017116              MOVE.W    $00017116,D0         L06AE
00E5CC  9068000C                  SUB.W     12(A0),D0           
00E5D0  5340                      SUBQ.W    #1,D0               
00E5D2  E848                      LSR.W     #4,D0               
00E5D4  E740                      ASL.W     #3,D0               
00E5D6  42B90001711E              CLR.L     $0001711E            L06B2
00E5DC  B868000C                  CMP.W     12(A0),D4           
00E5E0  6E000010                  BGT       16(PC)               __L0220
00E5E4  33FC00010001711E          MOVE.W    #1,$0001711E         L06B2
00E5EC  4240                      CLR.W     D0                  
00E5EE  DCFC0008                  ADDA.W    #8,A6               
00E5F2  98C0                __L0220:
                                  SUBA.W    D0,A4               
00E5F4  303900017134              MOVE.W    $00017134,D0         L06BD
00E5FA  D040                      ADD.W     D0,D0               
00E5FC  D040                      ADD.W     D0,D0               
00E5FE  9CC0                      SUBA.W    D0,A6               
00E600  302A0002                  MOVE.W    2(A2),D0            
00E604  0040000F                  ORI.W     #$F,D0              
00E608  D068000C                  ADD.W     12(A0),D0           
00E60C  BA40                      CMP.W     D0,D5               
00E60E  6F00001E                  BLE       30(PC)               __L0222
00E612  33FC000100017120          MOVE.W    #1,$00017120         L06B3
00E61A  5346                      SUBQ.W    #1,D6               
00E61C  6A00000C                  BPL       12(PC)               __L0221
00E620  5246                      ADDQ.W    #1,D6               
00E622  33FCFFFF00017120          MOVE.W    #$FFFF,$00017120     L06B3
00E62A  DCFC0008            __L0221:
                                  ADDA.W    #8,A6               
00E62E  3A28000C            __L0222:
                                  MOVE.W    12(A0),D5           
00E632  0245000F                  ANDI.W    #$F,D5              
00E636  344E                      MOVEA.W   A6,A2               
00E638  204C                      MOVEA.L   A4,A0               
00E63A  2A7C00017244              MOVEA.L   #$00017244,A5        L071A
00E640  3C7900017120              MOVEA.W   $00017120,A6         L06B3
00E646  3846                      MOVEA.W   D6,A4               
00E648  4845                      SWAP      D5                  
00E64A  4A790001711E        __L0223:
                                  TST.W     $0001711E            L06B2
00E650  67000016                  BEQ       22(PC)               __L0224
00E654  7400                      MOVEQ     #0,D2               
00E656  7600                      MOVEQ     #0,D3               
00E658  4CE800030008              MOVEM.L   8(A0),D0-D1         
00E65E  380E                      MOVE.W    A6,D4               
00E660  6B0000BE                  BMI       190(PC)              __L0228
00E664  60000012                  BRA       18(PC)               __L0226
00E668  380E                __L0224:
                                  MOVE.W    A6,D4               
00E66A  6B0000AA                  BMI       170(PC)              __L0227
00E66E  4CD0000F            __L0225:
                                  MOVEM.L   (A0),D0-D3          
00E672  91FC00000008              SUBA.L    #8,A0               
00E678  4845                __L0226:
                                  SWAP      D5                  
00E67A  4846                      SWAP      D6                  
00E67C  4847                      SWAP      D7                  
00E67E  4842                      SWAP      D2                  
00E680  3800                      MOVE.W    D0,D4               
00E682  3002                      MOVE.W    D2,D0               
00E684  3404                      MOVE.W    D4,D2               
00E686  4842                      SWAP      D2                  
00E688  4843                      SWAP      D3                  
00E68A  3801                      MOVE.W    D1,D4               
00E68C  3203                      MOVE.W    D3,D1               
00E68E  3604                      MOVE.W    D4,D3               
00E690  4843                      SWAP      D3                  
00E692  EBA0                      ASL.L     D5,D0               
00E694  EBA1                      ASL.L     D5,D1               
00E696  EBA2                      ASL.L     D5,D2               
00E698  EBA3                      ASL.L     D5,D3               
00E69A  4845                      SWAP      D5                  
00E69C  4840                      SWAP      D0                  
00E69E  4841                      SWAP      D1                  
00E6A0  4842                      SWAP      D2                  
00E6A2  4843                      SWAP      D3                  
00E6A4  4244                      CLR.W     D4                  
00E6A6  1800                      MOVE.B    D0,D4               
00E6A8  18354000                  MOVE.B    0(A5,D4.W),D4       
00E6AC  E144                      ASL.W     #8,D4               
00E6AE  E048                      LSR.W     #8,D0               
00E6B0  18350000                  MOVE.B    0(A5,D0.W),D4       
00E6B4  1002                      MOVE.B    D2,D0               
00E6B6  1A350000                  MOVE.B    0(A5,D0.W),D5       
00E6BA  E145                      ASL.W     #8,D5               
00E6BC  E04A                      LSR.W     #8,D2               
00E6BE  1A352000                  MOVE.B    0(A5,D2.W),D5       
00E6C2  1001                      MOVE.B    D1,D0               
00E6C4  1C350000                  MOVE.B    0(A5,D0.W),D6       
00E6C8  E146                      ASL.W     #8,D6               
00E6CA  E049                      LSR.W     #8,D1               
00E6CC  1C351000                  MOVE.B    0(A5,D1.W),D6       
00E6D0  1003                      MOVE.B    D3,D0               
00E6D2  1E350000                  MOVE.B    0(A5,D0.W),D7       
00E6D6  E147                      ASL.W     #8,D7               
00E6D8  E04B                      LSR.W     #8,D3               
00E6DA  1E353000                  MOVE.B    0(A5,D3.W),D7       
00E6DE  3004                      MOVE.W    D4,D0               
00E6E0  8045                      OR.W      D5,D0               
00E6E2  8046                      OR.W      D6,D0               
00E6E4  8047                      OR.W      D7,D0               
00E6E6  4640                      NOT.W     D0                  
00E6E8  4844                      SWAP      D4                  
00E6EA  3800                      MOVE.W    D0,D4               
00E6EC  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00E6F0  C044                      AND.W     D4,D0               
00E6F2  C244                      AND.W     D4,D1               
00E6F4  C444                      AND.W     D4,D2               
00E6F6  C644                      AND.W     D4,D3               
00E6F8  4844                      SWAP      D4                  
00E6FA  8840                      OR.W      D0,D4               
00E6FC  8A41                      OR.W      D1,D5               
00E6FE  8C42                      OR.W      D2,D6               
00E700  8E43                      OR.W      D3,D7               
00E702  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00E708  4846                      SWAP      D6                  
00E70A  4847                      SWAP      D7                  
00E70C  51CEFF60                  DBF       D6,-160(PC)          __L0225
00E710  380E                      MOVE.W    A6,D4               
00E712  670000A0                  BEQ       160(PC)              __L0229
00E716  7000                __L0227:
                                  MOVEQ     #0,D0               
00E718  7200                      MOVEQ     #0,D1               
00E71A  4CE8000C0008              MOVEM.L   8(A0),D2-D3         
00E720  4845                __L0228:
                                  SWAP      D5                  
00E722  4846                      SWAP      D6                  
00E724  4847                      SWAP      D7                  
00E726  4842                      SWAP      D2                  
00E728  3800                      MOVE.W    D0,D4               
00E72A  3002                      MOVE.W    D2,D0               
00E72C  3404                      MOVE.W    D4,D2               
00E72E  4842                      SWAP      D2                  
00E730  4843                      SWAP      D3                  
00E732  3801                      MOVE.W    D1,D4               
00E734  3203                      MOVE.W    D3,D1               
00E736  3604                      MOVE.W    D4,D3               
00E738  4843                      SWAP      D3                  
00E73A  EBA0                      ASL.L     D5,D0               
00E73C  EBA1                      ASL.L     D5,D1               
00E73E  EBA2                      ASL.L     D5,D2               
00E740  EBA3                      ASL.L     D5,D3               
00E742  4845                      SWAP      D5                  
00E744  4840                      SWAP      D0                  
00E746  4841                      SWAP      D1                  
00E748  4842                      SWAP      D2                  
00E74A  4843                      SWAP      D3                  
00E74C  4244                      CLR.W     D4                  
00E74E  1800                      MOVE.B    D0,D4               
00E750  18354000                  MOVE.B    0(A5,D4.W),D4       
00E754  E144                      ASL.W     #8,D4               
00E756  E048                      LSR.W     #8,D0               
00E758  18350000                  MOVE.B    0(A5,D0.W),D4       
00E75C  1002                      MOVE.B    D2,D0               
00E75E  1A350000                  MOVE.B    0(A5,D0.W),D5       
00E762  E145                      ASL.W     #8,D5               
00E764  E04A                      LSR.W     #8,D2               
00E766  1A352000                  MOVE.B    0(A5,D2.W),D5       
00E76A  1001                      MOVE.B    D1,D0               
00E76C  1C350000                  MOVE.B    0(A5,D0.W),D6       
00E770  E146                      ASL.W     #8,D6               
00E772  E049                      LSR.W     #8,D1               
00E774  1C351000                  MOVE.B    0(A5,D1.W),D6       
00E778  1003                      MOVE.B    D3,D0               
00E77A  1E350000                  MOVE.B    0(A5,D0.W),D7       
00E77E  E147                      ASL.W     #8,D7               
00E780  E04B                      LSR.W     #8,D3               
00E782  1E353000                  MOVE.B    0(A5,D3.W),D7       
00E786  3004                      MOVE.W    D4,D0               
00E788  8045                      OR.W      D5,D0               
00E78A  8046                      OR.W      D6,D0               
00E78C  8047                      OR.W      D7,D0               
00E78E  4640                      NOT.W     D0                  
00E790  4844                      SWAP      D4                  
00E792  3800                      MOVE.W    D0,D4               
00E794  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00E798  C044                      AND.W     D4,D0               
00E79A  C244                      AND.W     D4,D1               
00E79C  C444                      AND.W     D4,D2               
00E79E  C644                      AND.W     D4,D3               
00E7A0  4844                      SWAP      D4                  
00E7A2  8840                      OR.W      D0,D4               
00E7A4  8A41                      OR.W      D1,D5               
00E7A6  8C42                      OR.W      D2,D6               
00E7A8  8E43                      OR.W      D3,D7               
00E7AA  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00E7B0  4846                      SWAP      D6                  
00E7B2  4847                      SWAP      D7                  
00E7B4  91CA                __L0229:
                                  SUBA.L    A2,A0               
00E7B6  D3CB                      ADDA.L    A3,A1               
00E7B8  3C0C                      MOVE.W    A4,D6               
00E7BA  51CFFE8E                  DBF       D7,-370(PC)          __L0223
00E7BE  4E75                      RTS                           
00E7C0  49EA0006                  LEA       6(A2),A4            
00E7C4  303900017118              MOVE.W    $00017118,D0         L06AF
00E7CA  9068000E                  SUB.W     14(A0),D0           
00E7CE  343900017134              MOVE.W    $00017134,D2         L06BD
00E7D4  D442                      ADD.W     D2,D2               
00E7D6  C0C2                      MULU      D2,D0               
00E7D8  D9C0                      ADDA.L    D0,A4               
00E7DA  D8C2                      ADDA.W    D2,A4               
00E7DC  99FC00000010              SUBA.L    #$10,A4             
00E7E2  303900017116              MOVE.W    $00017116,D0         L06AE
00E7E8  9068000C                  SUB.W     12(A0),D0           
00E7EC  5340                      SUBQ.W    #1,D0               
00E7EE  E848                      LSR.W     #4,D0               
00E7F0  E740                      ASL.W     #3,D0               
00E7F2  42B90001711E              CLR.L     $0001711E            L06B2
00E7F8  B868000C                  CMP.W     12(A0),D4           
00E7FC  6E000010                  BGT       16(PC)               __L022A
00E800  33FC00010001711E          MOVE.W    #1,$0001711E         L06B2
00E808  4240                      CLR.W     D0                  
00E80A  DCFC0008                  ADDA.W    #8,A6               
00E80E  98C0                __L022A:
                                  SUBA.W    D0,A4               
00E810  303900017134              MOVE.W    $00017134,D0         L06BD
00E816  D040                      ADD.W     D0,D0               
00E818  D040                      ADD.W     D0,D0               
00E81A  9CC0                      SUBA.W    D0,A6               
00E81C  302A0002                  MOVE.W    2(A2),D0            
00E820  0040000F                  ORI.W     #$F,D0              
00E824  D068000C                  ADD.W     12(A0),D0           
00E828  BA40                      CMP.W     D0,D5               
00E82A  6F00001E                  BLE       30(PC)               __L022C
00E82E  33FC000100017120          MOVE.W    #1,$00017120         L06B3
00E836  5346                      SUBQ.W    #1,D6               
00E838  6A00000C                  BPL       12(PC)               __L022B
00E83C  5246                      ADDQ.W    #1,D6               
00E83E  33FCFFFF00017120          MOVE.W    #$FFFF,$00017120     L06B3
00E846  DCFC0008            __L022B:
                                  ADDA.W    #8,A6               
00E84A  3A28000C            __L022C:
                                  MOVE.W    12(A0),D5           
00E84E  0245000F                  ANDI.W    #$F,D5              
00E852  344E                      MOVEA.W   A6,A2               
00E854  204C                      MOVEA.L   A4,A0               
00E856  2A7C00017244              MOVEA.L   #$00017244,A5        L071A
00E85C  3C7900017120              MOVEA.W   $00017120,A6         L06B3
00E862  3846                      MOVEA.W   D6,A4               
00E864  4845                      SWAP      D5                  
00E866  4A790001711E        __L022D:
                                  TST.W     $0001711E            L06B2
00E86C  670000AA                  BEQ       170(PC)              __L022E
00E870  7400                      MOVEQ     #0,D2               
00E872  7600                      MOVEQ     #0,D3               
00E874  4CE800030008              MOVEM.L   8(A0),D0-D1         
00E87A  380E                      MOVE.W    A6,D4               
00E87C  6B000132                  BMI       306(PC)              __L0232
00E880  4845                      SWAP      D5                  
00E882  4846                      SWAP      D6                  
00E884  4847                      SWAP      D7                  
00E886  4842                      SWAP      D2                  
00E888  3800                      MOVE.W    D0,D4               
00E88A  3002                      MOVE.W    D2,D0               
00E88C  3404                      MOVE.W    D4,D2               
00E88E  4842                      SWAP      D2                  
00E890  4843                      SWAP      D3                  
00E892  3801                      MOVE.W    D1,D4               
00E894  3203                      MOVE.W    D3,D1               
00E896  3604                      MOVE.W    D4,D3               
00E898  4843                      SWAP      D3                  
00E89A  EBA0                      ASL.L     D5,D0               
00E89C  EBA1                      ASL.L     D5,D1               
00E89E  EBA2                      ASL.L     D5,D2               
00E8A0  EBA3                      ASL.L     D5,D3               
00E8A2  4845                      SWAP      D5                  
00E8A4  4840                      SWAP      D0                  
00E8A6  4841                      SWAP      D1                  
00E8A8  4842                      SWAP      D2                  
00E8AA  4843                      SWAP      D3                  
00E8AC  4244                      CLR.W     D4                  
00E8AE  1800                      MOVE.B    D0,D4               
00E8B0  18354000                  MOVE.B    0(A5,D4.W),D4       
00E8B4  E144                      ASL.W     #8,D4               
00E8B6  E048                      LSR.W     #8,D0               
00E8B8  18350000                  MOVE.B    0(A5,D0.W),D4       
00E8BC  1002                      MOVE.B    D2,D0               
00E8BE  1A350000                  MOVE.B    0(A5,D0.W),D5       
00E8C2  E145                      ASL.W     #8,D5               
00E8C4  E04A                      LSR.W     #8,D2               
00E8C6  1A352000                  MOVE.B    0(A5,D2.W),D5       
00E8CA  1001                      MOVE.B    D1,D0               
00E8CC  1C350000                  MOVE.B    0(A5,D0.W),D6       
00E8D0  E146                      ASL.W     #8,D6               
00E8D2  E049                      LSR.W     #8,D1               
00E8D4  1C351000                  MOVE.B    0(A5,D1.W),D6       
00E8D8  1003                      MOVE.B    D3,D0               
00E8DA  1E350000                  MOVE.B    0(A5,D0.W),D7       
00E8DE  E147                      ASL.W     #8,D7               
00E8E0  E04B                      LSR.W     #8,D3               
00E8E2  1E353000                  MOVE.B    0(A5,D3.W),D7       
00E8E6  3004                      MOVE.W    D4,D0               
00E8E8  8045                      OR.W      D5,D0               
00E8EA  8046                      OR.W      D6,D0               
00E8EC  8047                      OR.W      D7,D0               
00E8EE  4640                      NOT.W     D0                  
00E8F0  4844                      SWAP      D4                  
00E8F2  3800                      MOVE.W    D0,D4               
00E8F4  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00E8F8  C044                      AND.W     D4,D0               
00E8FA  C244                      AND.W     D4,D1               
00E8FC  C444                      AND.W     D4,D2               
00E8FE  C644                      AND.W     D4,D3               
00E900  4844                      SWAP      D4                  
00E902  8840                      OR.W      D0,D4               
00E904  8A41                      OR.W      D1,D5               
00E906  8C42                      OR.W      D2,D6               
00E908  8E43                      OR.W      D3,D7               
00E90A  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00E910  4846                      SWAP      D6                  
00E912  4847                      SWAP      D7                  
00E914  60000086                  BRA       134(PC)              __L0230
00E918  380E                __L022E:
                                  MOVE.W    A6,D4               
00E91A  6B00008A                  BMI       138(PC)              __L0231
00E91E  4CD0000F            L022F:MOVEM.L   (A0),D0-D3          
00E922  91FC00000008              SUBA.L    #8,A0               
00E928  4845                      SWAP      D5                  
00E92A  4846                      SWAP      D6                  
00E92C  4847                      SWAP      D7                  
00E92E  4842                      SWAP      D2                  
00E930  3800                      MOVE.W    D0,D4               
00E932  3002                      MOVE.W    D2,D0               
00E934  3404                      MOVE.W    D4,D2               
00E936  4842                      SWAP      D2                  
00E938  4843                      SWAP      D3                  
00E93A  3801                      MOVE.W    D1,D4               
00E93C  3203                      MOVE.W    D3,D1               
00E93E  3604                      MOVE.W    D4,D3               
00E940  4843                      SWAP      D3                  
00E942  EBA0                      ASL.L     D5,D0               
00E944  EBA1                      ASL.L     D5,D1               
00E946  EBA2                      ASL.L     D5,D2               
00E948  EBA3                      ASL.L     D5,D3               
00E94A  4845                      SWAP      D5                  
00E94C  4840                      SWAP      D0                  
00E94E  4841                      SWAP      D1                  
00E950  4842                      SWAP      D2                  
00E952  4843                      SWAP      D3                  
00E954  4244                      CLR.W     D4                  
00E956  1800                      MOVE.B    D0,D4               
00E958  18354000                  MOVE.B    0(A5,D4.W),D4       
00E95C  E144                      ASL.W     #8,D4               
00E95E  E048                      LSR.W     #8,D0               
00E960  18350000                  MOVE.B    0(A5,D0.W),D4       
00E964  1002                      MOVE.B    D2,D0               
00E966  1A350000                  MOVE.B    0(A5,D0.W),D5       
00E96A  E145                      ASL.W     #8,D5               
00E96C  E04A                      LSR.W     #8,D2               
00E96E  1A352000                  MOVE.B    0(A5,D2.W),D5       
00E972  1001                      MOVE.B    D1,D0               
00E974  1C350000                  MOVE.B    0(A5,D0.W),D6       
00E978  E146                      ASL.W     #8,D6               
00E97A  E049                      LSR.W     #8,D1               
00E97C  1C351000                  MOVE.B    0(A5,D1.W),D6       
00E980  1003                      MOVE.B    D3,D0               
00E982  1E350000                  MOVE.B    0(A5,D0.W),D7       
00E986  E147                      ASL.W     #8,D7               
00E988  E04B                      LSR.W     #8,D3               
00E98A  1E353000                  MOVE.B    0(A5,D3.W),D7       
00E98E  489100F0                  MOVEM.W   D4-D7,(A1)          
00E992  D3FC00000008              ADDA.L    #8,A1               
00E998  4846                      SWAP      D6                  
00E99A  4847                      SWAP      D7                  
00E99C  51CEFF80            __L0230:
                                  DBF       D6,-128(PC)          L022F
00E9A0  380E                      MOVE.W    A6,D4               
00E9A2  670000A0                  BEQ       160(PC)              __L0233
00E9A6  7000                __L0231:
                                  MOVEQ     #0,D0               
00E9A8  7200                      MOVEQ     #0,D1               
00E9AA  4CE8000C0008              MOVEM.L   8(A0),D2-D3         
00E9B0  4845                __L0232:
                                  SWAP      D5                  
00E9B2  4846                      SWAP      D6                  
00E9B4  4847                      SWAP      D7                  
00E9B6  4842                      SWAP      D2                  
00E9B8  3800                      MOVE.W    D0,D4               
00E9BA  3002                      MOVE.W    D2,D0               
00E9BC  3404                      MOVE.W    D4,D2               
00E9BE  4842                      SWAP      D2                  
00E9C0  4843                      SWAP      D3                  
00E9C2  3801                      MOVE.W    D1,D4               
00E9C4  3203                      MOVE.W    D3,D1               
00E9C6  3604                      MOVE.W    D4,D3               
00E9C8  4843                      SWAP      D3                  
00E9CA  EBA0                      ASL.L     D5,D0               
00E9CC  EBA1                      ASL.L     D5,D1               
00E9CE  EBA2                      ASL.L     D5,D2               
00E9D0  EBA3                      ASL.L     D5,D3               
00E9D2  4845                      SWAP      D5                  
00E9D4  4840                      SWAP      D0                  
00E9D6  4841                      SWAP      D1                  
00E9D8  4842                      SWAP      D2                  
00E9DA  4843                      SWAP      D3                  
00E9DC  4244                      CLR.W     D4                  
00E9DE  1800                      MOVE.B    D0,D4               
00E9E0  18354000                  MOVE.B    0(A5,D4.W),D4       
00E9E4  E144                      ASL.W     #8,D4               
00E9E6  E048                      LSR.W     #8,D0               
00E9E8  18350000                  MOVE.B    0(A5,D0.W),D4       
00E9EC  1002                      MOVE.B    D2,D0               
00E9EE  1A350000                  MOVE.B    0(A5,D0.W),D5       
00E9F2  E145                      ASL.W     #8,D5               
00E9F4  E04A                      LSR.W     #8,D2               
00E9F6  1A352000                  MOVE.B    0(A5,D2.W),D5       
00E9FA  1001                      MOVE.B    D1,D0               
00E9FC  1C350000                  MOVE.B    0(A5,D0.W),D6       
00EA00  E146                      ASL.W     #8,D6               
00EA02  E049                      LSR.W     #8,D1               
00EA04  1C351000                  MOVE.B    0(A5,D1.W),D6       
00EA08  1003                      MOVE.B    D3,D0               
00EA0A  1E350000                  MOVE.B    0(A5,D0.W),D7       
00EA0E  E147                      ASL.W     #8,D7               
00EA10  E04B                      LSR.W     #8,D3               
00EA12  1E353000                  MOVE.B    0(A5,D3.W),D7       
00EA16  3004                      MOVE.W    D4,D0               
00EA18  8045                      OR.W      D5,D0               
00EA1A  8046                      OR.W      D6,D0               
00EA1C  8047                      OR.W      D7,D0               
00EA1E  4640                      NOT.W     D0                  
00EA20  4844                      SWAP      D4                  
00EA22  3800                      MOVE.W    D0,D4               
00EA24  4C99000F                  MOVEM.W   (A1)+,D0-D3         
00EA28  C044                      AND.W     D4,D0               
00EA2A  C244                      AND.W     D4,D1               
00EA2C  C444                      AND.W     D4,D2               
00EA2E  C644                      AND.W     D4,D3               
00EA30  4844                      SWAP      D4                  
00EA32  8840                      OR.W      D0,D4               
00EA34  8A41                      OR.W      D1,D5               
00EA36  8C42                      OR.W      D2,D6               
00EA38  8E43                      OR.W      D3,D7               
00EA3A  48A900F0FFF8              MOVEM.W   D4-D7,-8(A1)        
00EA40  4846                      SWAP      D6                  
00EA42  4847                      SWAP      D7                  
00EA44  91CA                __L0233:
                                  SUBA.L    A2,A0               
00EA46  D3CB                      ADDA.L    A3,A1               
00EA48  3C0C                      MOVE.W    A4,D6               
00EA4A  51CFFE1A                  DBF       D7,-486(PC)          __L022D
00EA4E  4E75                      RTS                           
00EA50  0000                L0234:DC.B      $00,$00 
00EA52  0000                L0235:DC.B      $00,$00 
00EA54  0000                L0236:DC.B      $00,$00 
00EA56                      L0237:DCB.W     2,0 
00EA5A                      L0238:DCB.W     2,0 
00EA5E                      L0239:DCB.W     2,0 
00EA62                      L023A:DCB.W     2,0 
00EA66  0000                L023B:DC.B      $00,$00 
00EA68  0000                L023C:DC.B      $00,$00 
00EA6A  0000                __L023D:
                                  DC.B      $00,$00 
00EA6C                      L023E:DCB.W     2,0 
00EA70                      L023F:DCB.W     2,0 
00EA74  0000                L0240:DC.B      $00,$00 
00EA76  0000                __L0241:
                                  DC.B      $00,$00 
00EA78  0000                L0242:DC.B      $00,$00 
00EA7A  0000                __L0243:
                                  DC.B      $00,$00 
00EA7C                      L0244:DCB.W     2,0 
00EA80                      L0245:DCB.W     2,0 
00EA84  0000                L0246:DC.B      $00,$00 
00EA86  4EB90000AC2C        __L0247:
                                  JSR       $0000AC2C            L0003
00EA8C  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00EA90  42B90000EA5E              CLR.L     $0000EA5E            L0239
00EA96  23F90000AC4E0000EA62      MOVE.L    $0000AC4E,$0000EA62  L0009 L023A
00EAA0  203C00007E00              MOVE.L    #$7E00,D0           
00EAA6  4EB90000BBF8              JSR       $0000BBF8            L0092
00EAAC  4A80                      TST.L     D0                  
00EAAE  67000038                  BEQ       56(PC)               __L0248
00EAB2  23C00000EA5E              MOVE.L    D0,$0000EA5E         L0239
00EAB8  5380                      SUBQ.L    #1,D0               
00EABA  0080000000FF              ORI.L     #$FF,D0             
00EAC0  5280                      ADDQ.L    #1,D0               
00EAC2  23C00000EA62              MOVE.L    D0,$0000EA62         L023A
00EAC8  20790000AC4E              MOVEA.L   $0000AC4E,A0         L0009
00EACE  22790000EA62              MOVEA.L   $0000EA62,A1         L023A
00EAD4  3E3C00C7                  MOVE.W    #$C7,D7             
00EAD8  3C3C0013                  MOVE.W    #$13,D6             
00EADC  247C00000000              MOVEA.L   #0,A2               
00EAE2  264A                      MOVEA.L   A2,A3               
00EAE4  6100F3EE                  BSR       -3090(PC)            L01F5
00EAE8  42390001718B        __L0248:
                                  CLR.B     $0001718B            L06DC
00EAEE  6100010C                  BSR       268(PC)              __L0252
00EAF2  4A790001723E              TST.W     $0001723E            L0717
00EAF8  67000006                  BEQ       6(PC)                __L0249
00EAFC  610001E4                  BSR       484(PC)              __L0258
00EB00  4A7900017240        __L0249:
                                  TST.W     $00017240            L0718
00EB06  67000006                  BEQ       6(PC)                __L024A
00EB0A  610002A0                  BSR       672(PC)              __L025D
00EB0E  383900017242        __L024A:
                                  MOVE.W    $00017242,D4         L0719
00EB14  5344                      SUBQ.W    #1,D4               
00EB16  6B0000B2                  BMI       178(PC)              L0250
00EB1A  3F04                __L024B:
                                  MOVE.W    D4,-(A7)            
00EB1C  6100032C                  BSR       812(PC)              __L0261
00EB20  4A790001723E              TST.W     $0001723E            L0717
00EB26  67000006                  BEQ       6(PC)                __L024C
00EB2A  610003CC                  BSR       972(PC)              __L0266
00EB2E  4A7900017240        __L024C:
                                  TST.W     $00017240            L0718
00EB34  67000006                  BEQ       6(PC)                __L024D
00EB38  610003FC                  BSR       1020(PC)             __L0267
00EB3C  4AB90000EA5E        __L024D:
                                  TST.L     $0000EA5E            L0239
00EB42  67000024                  BEQ       36(PC)               __L024E
00EB46  20390000EA62              MOVE.L    $0000EA62,D0         L023A
00EB4C  23F90000AC4E0000EA62      MOVE.L    $0000AC4E,$0000EA62  L0009 L023A
00EB56  23C00000AC4E              MOVE.L    D0,$0000AC4E         L0009
00EB5C  4EB90000B64E              JSR       $0000B64E            L0056
00EB62  4EB90000B642              JSR       $0000B642            L0055
00EB68  381F                __L024E:
                                  MOVE.W    (A7)+,D4            
00EB6A  51CCFFAE                  DBF       D4,-82(PC)           __L024B
00EB6E  4AB90000EA5E              TST.L     $0000EA5E            L0239
00EB74  67000054                  BEQ       84(PC)               L0250
00EB78  20390000EA5E              MOVE.L    $0000EA5E,D0         L0239
00EB7E  B0B90000AC4E              CMP.L     $0000AC4E,D0         L0009
00EB84  66000038                  BNE       56(PC)               __L024F
00EB88  20790000AC4E              MOVEA.L   $0000AC4E,A0         L0009
00EB8E  22790000EA62              MOVEA.L   $0000EA62,A1         L023A
00EB94  3E3C00C7                  MOVE.W    #$C7,D7             
00EB98  3C3C0013                  MOVE.W    #$13,D6             
00EB9C  247C00000000              MOVEA.L   #0,A2               
00EBA2  264A                      MOVEA.L   A2,A3               
00EBA4  6100F32E                  BSR       -3282(PC)            L01F5
00EBA8  23F90000EA620000AC4E      MOVE.L    $0000EA62,$0000AC4E  L023A L0009
00EBB2  4EB90000B64E              JSR       $0000B64E            L0056
00EBB8  4EB90000B642              JSR       $0000B642            L0055
00EBBE  20390000EA5E        __L024F:
                                  MOVE.L    $0000EA5E,D0         L0239
00EBC4  4EB90000BC34              JSR       $0000BC34            __L0095
00EBCA  0C79001400017242    L0250:CMPI.W    #$14,$00017242       L0719
00EBD2  67000022                  BEQ       34(PC)               __L0251
00EBD6  20790000AC4E              MOVEA.L   $0000AC4E,A0         L0009
00EBDC  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00EBE2  3E3C00C7                  MOVE.W    #$C7,D7             
00EBE6  3C3C0013                  MOVE.W    #$13,D6             
00EBEA  247C00000000              MOVEA.L   #0,A2               
00EBF0  264A                      MOVEA.L   A2,A3               
00EBF2  6100F2E0                  BSR       -3360(PC)            L01F5
00EBF6  4CDF7FFF            __L0251:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00EBFA  4E75                      RTS                           
00EBFC  207C00000000        __L0252:
                                  MOVEA.L   #0,A0               
00EC02  30390001710A              MOVE.W    $0001710A,D0         L06A9
00EC08  C0FC00A0                  MULU      #$A0,D0             
00EC0C  323900017108              MOVE.W    $00017108,D1         L06A8
00EC12  E849                      LSR.W     #4,D1               
00EC14  E749                      LSL.W     #3,D1               
00EC16  D041                      ADD.W     D1,D0               
00EC18  D0C0                      ADDA.W    D0,A0               
00EC1A  227C00000000              MOVEA.L   #0,A1               
00EC20  30390001710E              MOVE.W    $0001710E,D0         L06AB
00EC26  C0FC00A0                  MULU      #$A0,D0             
00EC2A  32390001710C              MOVE.W    $0001710C,D1         L06AA
00EC30  5241                      ADDQ.W    #1,D1               
00EC32  E849                      LSR.W     #4,D1               
00EC34  E749                      LSL.W     #3,D1               
00EC36  D041                      ADD.W     D1,D0               
00EC38  D2C0                      ADDA.W    D0,A1               
00EC3A  303900017240              MOVE.W    $00017240,D0         L0718
00EC40  C0FC00A0                  MULU      #$A0,D0             
00EC44  32390001723E              MOVE.W    $0001723E,D1         L0717
00EC4A  E841                      ASR.W     #4,D1               
00EC4C  E741                      ASL.W     #3,D1               
00EC4E  D041                      ADD.W     D1,D0               
00EC50  3A390001710C              MOVE.W    $0001710C,D5         L06AA
00EC56  9A7900017108              SUB.W     $00017108,D5         L06A8
00EC5C  5245                      ADDQ.W    #1,D5               
00EC5E  E84D                      LSR.W     #4,D5               
00EC60  E74D                      LSL.W     #3,D5               
00EC62  4445                      NEG.W     D5                  
00EC64  064500A0                  ADDI.W    #$A0,D5             
00EC68  32390001723E              MOVE.W    $0001723E,D1         L0717
00EC6E  6A000004                  BPL       4(PC)                __L0253
00EC72  4441                      NEG.W     D1                  
00EC74  E849                __L0253:
                                  LSR.W     #4,D1               
00EC76  E749                      LSL.W     #3,D1               
00EC78  DA41                      ADD.W     D1,D5               
00EC7A  33C50000EA50              MOVE.W    D5,$0000EA50         L0234
00EC80  4440                      NEG.W     D0                  
00EC82  6B00000A                  BMI       10(PC)               __L0254
00EC86  2248                      MOVEA.L   A0,A1               
00EC88  D0C0                      ADDA.W    D0,A0               
00EC8A  60000006                  BRA       6(PC)                __L0255
00EC8E  2049                __L0254:
                                  MOVEA.L   A1,A0               
00EC90  D0C0                      ADDA.W    D0,A0               
00EC92  23C80000EA56        __L0255:
                                  MOVE.L    A0,$0000EA56         L0237
00EC98  23C90000EA5A              MOVE.L    A1,$0000EA5A         L0238
00EC9E  3E390001710E              MOVE.W    $0001710E,D7         L06AB
00ECA4  9E790001710A              SUB.W     $0001710A,D7         L06A9
00ECAA  303900017240              MOVE.W    $00017240,D0         L0718
00ECB0  6A000004                  BPL       4(PC)                __L0256
00ECB4  4440                      NEG.W     D0                  
00ECB6  9E40                __L0256:
                                  SUB.W     D0,D7               
00ECB8  33C70000EA54              MOVE.W    D7,$0000EA54         L0236
00ECBE  3C390001710C              MOVE.W    $0001710C,D6         L06AA
00ECC4  9C7900017108              SUB.W     $00017108,D6         L06A8
00ECCA  30390001723E              MOVE.W    $0001723E,D0         L0717
00ECD0  6A000004                  BPL       4(PC)                __L0257
00ECD4  4440                      NEG.W     D0                  
00ECD6  9C40                __L0257:
                                  SUB.W     D0,D6               
00ECD8  E84E                      LSR.W     #4,D6               
00ECDA  33C60000EA52              MOVE.W    D6,$0000EA52         L0235
00ECE0  4E75                      RTS                           
00ECE2  30390001710A        __L0258:
                                  MOVE.W    $0001710A,D0         L06A9
00ECE8  C0FC00A0                  MULU      #$A0,D0             
00ECEC  323900017108              MOVE.W    $00017108,D1         L06A8
00ECF2  4A790001723E              TST.W     $0001723E            L0717
00ECF8  6A000008                  BPL       8(PC)                __L0259
00ECFC  32390001710C              MOVE.W    $0001710C,D1         L06AA
00ED02  E849                __L0259:
                                  LSR.W     #4,D1               
00ED04  E749                      LSL.W     #3,D1               
00ED06  D240                      ADD.W     D0,D1               
00ED08  227C00000000              MOVEA.L   #0,A1               
00ED0E  D2C1                      ADDA.W    D1,A1               
00ED10  23C90000EA70              MOVE.L    A1,$0000EA70         L023F
00ED16  36390001710C              MOVE.W    $0001710C,D3         L06AA
00ED1C  967900017108              SUB.W     $00017108,D3         L06A8
00ED22  5243                      ADDQ.W    #1,D3               
00ED24  E84B                      LSR.W     #4,D3               
00ED26  967900017242              SUB.W     $00017242,D3         L0719
00ED2C  343900017108              MOVE.W    $00017108,D2         L06A8
00ED32  4A790001723E              TST.W     $0001723E            L0717
00ED38  6B000012                  BMI       18(PC)               __L025A
00ED3C  34390001710C              MOVE.W    $0001710C,D2         L06AA
00ED42  5242                      ADDQ.W    #1,D2               
00ED44  94790001723E              SUB.W     $0001723E,D2         L0717
00ED4A  4443                      NEG.W     D3                  
00ED4C  E84A                __L025A:
                                  LSR.W     #4,D2               
00ED4E  D443                      ADD.W     D3,D2               
00ED50  E74A                      LSL.W     #3,D2               
00ED52  D440                      ADD.W     D0,D2               
00ED54  20790000AC52              MOVEA.L   $0000AC52,A0         L000A
00ED5A  D0C2                      ADDA.W    D2,A0               
00ED5C  23C80000EA6C              MOVE.L    A0,$0000EA6C         L023E
00ED62  30390001723E              MOVE.W    $0001723E,D0         L0717
00ED68  6A000004                  BPL       4(PC)                __L025B
00ED6C  4440                      NEG.W     D0                  
00ED6E  E848                __L025B:
                                  LSR.W     #4,D0               
00ED70  33C00000EA68              MOVE.W    D0,$0000EA68         L023C
00ED76  53790000EA68              SUBQ.W    #1,$0000EA68         L023C
00ED7C  E748                      LSL.W     #3,D0               
00ED7E  33C00000EA74              MOVE.W    D0,$0000EA74         L0240
00ED84  4A790001723E              TST.W     $0001723E            L0717
00ED8A  6B000008                  BMI       8(PC)                __L025C
00ED8E  44790000EA74              NEG.W     $0000EA74            L0240
00ED94  4440                __L025C:
                                  NEG.W     D0                  
00ED96  064000A0                  ADDI.W    #$A0,D0             
00ED9A  33C00000EA66              MOVE.W    D0,$0000EA66         L023B
00EDA0  33F90000EA540000EA6A      MOVE.W    $0000EA54,$0000EA6A  L0236 L023D
00EDAA  4E75                      RTS                           
00EDAC  32390001710A        __L025D:
                                  MOVE.W    $0001710A,D1         L06A9
00EDB2  4A7900017240              TST.W     $00017240            L0718
00EDB8  6A000010                  BPL       16(PC)               __L025E
00EDBC  32390001710E              MOVE.W    $0001710E,D1         L06AB
00EDC2  5241                      ADDQ.W    #1,D1               
00EDC4  D27900017240              ADD.W     $00017240,D1         L0718
00EDCA  C2FC00A0            __L025E:
                                  MULU      #$A0,D1             
00EDCE  303900017108              MOVE.W    $00017108,D0         L06A8
00EDD4  E848                      LSR.W     #4,D0               
00EDD6  E748                      LSL.W     #3,D0               
00EDD8  D240                      ADD.W     D0,D1               
00EDDA  227C00000000              MOVEA.L   #0,A1               
00EDE0  D2C1                      ADDA.W    D1,A1               
00EDE2  23C90000EA80              MOVE.L    A1,$0000EA80         L0245
00EDE8  34390001710A              MOVE.W    $0001710A,D2         L06A9
00EDEE  4A7900017240              TST.W     $00017240            L0718
00EDF4  6B000010                  BMI       16(PC)               __L025F
00EDF8  34390001710E              MOVE.W    $0001710E,D2         L06AB
00EDFE  5242                      ADDQ.W    #1,D2               
00EE00  947900017240              SUB.W     $00017240,D2         L0718
00EE06  C4FC00A0            __L025F:
                                  MULU      #$A0,D2             
00EE0A  D440                      ADD.W     D0,D2               
00EE0C  20790000AC52              MOVEA.L   $0000AC52,A0         L000A
00EE12  D0C2                      ADDA.W    D2,A0               
00EE14  23C80000EA7C              MOVE.L    A0,$0000EA7C         L0244
00EE1A  33F90000EA520000EA78      MOVE.W    $0000EA52,$0000EA78  L0235 L0242
00EE24  33F90000EA500000EA76      MOVE.W    $0000EA50,$0000EA76  L0234 L0241
00EE2E  303900017240              MOVE.W    $00017240,D0         L0718
00EE34  6B000008                  BMI       8(PC)                __L0260
00EE38  447900017240              NEG.W     $00017240            L0718
00EE3E  C1FC00A0            __L0260:
                                  MULS      #$A0,D0             
00EE42  33C00000EA84              MOVE.W    D0,$0000EA84         L0246
00EE48  4E75                      RTS                           
00EE4A  20790000EA56        __L0261:
                                  MOVEA.L   $0000EA56,A0         L0237
00EE50  22790000EA5A              MOVEA.L   $0000EA5A,A1         L0238
00EE56  34790000EA50              MOVEA.W   $0000EA50,A2         L0234
00EE5C  264A                      MOVEA.L   A2,A3               
00EE5E  3C390000EA52              MOVE.W    $0000EA52,D6         L0235
00EE64  3E390000EA54              MOVE.W    $0000EA54,D7         L0236
00EE6A  B1C9                      CMPA.L    A1,A0               
00EE6C  6D000014                  BLT       20(PC)               __L0262
00EE70  D1F90000AC4E              ADDA.L    $0000AC4E,A0         L0009
00EE76  D3F90000EA62              ADDA.L    $0000EA62,A1         L023A
00EE7C  6100F056                  BSR       -4010(PC)            L01F5
00EE80  4E75                      RTS                           
00EE82  D1F90000AC4E        __L0262:
                                  ADDA.L    $0000AC4E,A0         L0009
00EE88  D3F90000EA62              ADDA.L    $0000EA62,A1         L023A
00EE8E  BC7C0012                  CMP.W     #$12,D6             
00EE92  67000018                  BEQ       24(PC)               __L0264
00EE96  3606                      MOVE.W    D6,D3               
00EE98  2320                L0263:MOVE.L    -(A0),-(A1)         
00EE9A  2320                      MOVE.L    -(A0),-(A1)         
00EE9C  51CEFFFA                  DBF       D6,-6(PC)            L0263
00EEA0  90CA                      SUBA.W    A2,A0               
00EEA2  92CB                      SUBA.W    A3,A1               
00EEA4  3C03                      MOVE.W    D3,D6               
00EEA6  51CFFFF0                  DBF       D7,-16(PC)           L0263
00EEAA  4E75                      RTS                           
00EEAC  94FC0140            __L0264:
                                  SUBA.W    #$140,A2            
00EEB0  96FC0140                  SUBA.W    #$140,A3            
00EEB4  3007                      MOVE.W    D7,D0               
00EEB6  C0FC00A0                  MULU      #$A0,D0             
00EEBA  90C0                      SUBA.W    D0,A0               
00EEBC  92C0                      SUBA.W    D0,A1               
00EEBE  41E8FFD8            __L0265:
                                  LEA       -40(A0),A0          
00EEC2  4CD0707F                  MOVEM.L   (A0),A4-A6/D0-D6    
00EEC6  48E1FE0E                  MOVEM.L   A4-A6/D0-D6,-(A1)   
00EECA  41E8FFD8                  LEA       -40(A0),A0          
00EECE  4CD0707F                  MOVEM.L   (A0),A4-A6/D0-D6    
00EED2  48E1FE0E                  MOVEM.L   A4-A6/D0-D6,-(A1)   
00EED6  41E8FFD8                  LEA       -40(A0),A0          
00EEDA  4CD0707F                  MOVEM.L   (A0),A4-A6/D0-D6    
00EEDE  48E1FE0E                  MOVEM.L   A4-A6/D0-D6,-(A1)   
00EEE2  41E8FFE0                  LEA       -32(A0),A0          
00EEE6  4CD0107F                  MOVEM.L   (A0),A4/D0-D6       
00EEEA  48E1FE08                  MOVEM.L   A4/D0-D6,-(A1)      
00EEEE  90CA                      SUBA.W    A2,A0               
00EEF0  92CB                      SUBA.W    A3,A1               
00EEF2  51CFFFCA                  DBF       D7,-54(PC)           __L0265
00EEF6  4E75                      RTS                           
00EEF8  3C390000EA68        __L0266:
                                  MOVE.W    $0000EA68,D6         L023C
00EEFE  3E390000EA6A              MOVE.W    $0000EA6A,D7         __L023D
00EF04  20790000EA6C              MOVEA.L   $0000EA6C,A0         L023E
00EF0A  22790000EA70              MOVEA.L   $0000EA70,A1         L023F
00EF10  D3F90000EA62              ADDA.L    $0000EA62,A1         L023A
00EF16  34790000EA66              MOVEA.W   $0000EA66,A2         L023B
00EF1C  364A                      MOVEA.W   A2,A3               
00EF1E  6100EFB4                  BSR       -4172(PC)            L01F5
00EF22  20790000EA6C              MOVEA.L   $0000EA6C,A0         L023E
00EF28  D0F90000EA74              ADDA.W    $0000EA74,A0         L0240
00EF2E  23C80000EA6C              MOVE.L    A0,$0000EA6C         L023E
00EF34  4E75                      RTS                           
00EF36  3C390000EA78        __L0267:
                                  MOVE.W    $0000EA78,D6         L0242
00EF3C  3E390000EA7A              MOVE.W    $0000EA7A,D7         __L0243
00EF42  20790000EA7C              MOVEA.L   $0000EA7C,A0         L0244
00EF48  22790000EA80              MOVEA.L   $0000EA80,A1         L0245
00EF4E  D3F90000EA62              ADDA.L    $0000EA62,A1         L023A
00EF54  34790000EA76              MOVEA.W   $0000EA76,A2         __L0241
00EF5A  364A                      MOVEA.W   A2,A3               
00EF5C  6100EF76                  BSR       -4234(PC)            L01F5
00EF60  20790000EA7C              MOVEA.L   $0000EA7C,A0         L0244
00EF66  D0F90000EA84              ADDA.W    $0000EA84,A0         L0246
00EF6C  23C80000EA7C              MOVE.L    A0,$0000EA7C         L0244
00EF72  4E75                      RTS                           
00EF74  48E7FFFE            __L0268:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00EF78  20790000AC4E              MOVEA.L   $0000AC4E,A0         L0009
00EF7E  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
00EF84  3E3C00C7                  MOVE.W    #$C7,D7             
00EF88  3C3C0013                  MOVE.W    #$13,D6             
00EF8C  247C00000000              MOVEA.L   #0,A2               
00EF92  264A                      MOVEA.L   A2,A3               
00EF94  6100EF3E                  BSR       -4290(PC)            L01F5
00EF98  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00EF9C  4E75                      RTS                           
00EF9E                      L0269:DCB.W     2,0 
00EFA2  42A7                __L026A:
                                  CLR.L     -(A7)               
00EFA4  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00EFA8  4E41                      TRAP      #1     				 ; GEMDOS                  
00EFAA  DFFC00000006              ADDA.L    #6,A7               
00EFB0  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00EFB6  23F90000008C0000EF9E      MOVE.L    $8C.L,$0000EF9E      L0269
00EFC0  23FC0000F25C0000008C      MOVE.L    #$0000F25C,$8C.L     __L0284
00EFCA  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00EFD0  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00EFD4  4E41                      TRAP      #1     				 ; GEMDOS                  
00EFD6  DFFC00000006              ADDA.L    #6,A7               
00EFDC  42790000FE28              CLR.W     $0000FE28            L02E1
00EFE2  23FC0000FEA60000FE2A      MOVE.L    #$0000FEA6,$0000FE2A L02E3 L02E2
00EFEC  4E75                      RTS                           
00EFEE  4EB90000F0C4        __L026B:
                                  JSR       $0000F0C4            L0271
00EFF4  4EB90000F19C              JSR       $0000F19C            L027A
00EFFA  4EB90000F22C              JSR       $0000F22C            __L0281
00F000  42A7                      CLR.L     -(A7)               
00F002  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00F006  4E41                      TRAP      #1     				 ; GEMDOS                  
00F008  DFFC00000006              ADDA.L    #6,A7               
00F00E  23C00001713C              MOVE.L    D0,$0001713C         L06BF
00F014  23F90000EF9E0000008C      MOVE.L    $0000EF9E,$8C.L      L0269
00F01E  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
00F024  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
00F028  4E41                      TRAP      #1     				 ; GEMDOS                  
00F02A  DFFC00000006              ADDA.L    #6,A7               
00F030  4E75                      RTS                           
00F032  48E7FFFE            L026C:MOVEM.L   A0-A6/D0-D7,-(A7)   
00F036  4240                      CLR.W     D0                  
00F038  10390000AC83              MOVE.B    $0000AC83,D0         L001B
00F03E  33C00000F268              MOVE.W    D0,$0000F268         L0285
00F044  10390001719C              MOVE.B    $0001719C,D0         L06E2
00F04A  B03900017D05              CMP.B     $00017D05,D0         L0725
00F050  6D00006C                  BLT       108(PC)              L0270
00F054  B03900017D15              CMP.B     $00017D15,D0         L0727
00F05A  6D000062                  BLT       98(PC)               L0270
00F05E  B03900017D25              CMP.B     $00017D25,D0         L0729
00F064  6D000058                  BLT       88(PC)               L0270
00F068  B039000171DB              CMP.B     $000171DB,D0         L06F6
00F06E  6D00004E                  BLT       78(PC)               L0270
00F072  13C0000171DB              MOVE.B    D0,$000171DB         L06F6
00F078  4282                      CLR.L     D2                  
00F07A  1439000171DD              MOVE.B    $000171DD,D2         L06F7
00F080  B43C0001                  CMP.B     #1,D2               
00F084  6D00000A                  BLT       10(PC)               __L026D
00F088  B43C0014                  CMP.B     #$14,D2             
00F08C  6F000008                  BLE       8(PC)                __L026E
00F090  243C0000000A        __L026D:
                                  MOVE.L    #$A,D2              
00F096  3039000171DE        __L026E:
                                  MOVE.W    $000171DE,D0         L06F8
00F09C  13FC0001000171DA          MOVE.B    #1,$000171DA         L06F5
00F0A4  383CFFFF                  MOVE.W    #$FFFF,D4           
00F0A8  4A390000AC83              TST.B     $0000AC83            L001B
00F0AE  67000006                  BEQ       6(PC)                __L026F
00F0B2  383C0001                  MOVE.W    #1,D4               
00F0B6  41F90000F364        __L026F:
                                  LEA       $0000F364,A0         L0291
00F0BC  4E43                      TRAP      #3                  
00F0BE  4CDF7FFF            L0270:MOVEM.L   (A7)+,A0-A6/D0-D7   
00F0C2  4E75                      RTS                           
00F0C4  48E78080            L0271:MOVEM.L   A0/D0,-(A7)         
00F0C8  303CFFFF                  MOVE.W    #$FFFF,D0           
00F0CC  41F90000F732              LEA       $0000F732,A0         L02B6
00F0D2  4E43                      TRAP      #3                  
00F0D4  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
00F0D8  4E75                      RTS                           
00F0DA  4E75                      RTS                           
00F0DC  48E7FFFE            __L0272:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00F0E0  0241007F                  ANDI.W    #$7F,D1             
00F0E4  4441                      NEG.W     D1                  
00F0E6  0641007F                  ADDI.W    #$7F,D1             
00F0EA  E849                      LSR.W     #4,D1               
00F0EC  02410007                  ANDI.W    #7,D1               
00F0F0  0240007F                  ANDI.W    #$7F,D0             
00F0F4  E748                      LSL.W     #3,D0               
00F0F6  2449                      MOVEA.L   A1,A2               
00F0F8  2248                      MOVEA.L   A0,A1               
00F0FA  4242                      CLR.W     D2                  
00F0FC  3639000171D8              MOVE.W    $000171D8,D3         L06F4
00F102  67000018                  BEQ       24(PC)               L0274
00F106  4A390000AC83              TST.B     $0000AC83            L001B
00F10C  6700000A                  BEQ       10(PC)               __L0273
00F110  04430001                  SUBI.W    #1,D3               
00F114  60000006                  BRA       6(PC)                L0274
00F118  363C0001            __L0273:
                                  MOVE.W    #1,D3               
00F11C  13FC0001000171DA    L0274:MOVE.B    #1,$000171DA         L06F5
00F124  4A39000170FC              TST.B     $000170FC            L069E
00F12A  67000014                  BEQ       20(PC)               __L0275
00F12E  40E7                      MOVE      SR,-(A7)            
00F130  007C0700                  ORI.W     #$700,SR            
00F134  4EB90000F4F6              JSR       $0000F4F6            L02A3
00F13A  46DF                      MOVE      (A7)+,SR            
00F13C  60000010                  BRA       16(PC)               __L0276
00F140  43F9000147E2        __L0275:
                                  LEA       $000147E2,A1         L0516
00F146  41F90000F4F6              LEA       $0000F4F6,A0         L02A3
00F14C  4E43                      TRAP      #3                  
00F14E  4CDF7FFF            __L0276:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00F152  4E75                      RTS                           
00F154  48E7E0E0            L0277:MOVEM.L   A0-A2/D0-D2,-(A7)   
00F158  0241007F                  ANDI.W    #$7F,D1             
00F15C  4441                      NEG.W     D1                  
00F15E  0641007F                  ADDI.W    #$7F,D1             
00F162  E849                      LSR.W     #4,D1               
00F164  02410007                  ANDI.W    #7,D1               
00F168  0240007F                  ANDI.W    #$7F,D0             
00F16C  E748                      LSL.W     #3,D0               
00F16E  2449                      MOVEA.L   A1,A2               
00F170  2248                      MOVEA.L   A0,A1               
00F172  13FC0001000171DA          MOVE.B    #1,$000171DA         L06F5
00F17A  4A39000170FC              TST.B     $000170FC            L069E
00F180  6700000E                  BEQ       14(PC)               __L0278
00F184  40E7                      MOVE      SR,-(A7)            
00F186  007C0700                  ORI.W     #$700,SR            
00F18A  46DF                      MOVE      (A7)+,SR            
00F18C  60000008                  BRA       8(PC)                __L0279
00F190  43F9000147E2        __L0278:
                                  LEA       $000147E2,A1         L0516
00F196  4CDF0707            __L0279:
                                  MOVEM.L   (A7)+,A0-A2/D0-D2   
00F19A  4E75                      RTS                           
00F19C  48E78080            L027A:MOVEM.L   A0/D0,-(A7)         
00F1A0  303C0001                  MOVE.W    #1,D0               
00F1A4  4A39000170FC              TST.B     $000170FC            L069E
00F1AA  67000014                  BEQ       20(PC)               __L027B
00F1AE  40E7                      MOVE      SR,-(A7)            
00F1B0  007C0700                  ORI.W     #$700,SR            
00F1B4  4EB90000F732              JSR       $0000F732            L02B6
00F1BA  46DF                      MOVE      (A7)+,SR            
00F1BC  6000000A                  BRA       10(PC)               __L027C
00F1C0  41F90000F732        __L027B:
                                  LEA       $0000F732,A0         L02B6
00F1C6  4E43                      TRAP      #3                  
00F1C8  4CDF0101            __L027C:
                                  MOVEM.L   (A7)+,A0/D0         
00F1CC  4E75                      RTS                           
00F1CE  48E78080            __L027D:
                                  MOVEM.L   A0/D0,-(A7)         
00F1D2  0240007F                  ANDI.W    #$7F,D0             
00F1D6  4440                      NEG.W     D0                  
00F1D8  0640007F                  ADDI.W    #$7F,D0             
00F1DC  E848                      LSR.W     #4,D0               
00F1DE  02400007                  ANDI.W    #7,D0               
00F1E2  4A39000170FC              TST.B     $000170FC            L069E
00F1E8  67000014                  BEQ       20(PC)               __L027E
00F1EC  40E7                      MOVE      SR,-(A7)            
00F1EE  007C0700                  ORI.W     #$700,SR            
00F1F2  4EB90000F314              JSR       $0000F314            __L028D
00F1F8  46DF                      MOVE      (A7)+,SR            
00F1FA  6000000A                  BRA       10(PC)               __L027F
00F1FE  41F90000F314        __L027E:
                                  LEA       $0000F314,A0         L028D
00F204  4E43                      TRAP      #3                  
00F206  4CDF0101            __L027F:
                                  MOVEM.L   (A7)+,A0/D0         
00F20A  4E75                      RTS                           
00F20C  0240007F            __L0280:
                                  ANDI.W    #$7F,D0             
00F210  E748                      LSL.W     #3,D0               
00F212  33C00000F7DC              MOVE.W    D0,$0000F7DC         L02BF
00F218  4E75                      RTS                           
00F21A  48E780C0                  MOVEM.L   A0-A1/D0,-(A7)      
00F21E  41F90000F7C4              LEA       $0000F7C4,A0         __L02BD
00F224  4E43                      TRAP      #3                  
00F226  4CDF0301                  MOVEM.L   (A7)+,A0-A1/D0      
00F22A  4E75                      RTS                           
00F22C  48E780C0            __L0281:
                                  MOVEM.L   A0-A1/D0,-(A7)      
00F230  41F90000F23E              LEA       $0000F23E,A0         __L0282
00F236  4E43                      TRAP      #3                  
00F238  4CDF0301                  MOVEM.L   (A7)+,A0-A1/D0      
00F23C  4E75                      RTS                           
00F23E  48E700C0            __L0282:
                                  MOVEM.L   A0-A1,-(A7)         
00F242  43F900FF8800              LEA       $FF8800.L,A1         ; YM2149 Sound Chip - Read data/Register select
00F248  41FA0294                  LEA       660(PC),A0           __L02A2
00F24C  2018                __L0283:
                                  MOVE.L    (A0)+,D0            
00F24E  01C90000                  MOVEP.L   D0,0(A1)            
00F252  6600FFF8                  BNE       -8(PC)               __L0283
00F256  4CDF0300                  MOVEM.L   (A7)+,A0-A1         
00F25A  4E75                      RTS                           
00F25C  46FC2700            __L0284:
                                  MOVE      #$2700,SR           
00F260  2F08                      MOVE.L    A0,-(A7)            
00F262  4E90                      JSR       (A0)                
00F264  205F                      MOVEA.L   (A7)+,A0            
00F266  4E73                      RTE                           
00F268  0000                L0285:DC.B      $00,$00 
00F26A                      __L0286:
                                  DCB.W     2,0 
00F26E  0000                L0287:DC.B      $00,$00 
00F270  46FC2700            L0288:MOVE      #$2700,SR           
00F274  61000464                  BSR       1124(PC)             L02B4
00F278  46FC2500                  MOVE      #$2500,SR           
00F27C  48E7E0E2                  MOVEM.L   A6/A0-A2/D0-D2,-(A7)
00F280  41FA008C                  LEA       140(PC),A0           __L028C
00F284  E248                      LSR.W     #1,D0               
00F286  30300000                  MOVE.W    0(A0,D0.W),D0       
00F28A  41FA007C                  LEA       124(PC),A0           __L028B
00F28E  48C0                      EXT.L     D0                  
00F290  80FC0064                  DIVU      #$64,D0             
00F294  06400030                  ADDI.W    #$30,D0             
00F298  10C0                      MOVE.B    D0,(A0)+            
00F29A  4840                      SWAP      D0                  
00F29C  48C0                      EXT.L     D0                  
00F29E  80FC000A                  DIVU      #$A,D0              
00F2A2  06400030                  ADDI.W    #$30,D0             
00F2A6  10C0                      MOVE.B    D0,(A0)+            
00F2A8  4840                      SWAP      D0                  
00F2AA  48C0                      EXT.L     D0                  
00F2AC  06400030                  ADDI.W    #$30,D0             
00F2B0  1080                      MOVE.B    D0,(A0)             
00F2B2  04B9000000460000090C      SUBI.L    #$46,$90C.L         
00F2BC  4DFA002C                  LEA       44(PC),A6            __L028A
00F2C0  101E                __L0289:
                                  MOVE.B    (A6)+,D0            
00F2C2  4880                      EXT.W     D0                  
00F2C4  3F00                      MOVE.W    D0,-(A7)            
00F2C6  3F3C0002                  MOVE.W    #2,-(A7)             ; Console, VT-52
00F2CA  3F3C0003                  MOVE.W    #3,-(A7)             ; Bconout - Bios console output
00F2CE  4E4D                      TRAP      #$D                  ; BIOS                 
00F2D0  5C4F                      ADDQ.W    #6,A7               
00F2D2  4A16                      TST.B     (A6)                
00F2D4  6600FFEA                  BNE       -22(PC)              __L0289
00F2D8  4CDF4707                  MOVEM.L   (A7)+,A6/A0-A2/D0-D2
00F2DC  06B9000000460000090C      ADDI.L    #$46,$90C.L         
00F2E6  6000058A                  BRA       1418(PC)             L02CD
00F2EA  496C206D616E7175    __L028A:
                                  DC.B      $49,$6C,$20,$6D,$61,$6E,$71,$75 
00F2F2  65206C27696E7374          DC.B      $65,$20,$6C,$27,$69,$6E,$73,$74 
00F2FA  72756D656E74206E          DC.B      $72,$75,$6D,$65,$6E,$74,$20,$6E 
00F302  756D65726F20              DC.B      $75,$6D,$65,$72,$6F,$20 
00F308  5858580D0A00        __L028B:
                                  DC.B      $58,$58,$58,$0D,$0A,$00 
00F30E  000000010002        __L028C:
                                  DC.B      $00,$00,$00,$01,$00,$02 
00F314  41FA04D6            __L028D:
                                  LEA       1238(PC),A0          L02C4
00F318  3140FFF6                  MOVE.W    D0,-10(A0)          
00F31C  4A790000F268              TST.W     $0000F268            L0285
00F322  66000014                  BNE       20(PC)               L028E
00F326  4A10                      TST.B     (A0)                
00F328  6C00000E                  BGE       14(PC)               L028E
00F32C  5440                      ADDQ.W    #2,D0               
00F32E  B07C0002                  CMP.W     #2,D0               
00F332  66000004                  BNE       4(PC)                L028E
00F336  5240                      ADDQ.W    #1,D0               
00F338  4A790000F268        L028E:TST.W     $0000F268            L0285
00F33E  660C                      BNE.S     12(PC)               __L028F
00F340  D040                      ADD.W     D0,D0               
00F342  33FB000A000100C6          MOVE.W    10(PC,D0.W),$000100C6  __L0290 L030A
00F34A  4E75                      RTS                           
00F34C  4E75                __L028F:
                                  RTS                           
00F34E  E74B                __L0290:
                                  LSL.W     #3,D3               
00F350  E54B                      LSL.W     #2,D3               
00F352  D643                      ADD.W     D3,D3               
00F354  4E71                      NOP                           
00F356  4243                      CLR.W     D3                  
00F358  4243                      CLR.W     D3                  
00F35A  4243                      CLR.W     D3                  
00F35C  4243                      CLR.W     D3                  
00F35E  4243                      CLR.W     D3                  
00F360  4243                      CLR.W     D3                  
00F362  4243                      CLR.W     D3                  
00F364  4A44                L0291:TST.W     D4                  
00F366  60000002                  BRA       2(PC)                __L0292
00F36A  4A790000F268        __L0292:
                                  TST.W     $0000F268            L0285
00F370  6700001E                  BEQ       30(PC)               __L0293
00F374  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
00F378  303C0000                  MOVE.W    #0,D0               
00F37C  42790000F7E2              CLR.W     $0000F7E2            L02C1
00F382  6100FFB4                  BSR       -76(PC)              L028E
00F386  42790000F268              CLR.W     $0000F268            L0285
00F38C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
00F390  4A390000F7EC        __L0293:
                                  TST.B     $0000F7EC            L02C4
00F396  6700006E                  BEQ       110(PC)              L0295
00F39A  4A390000FEA8              TST.B     $0000FEA8            L02E4
00F3A0  6A000064                  BPL       100(PC)              L0295
00F3A4  3F00                      MOVE.W    D0,-(A7)            
00F3A6  1039000171DB              MOVE.B    $000171DB,D0         L06F6
00F3AC  B0390000FEA8              CMP.B     $0000FEA8,D0         L02E4
00F3B2  66000008                  BNE       8(PC)                __L0294
00F3B6  301F                      MOVE.W    (A7)+,D0            
00F3B8  6000004C                  BRA       76(PC)               L0295
00F3BC  301F                __L0294:
                                  MOVE.W    (A7)+,D0            
00F3BE  0C7900060000FE28          CMPI.W    #6,$0000FE28         L02E1
00F3C6  6C00003E                  BGE       62(PC)               L0295
00F3CA  2F08                      MOVE.L    A0,-(A7)            
00F3CC  20790000FE2A              MOVEA.L   $0000FE2A,A0         L02E2
00F3D2  213900000258              MOVE.L    $258.L,-(A0)        
00F3D8  21390000FEAC              MOVE.L    $0000FEAC,-(A0)      L02E6
00F3DE  21390000FEB0              MOVE.L    $0000FEB0,-(A0)      L02E7
00F3E4  31390000F4BA              MOVE.W    $0000F4BA,-(A0)      L029D
00F3EA  4260                      CLR.W     -(A0)               
00F3EC  31390000FEAA              MOVE.W    $0000FEAA,-(A0)      L02E5
00F3F2  31390000FEA8              MOVE.W    $0000FEA8,-(A0)      L02E4
00F3F8  23C80000FE2A              MOVE.L    A0,$0000FE2A         L02E2
00F3FE  52790000FE28              ADDQ.W    #1,$0000FE28         L02E1
00F404  205F                      MOVEA.L   (A7)+,A0            
00F406  42790000F268        L0295:CLR.W     $0000F268            L0285
00F40C  33C00000FEAA              MOVE.W    D0,$0000FEAA         L02E5
00F412  13F9000171DB0000FEA8      MOVE.B    $000171DB,$0000FEA8  L06F6 L02E4
00F41C  21C90258                  MOVE.L    A1,$258.W           
00F420  23C90000FEB0              MOVE.L    A1,$0000FEB0         L02E7
00F426  D3C1                      ADDA.L    D1,A1               
00F428  23C90000FEAC              MOVE.L    A1,$0000FEAC         L02E6
00F42E  41FA03BC                  LEA       956(PC),A0           L02C4
00F432  4A50                      TST.W     (A0)                
00F434  6B000044                  BMI       68(PC)               __L0299
00F438  66000024                  BNE       36(PC)               __L0296
00F43C  10BC00FF                  MOVE.B    #$FF,(A0)           
00F440  61000244                  BSR       580(PC)              L02B1
00F444  5342                      SUBQ.W    #1,D2               
00F446  E34A                      LSL.W     #1,D2               
00F448  33FB20480000F4BA          MOVE.W    72(PC,D2.W),$0000F4BA  L029A L029D
00F450  23FC0001011C00000134      MOVE.L    #$0001011C,$134.L    L0313
00F45A  6000014C                  BRA       332(PC)              L02A8
00F45E  303A0382            __L0296:
                                  MOVE.W    898(PC),D0           L02C1
00F462  5440                      ADDQ.W    #2,D0               
00F464  B07C0002                  CMP.W     #2,D0               
00F468  66000004                  BNE       4(PC)                __L0297
00F46C  5240                      ADDQ.W    #1,D0               
00F46E  6100FEC8            __L0297:
                                  BSR       -312(PC)             L028E
00F472  317C2A7808DC              MOVE.W    #$2A78,2268(A0)     ;(#"*x")
00F478  4E75                __L0298:
                                  RTS                           
00F47A  4A280001            __L0299:
                                  TST.B     1(A0)               
00F47E  6600FFF8                  BNE       -8(PC)               __L0298
00F482  5342                      SUBQ.W    #1,D2               
00F484  E34A                      LSL.W     #1,D2               
00F486  33FB200A0000F4BA          MOVE.W    10(PC,D2.W),$0000F4BA  L029A L029D
00F48E  60000140                  BRA       320(PC)              L02AA
00F492  F5027B02CD019A01    L029A:DC.B      $F5,$02,$7B,$02,$CD,$01,$9A,$01 
00F49A  0506290258010505          DC.B      $05,$06,$29,$02,$58,$01,$05,$05 
00F4A2  4401                      DC.B      $44,$01 
00F4A4  0405380133012F01    __L029B:
                                  DC.B      $04,$05,$38,$01,$33,$01,$2F,$01 
00F4AC  2C01290126012401    L029C:DC.B      $2C,$01,$29,$01,$26,$01,$24,$01 
00F4B4  220120011F01              DC.B      $22,$01,$20,$01,$1F,$01 
00F4BA  00                  L029D:DC.B      $00 
00F4BB  00                  __L029E:
                                  DC.B      $00 
00F4BC  46FC25004EF9        __L029F:
                                  DC.B      $46,$FC,$25,$00,$4E,$F9 
00F4C2  00FC281C            __L02A0:
                                  DC.B      $00,$FC,$28,$1C 
00F4C6  0000010002000300    __L02A1:
                                  DC.B      $00,$00,$01,$00,$02,$00,$03,$00 
00F4CE  0400050007FF0808          DC.B      $04,$00,$05,$00,$07,$FF,$08,$08 
00F4D6  09080A0800000000          DC.B      $09,$08,$0A,$08,$00,$00,$00,$00 
00F4DE  0000010002000300    __L02A2:
                                  DC.B      $00,$00,$01,$00,$02,$00,$03,$00 
00F4E6  0400050007FF0808          DC.B      $04,$00,$05,$00,$07,$FF,$08,$08 
00F4EE  09080A0800000000          DC.B      $09,$08,$0A,$08,$00,$00,$00,$00 
00F4F6  42790000F268        L02A3:CLR.W     $0000F268            L0285
00F4FC  4A43                      TST.W     D3                  
00F4FE  6612                      BNE.S     18(PC)               __L02A4
00F500  42790000F26E              CLR.W     $0000F26E            L0287
00F506  23FC0000FA280000F26A      MOVE.L    #$0000FA28,$0000F26A __L02DF L0286
00F510  6012                      BRA.S     18(PC)               __L02A5
00F512  33FC00010000F26E    __L02A4:
                                  MOVE.W    #1,$0000F26E         L0287
00F51A  23FC0000FC280000F26A      MOVE.L    #$0000FC28,$0000F26A __L02E0 L0286
00F524  41FA02B2            __L02A5:
                                  LEA       690(PC),A0           L02BE
00F528  20C9                      MOVE.L    A1,(A0)+            
00F52A  30C0                      MOVE.W    D0,(A0)+            
00F52C  20CA                      MOVE.L    A2,(A0)+            
00F52E  30C1                      MOVE.W    D1,(A0)+            
00F530  30C4                      MOVE.W    D4,(A0)+            
00F532  30C5                      MOVE.W    D5,(A0)+            
00F534  42B8025C                  CLR.L     $25C.W              
00F538  42B80260                  CLR.L     $260.W              
00F53C  42B80264                  CLR.L     $264.W              
00F540  41FA02AA                  LEA       682(PC),A0           L02C4
00F544  4A280001                  TST.B     1(A0)               
00F548  660000C0                  BNE       192(PC)              __L02AC
00F54C  4A10                      TST.B     (A0)                
00F54E  660000D6                  BNE       214(PC)              __L02AE
00F552  61000116                  BSR       278(PC)              __L02B0
00F556  317C601608DC              MOVE.W    #$6016,2268(A0)     
00F55C  117C00FF0001              MOVE.B    #$FF,1(A0)          
00F562  61000122                  BSR       290(PC)              L02B1
00F566  23FC0001008A00000134      MOVE.L    #$0001008A,$134.L    L0300
00F570  23FC0000F7EE00000114      MOVE.L    #$0000F7EE,$114.L    L02C5
00F57A  4A790000F26E              TST.W     $0000F26E            L0287
00F580  660C                      BNE.S     12(PC)               __L02A6
00F582  33F90000F4A40000F4BA      MOVE.W    $0000F4A4,$0000F4BA  __L029B L029D
00F58C  600A                      BRA.S     10(PC)               __L02A7
00F58E  33F90000F4AC0000F4BA__L02A6:
                                  MOVE.W    $0000F4AC,$0000F4BA  L029C L029D
00F598  303A0248            __L02A7:
                                  MOVE.W    584(PC),D0           L02C1
00F59C  6100FD9A                  BSR       -614(PC)             L028E
00F5A0  4A790000F268              TST.W     $0000F268            L0285
00F5A6  6660                      BNE.S     96(PC)               __L02AB
00F5A8  43F900FF8800        L02A8:LEA       $FF8800.L,A1         ; YM2149 Sound Chip - Read data/Register select
00F5AE  41FAFF16                  LEA       -234(PC),A0          __L02A1
00F5B2  2018                __L02A9:
                                  MOVE.L    (A0)+,D0            
00F5B4  01C90000                  MOVEP.L   D0,0(A1)            
00F5B8  6600FFF8                  BNE       -8(PC)               __L02A9
00F5BC  23F9000001180000F4C2      MOVE.L    $118.L,$0000F4C2     __L02A0
00F5C6  23FC0000F4BC00000118      MOVE.L    #$0000F4BC,$118.L    __L029F
00F5D0  41F900FFFA01        L02AA:LEA       $FFFA01.L,A0         ; MFP 68901 - Multi Function Peripheral Chip
00F5D6  303C00DF                  MOVE.W    #$DF,D0             
00F5DA  C1280012                  AND.B     D0,18(A0)           
00F5DE  C128000E                  AND.B     D0,14(A0)           
00F5E2  C128000A                  AND.B     D0,10(A0)           
00F5E6  C1280006                  AND.B     D0,6(A0)            
00F5EA  42280018                  CLR.B     24(A0)              
00F5EE  117AFECA001E              MOVE.B    -310(PC),30(A0)      L029D
00F5F4  117AFEC50018              MOVE.B    -315(PC),24(A0)      __L029E
00F5FA  002800200006              ORI.B     #$20,6(A0)          
00F600  002800200012              ORI.B     #$20,18(A0)         
00F606  4E75                      RTS                           
00F608  4E75                __L02AB:
                                  RTS                           
00F60A  303A01D6            __L02AC:
                                  MOVE.W    470(PC),D0           L02C1
00F60E  4A10                      TST.B     (A0)                
00F610  6700000E                  BEQ       14(PC)               L02AD
00F614  5440                      ADDQ.W    #2,D0               
00F616  B07C0002                  CMP.W     #2,D0               
00F61A  66000004                  BNE       4(PC)                L02AD
00F61E  5240                      ADDQ.W    #1,D0               
00F620  6100FD16            L02AD:BSR       -746(PC)             L028E
00F624  4E75                      RTS                           
00F626  61000042            __L02AE:
                                  BSR       66(PC)               L02B0
00F62A  303A01B6                  MOVE.W    438(PC),D0           L02C1
00F62E  5440                      ADDQ.W    #2,D0               
00F630  B07C0002                  CMP.W     #2,D0               
00F634  66000004                  BNE       4(PC)                __L02AF
00F638  5240                      ADDQ.W    #1,D0               
00F63A  6100FCFC            __L02AF:
                                  BSR       -772(PC)             L028E
00F63E  317C2A7808DC              MOVE.W    #$2A78,2268(A0)     ;(#"*x")
00F644  117C00FF0001              MOVE.B    #$FF,1(A0)          
00F64A  217900000114FEEA          MOVE.L    $114.L,-278(A0)     
00F652  23FC0000F7EE00000114      MOVE.L    #$0000F7EE,$114.L    L02C5
00F65C  23FC0001008A00000134      MOVE.L    #$0001008A,$134.L    L0300
00F666  6000FF68                  BRA       -152(PC)             L02AA
00F66A  303C6010            __L02B0:
                                  MOVE.W    #$6010,D0           
00F66E  117C007F08A3              MOVE.B    #$7F,2211(A0)       ;(#"") 
00F674  314008A4                  MOVE.W    D0,2212(A0)         
00F678  314008B6                  MOVE.W    D0,2230(A0)         
00F67C  314008C8                  MOVE.W    D0,2248(A0)         
00F680  42A8078E                  CLR.L     1934(A0)            
00F684  4E75                      RTS                           
00F686  41F900FFFA01        L02B1:LEA       $FFFA01.L,A0         ; MFP 68901 - Multi Function Peripheral Chip
00F68C  43FA0038                  LEA       56(PC),A1            L02B2
00F690  12E80012                  MOVE.B    18(A0),(A1)+        
00F694  12E8000E                  MOVE.B    14(A0),(A1)+        
00F698  12E8000A                  MOVE.B    10(A0),(A1)+        
00F69C  12E80006                  MOVE.B    6(A0),(A1)+         
00F6A0  12E80018                  MOVE.B    24(A0),(A1)+        
00F6A4  12E80002                  MOVE.B    2(A0),(A1)+         
00F6A8  12F900000484              MOVE.B    $484.L,(A1)+        
00F6AE  5249                      ADDQ.W    #1,A1               
00F6B0  22F900000134              MOVE.L    $134.L,(A1)+        
00F6B6  22F900000118              MOVE.L    $118.L,(A1)+        
00F6BC  22B900000114              MOVE.L    $114.L,(A1)         
00F6C2  4E75                      RTS                           
00F6C4  1200                      DC.B      $12,$00 
00F6C6                      L02B2:DCB.W     8,0 
00F6D6                      __L02B3:
                                  DCB.W     2,0 
00F6DA  48E700C0            L02B4:MOVEM.L   A0-A1,-(A7)         
00F6DE  41F9FFFFFA01              LEA       $FFFFFA01.L,A0       ; MFP 68901 - Multi Function Peripheral Chip
00F6E4  43FAFFE0                  LEA       -32(PC),A1           L02B2
00F6E8  11590012                  MOVE.B    (A1)+,18(A0)        
00F6EC  1159000E                  MOVE.B    (A1)+,14(A0)        
00F6F0  1159000A                  MOVE.B    (A1)+,10(A0)        
00F6F4  11590006                  MOVE.B    (A1)+,6(A0)         
00F6F8  11590018                  MOVE.B    (A1)+,24(A0)        
00F6FC  11590002                  MOVE.B    (A1)+,2(A0)         
00F700  13D900000484              MOVE.B    (A1)+,$484.L        
00F706  5249                      ADDQ.W    #1,A1               
00F708  23D900000134              MOVE.L    (A1)+,$134.L        
00F70E  23D900000118              MOVE.L    (A1)+,$118.L        
00F714  23D100000114              MOVE.L    (A1),$114.L         
00F71A  4CDF0300                  MOVEM.L   (A7)+,A0-A1         
00F71E  4E75                      RTS                           
00F720  0800                      DC.B      $08,$00 
00F722  4239000171DA        L02B5:CLR.B     $000171DA            L06F5
00F728  13FC0080000171DB          MOVE.B    #$80,$000171DB       L06F6
00F730  4E75                      RTS                           
00F732  41FA00B8            L02B6:LEA       184(PC),A0           L02C4
00F736  4A50                      TST.W     (A0)                
00F738  6720                      BEQ.S     32(PC)               __L02B7
00F73A  4A40                      TST.W     D0                  
00F73C  6B00001E                  BMI       30(PC)               __L02B8
00F740  6600005C                  BNE       92(PC)               L02BA
00F744  217900000114FFD0          MOVE.L    $114.L,-48(A0)      
00F74C  217900000134FFD4          MOVE.L    $134.L,-44(A0)      
00F754  4250                      CLR.W     (A0)                
00F756  6100FF82                  BSR       -126(PC)             L02B4
00F75A  4E75                __L02B7:
                                  RTS                           
00F75C  40E7                __L02B8:
                                  MOVE      SR,-(A7)            
00F75E  007C0700                  ORI.W     #$700,SR            
00F762  42790000FE28              CLR.W     $0000FE28            L02E1
00F768  23FC0000FEA60000FE2A      MOVE.L    #$0000FEA6,$0000FE2A L02E3 L02E2
00F772  46DF                      MOVE      (A7)+,SR            
00F774  4A280001                  TST.B     1(A0)               
00F778  67000016                  BEQ       22(PC)               L02B9
00F77C  303A0064                  MOVE.W    100(PC),D0           L02C1
00F780  6100FBB6                  BSR       -1098(PC)            L028E
00F784  33FC6016000100C8          MOVE.W    #$6016,$000100C8     L030B
00F78C  4210                      CLR.B     (A0)                
00F78E  4E75                      RTS                           
00F790  6100FF48            L02B9:BSR       -184(PC)             L02B4
00F794  4250                      CLR.W     (A0)                
00F796  4EB90000F722              JSR       $0000F722            L02B5
00F79C  4E75                      RTS                           
00F79E  4A10                L02BA:TST.B     (A0)                
00F7A0  6700FFEE                  BEQ       -18(PC)              L02B9
00F7A4  42280001                  CLR.B     1(A0)               
00F7A8  23FAFF2C00000114          MOVE.L    -212(PC),$114.L      __L02B3
00F7B0  23FC0001011C00000134      MOVE.L    #$0001011C,$134.L    L0313
00F7BA  4E75                      RTS                           
00F7BC                      L02BB:DCB.W     2,0 
00F7C0                      __L02BC:
                                  DCB.W     2,0 
00F7C4  23FAFFF600000114    __L02BD:
                                  MOVE.L    -10(PC),$114.L       L02BB
00F7CC  23FAFFF200000134          MOVE.L    -14(PC),$134.L       __L02BC
00F7D4  6000FDD2                  BRA       -558(PC)             L02A8
00F7D8                      __L02BE:
                                  DCB.W     2,0 
00F7DC  FF00                L02BF:DC.B      $FF,$00 
00F7DE                      L02C0:DCB.W     2,0 
00F7E2  000000060006        L02C1:DC.B      $00,$00,$00,$06,$00,$06 
00F7E8  0000                L02C2:DC.B      $00,$00 
00F7EA  0000                L02C3:DC.B      $00,$00 
00F7EC  0000                L02C4:DC.B      $00,$00 
00F7EE  46FC2500            L02C5:MOVE      #$2500,SR           
00F7F2  3F00                      MOVE.W    D0,-(A7)            
00F7F4  303AFFE6                  MOVE.W    -26(PC),D0           L02BF
00F7F8  91790000F7E8              SUB.W     D0,$0000F7E8         L02C2
00F7FE  6312                      BLS.S     18(PC)               __L02C7
00F800  301F                L02C6:MOVE.W    (A7)+,D0            
00F802  52B9000004BA              ADDQ.L    #1,$4BA.L           
00F808  08B90005FFFFFA11          BCLR      #5,$FFFFFA11.L       ; Interrupt In-service B
00F810  4E73                      RTE                           
00F812  42790000F7E8        __L02C7:
                                  CLR.W     $0000F7E8            L02C2
00F818  48E74094                  MOVEM.L   A5/A3/A0/D1,-(A7)   
00F81C  207AFFC0                  MOVEA.L   -64(PC),A0           L02C0
00F820  4240                L02C8:CLR.W     D0                  
00F822  7200                      MOVEQ     #0,D1               
00F824  1018                      MOVE.B    (A0)+,D0            
00F826  6A20                      BPL.S     32(PC)               __L02CB
00F828  0C0000FF                  CMPI.B    #$FF,D0             
00F82C  6644                      BNE.S     68(PC)               L02CD
00F82E  1018                      MOVE.B    (A0)+,D0            
00F830  13C00000F7EA              MOVE.B    D0,$0000F7EA         L02C3
00F836  B03C00FF            __L02C9:
                                  CMP.B     #$FF,D0             
00F83A  6602                      BNE.S     2(PC)                __L02CA
00F83C  4240                      CLR.W     D0                  
00F83E  1200                __L02CA:
                                  MOVE.B    D0,D1               
00F840  E509                      LSL.B     #2,D1               
00F842  2A7B1046                  MOVEA.L   70(PC,D1.W),A5       __L02CE
00F846  4ED5                      JMP       (A5)                
00F848  5388                __L02CB:
                                  SUBQ.L    #1,A0               
00F84A  103AFF9E                  MOVE.B    -98(PC),D0           L02C3
00F84E  60E6                      BRA.S     -26(PC)              __L02C9
00F850  2079000171B8        L02CC:MOVEA.L   $000171B8,A0         L06EE
00F856  6000FFC8                  BRA       -56(PC)              L02C8
00F85A  46FC2700                  MOVE      #$2700,SR           
00F85E  41FAFF8C                  LEA       -116(PC),A0          L02C4
00F862  6100FF3A                  BSR       -198(PC)             L02BA
00F866  46FC2500                  MOVE      #$2500,SR           
00F86A  4CDF2902                  MOVEM.L   (A7)+,A5/A3/A0/D1   
00F86E  6000FF90                  BRA       -112(PC)             L02C6
00F872  04400080            L02CD:SUBI.W    #$80,D0             
00F876  13C00000F7E8              MOVE.B    D0,$0000F7E8         L02C2
00F87C  23C80000F7DE              MOVE.L    A0,$0000F7DE         L02C0
00F882  4CDF2902                  MOVEM.L   (A7)+,A5/A3/A0/D1   
00F886  6000FF78                  BRA       -136(PC)             L02C6
00F88A  0000F850            __L02CE:
                                  DC.L      $0000F850            L02CC
00F88E  0000F968                  DC.L      $0000F968            __L02DA
00F892  0000F89A                  DC.L      $0000F89A            __L02CF
00F896  0000F8BC                  DC.L      $0000F8BC            __L02D0
00F89A  1218                __L02CF:
                                  MOVE.B    (A0)+,D1            
00F89C  E749                      LSL.W     #3,D1               
00F89E  E848                      LSR.W     #4,D0               
00F8A0  267AFF36                  MOVEA.L   -202(PC),A3          __L02BE
00F8A4  4BFA0050                  LEA       80(PC),A5            L02D4
00F8A8  3BB310040000              MOVE.W    4(A3,D1.W),0(A5,D0.W) 
00F8AE  4BFA0052                  LEA       82(PC),A5            L02D5
00F8B2  2BB310000000              MOVE.L    0(A3,D1.W),0(A5,D0.W) 
00F8B8  6000FF66                  BRA       -154(PC)             L02C8
00F8BC  E848                __L02D0:
                                  LSR.W     #4,D0               
00F8BE  267B0042                  MOVEA.L   66(PC,D0.W),A3       L02D5
00F8C2  B6FC0000                  CMPA.W    #0,A3               
00F8C6  6700F9A8                  BEQ       -1624(PC)            L0288
00F8CA  222BFFFC                  MOVE.L    -4(A3),D1           
00F8CE  6702                      BEQ.S     2(PC)                __L02D1
00F8D0  D28B                      ADD.L     A3,D1               
00F8D2  4BF900000240        __L02D1:
                                  LEA       $240.L,A5           
00F8D8  2B81001C                  MOVE.L    D1,28(A5,D0.W)      
00F8DC  267B000A                  MOVEA.L   10(PC,D0.W),A3       L02D2
00F8E0  36BC42B8                  MOVE.W    #$42B8,(A3)         
00F8E4  6000FF3A                  BRA       -198(PC)             L02C8
00F8E8  00010020            L02D2:DC.L      $00010020            L02F7
00F8EC  0001003A                  DC.L      $0001003A            __L02F9
00F8F0  00010054                  DC.L      $00010054            __L02FB
00F8F4  0000                L02D3:DC.B      $00,$00 
00F8F6                      L02D4:DCB.W     6,0 
00F902                      L02D5:DCB.W     6,0 
00F90E  5448                __L02D6:
                                  ADDQ.W    #2,A0               
00F910  E448                      LSR.W     #2,D0               
00F912  4BFA0666                  LEA       1638(PC),A5          L02EF
00F916  46FC2700                  MOVE      #$2700,SR           
00F91A  4A350000                  TST.B     0(A5,D0.W)          
00F91E  671E                      BEQ.S     30(PC)               L02D7
00F920  42350000                  CLR.B     0(A5,D0.W)          
00F924  D040                      ADD.W     D0,D0               
00F926  303B003A                  MOVE.W    58(PC,D0.W),D0       L02D9
00F92A  3BBC60100000              MOVE.W    #$6010,0(A5,D0.W)   
00F930  042D00400003              SUBI.B    #$40,3(A5)          
00F936  6B0E                      BMI.S     14(PC)               __L02D8
00F938  062D003F0115              ADDI.B    #$3F,277(A5)        
00F93E  46FC2500            L02D7:MOVE      #$2500,SR           
00F942  6000FEDC                  BRA       -292(PC)             L02C8
00F946  3B7C606400E6        __L02D8:
                                  MOVE.W    #$6064,230(A5)      ;(#"`d")
00F94C  4A2D0002                  TST.B     2(A5)               
00F950  67EC                      BEQ.S     -20(PC)              L02D7
00F952  33FC2A78000100B4          MOVE.W    #$2A78,$000100B4    ;(#"*x") L0307
00F95A  46FC2500                  MOVE      #$2500,SR           
00F95E  6000FEC0                  BRA       -320(PC)             L02C8
00F962  0116                L02D9:BTST      D0,(A6)             
00F964  0128013A                  BTST      D0,314(A0)          
00F968  E848                __L02DA:
                                  LSR.W     #4,D0               
00F96A  4A280001                  TST.B     1(A0)               
00F96E  679E                      BEQ.S     -98(PC)              __L02D6
00F970  4BFAFF76                  LEA       -138(PC),A5          L02D2
00F974  26750000                  MOVEA.L   0(A5,D0.W),A3       
00F978  36BC6002                  MOVE.W    #$6002,(A3)         
00F97C  4BFAFF84                  LEA       -124(PC),A5          L02D5
00F980  26750000                  MOVEA.L   0(A5,D0.W),A3       
00F984  B6FC0000                  CMPA.W    #0,A3               
00F988  6700F8E6                  BEQ       -1818(PC)            L0288
00F98C  4BFAFF68                  LEA       -152(PC),A5          L02D4
00F990  33F500000000F8F4          MOVE.W    0(A5,D0.W),$0000F8F4 L02D3
00F998  D040                      ADD.W     D0,D0               
00F99A  4BF900000240              LEA       $240.L,A5           
00F9A0  2B8B0000                  MOVE.L    A3,0(A5,D0.W)       
00F9A4  E248                      LSR.W     #1,D0               
00F9A6  222BFFF8                  MOVE.L    -8(A3),D1           
00F9AA  6702                      BEQ.S     2(PC)                __L02DB
00F9AC  D28B                      ADD.L     A3,D1               
00F9AE  2B81001C            __L02DB:
                                  MOVE.L    D1,28(A5,D0.W)      
00F9B2  4241                      CLR.W     D1                  
00F9B4  1218                      MOVE.B    (A0)+,D1            
00F9B6  5248                      ADDQ.W    #1,A0               
00F9B8  D2790000F8F4              ADD.W     $0000F8F4,D1         L02D3
00F9BE  E549                      LSL.W     #2,D1               
00F9C0  2A7B005A                  MOVEA.L   90(PC,D0.W),A5       __L02DE
00F9C4  267AF8A4                  MOVEA.L   -1884(PC),A3         __L0286
00F9C8  2AB31000                  MOVE.L    0(A3,D1.W),(A5)     
00F9CC  4BFA05AC                  LEA       1452(PC),A5          L02EF
00F9D0  E448                      LSR.W     #2,D0               
00F9D2  46FC2700                  MOVE      #$2700,SR           
00F9D6  4AF50000                  TAS       0(A5,D0.W)          
00F9DA  661E                      BNE.S     30(PC)               __L02DC
00F9DC  D040                      ADD.W     D0,D0               
00F9DE  303B0082                  MOVE.W    -126(PC,D0.W),D0     L02D9
00F9E2  3BBC2A780000              MOVE.W    #$2A78,0(A5,D0.W)   ;(#"*x")
00F9E8  4A2D0003                  TST.B     3(A5)               
00F9EC  6B14                      BMI.S     20(PC)               __L02DD
00F9EE  062D00400003              ADDI.B    #$40,3(A5)          
00F9F4  042D003F0115              SUBI.B    #$3F,277(A5)        
00F9FA  46FC2500            __L02DC:
                                  MOVE      #$2500,SR           
00F9FE  6000FE20                  BRA       -480(PC)             L02C8
00FA02  062D00400003        __L02DD:
                                  ADDI.B    #$40,3(A5)          
00FA08  3B7C60C0013A              MOVE.W    #$60C0,314(A5)      
00FA0E  3B7C4E7100E6              MOVE.W    #$4E71,230(A5)      ;(#"Nq")
00FA14  46FC2500                  MOVE      #$2500,SR           
00FA18  6000FE06                  BRA       -506(PC)             L02C8
00FA1C  0001009A            __L02DE:
                                  DC.L      $0001009A            L0303
00FA20  000100AC                  DC.L      $000100AC            L0306
00FA24  000100BE                  DC.L      $000100BE            L0309
00FA28  0001000000010000    __L02DF:
                                  DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA30  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA38  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA40  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA48  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA50  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FA58  000009E800000A7F          DC.B      $00,$00,$09,$E8,$00,$00,$0A,$7F 
00FA60  00000B1E00000BC8          DC.B      $00,$00,$0B,$1E,$00,$00,$0B,$C8 
00FA68  00000C7B00000D39          DC.B      $00,$00,$0C,$7B,$00,$00,$0D,$39 
00FA70  00000E0300000ED8          DC.B      $00,$00,$0E,$03,$00,$00,$0E,$D8 
00FA78  00000FBA000010AA          DC.B      $00,$00,$0F,$BA,$00,$00,$10,$AA 
00FA80  000011A7000012B4          DC.B      $00,$00,$11,$A7,$00,$00,$12,$B4 
00FA88  000013D0000014FF          DC.B      $00,$00,$13,$D0,$00,$00,$14,$FF 
00FA90  0000163E00001791          DC.B      $00,$00,$16,$3E,$00,$00,$17,$91 
00FA98  000018F700001A73          DC.B      $00,$00,$18,$F7,$00,$00,$1A,$73 
00FAA0  00001C0700001DB1          DC.B      $00,$00,$1C,$07,$00,$00,$1D,$B1 
00FAA8  00001F7500002155          DC.B      $00,$00,$1F,$75,$00,$00,$21,$55 
00FAB0  0000234F00002569          DC.B      $00,$00,$23,$4F,$00,$00,$25,$69 
00FAB8  000027A2000029FE          DC.B      $00,$00,$27,$A2,$00,$00,$29,$FE 
00FAC0  00002C7D00002F22          DC.B      $00,$00,$2C,$7D,$00,$00,$2F,$22 
00FAC8  000031F0000034E7          DC.B      $00,$00,$31,$F0,$00,$00,$34,$E7 
00FAD0  0000380E00003B63          DC.B      $00,$00,$38,$0E,$00,$00,$3B,$63 
00FAD8  00003EEB000042AA          DC.B      $00,$00,$3E,$EB,$00,$00,$42,$AA 
00FAE0  0000469F00004AD2          DC.B      $00,$00,$46,$9F,$00,$00,$4A,$D2 
00FAE8  00004F45000053FC          DC.B      $00,$00,$4F,$45,$00,$00,$53,$FC 
00FAF0  000058FB00005E45          DC.B      $00,$00,$58,$FB,$00,$00,$5E,$45 
00FAF8  000063E0000069D0          DC.B      $00,$00,$63,$E0,$00,$00,$69,$D0 
00FB00  0000701C000076C6          DC.B      $00,$00,$70,$1C,$00,$00,$76,$C6 
00FB08  00007DD600008555          DC.B      $00,$00,$7D,$D6,$00,$00,$85,$55 
00FB10  00008D3F000095A5          DC.B      $00,$00,$8D,$3F,$00,$00,$95,$A5 
00FB18  00009E8B0000A7F9          DC.B      $00,$00,$9E,$8B,$00,$00,$A7,$F9 
00FB20  0000B1F60000BC8A          DC.B      $00,$00,$B1,$F6,$00,$00,$BC,$8A 
00FB28  0000C7C10000D3A2          DC.B      $00,$00,$C7,$C1,$00,$00,$D3,$A2 
00FB30  0000E0380000ED8C          DC.B      $00,$00,$E0,$38,$00,$00,$ED,$8C 
00FB38  0000FBAD00010AAA          DC.B      $00,$00,$FB,$AD,$00,$01,$0A,$AA 
00FB40  00011A7E00012B4B          DC.B      $00,$01,$1A,$7E,$00,$01,$2B,$4B 
00FB48  00013D1700014FF2          DC.B      $00,$01,$3D,$17,$00,$01,$4F,$F2 
00FB50  000163EC00017915          DC.B      $00,$01,$63,$EC,$00,$01,$79,$15 
00FB58  00018F830001A744          DC.B      $00,$01,$8F,$83,$00,$01,$A7,$44 
00FB60  0001C0700001DB19          DC.B      $00,$01,$C0,$70,$00,$01,$DB,$19 
00FB68  0001F75A00021555          DC.B      $00,$01,$F7,$5A,$00,$02,$15,$55 
00FB70  000234FD00025697          DC.B      $00,$02,$34,$FD,$00,$02,$56,$97 
00FB78  00027A2E00029FE4          DC.B      $00,$02,$7A,$2E,$00,$02,$9F,$E4 
00FB80  0002C7D90002F22A          DC.B      $00,$02,$C7,$D9,$00,$02,$F2,$2A 
00FB88  00031F0600034E88          DC.B      $00,$03,$1F,$06,$00,$03,$4E,$88 
00FB90  000380E00003B632          DC.B      $00,$03,$80,$E0,$00,$03,$B6,$32 
00FB98  0003EEB400042AAA          DC.B      $00,$03,$EE,$B4,$00,$04,$2A,$AA 
00FBA0  000469FB0004AD2F          DC.B      $00,$04,$69,$FB,$00,$04,$AD,$2F 
00FBA8  0004F45C00053FC8          DC.B      $00,$04,$F4,$5C,$00,$05,$3F,$C8 
00FBB0  00058FB20005E455          DC.B      $00,$05,$8F,$B2,$00,$05,$E4,$55 
00FBB8  00063E0C00069D11          DC.B      $00,$06,$3E,$0C,$00,$06,$9D,$11 
00FBC0  000701C000076C64          DC.B      $00,$07,$01,$C0,$00,$07,$6C,$64 
00FBC8  0007DD6900085555          DC.B      $00,$07,$DD,$69,$00,$08,$55,$55 
00FBD0  0008D3F600095A5E          DC.B      $00,$08,$D3,$F6,$00,$09,$5A,$5E 
00FBD8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FBE0  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FBE8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FBF0  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FBF8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC00  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC08  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC10  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC18  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC20  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC28  0001000000010000    __L02E0:
                                  DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC30  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC38  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC40  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC48  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC50  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FC58  000006D000000738          DC.B      $00,$00,$06,$D0,$00,$00,$07,$38 
00FC60  000007A50000081A          DC.B      $00,$00,$07,$A5,$00,$00,$08,$1A 
00FC68  0000089500000918          DC.B      $00,$00,$08,$95,$00,$00,$09,$18 
00FC70  000009A300000A35          DC.B      $00,$00,$09,$A3,$00,$00,$0A,$35 
00FC78  00000AD000000B76          DC.B      $00,$00,$0A,$D0,$00,$00,$0B,$76 
00FC80  00000C2400000CDC          DC.B      $00,$00,$0C,$24,$00,$00,$0C,$DC 
00FC88  00000DA000000E70          DC.B      $00,$00,$0D,$A0,$00,$00,$0E,$70 
00FC90  00000F4C00001034          DC.B      $00,$00,$0F,$4C,$00,$00,$10,$34 
00FC98  0000112B00001230          DC.B      $00,$00,$11,$2B,$00,$00,$12,$30 
00FCA0  000013460000146B          DC.B      $00,$00,$13,$46,$00,$00,$14,$6B 
00FCA8  000015A1000016EC          DC.B      $00,$00,$15,$A1,$00,$00,$16,$EC 
00FCB0  00001848000019B9          DC.B      $00,$00,$18,$48,$00,$00,$19,$B9 
00FCB8  00001B4100001CE0          DC.B      $00,$00,$1B,$41,$00,$00,$1C,$E0 
00FCC0  00001E9800002069          DC.B      $00,$00,$1E,$98,$00,$00,$20,$69 
00FCC8  0000225700002461          DC.B      $00,$00,$22,$57,$00,$00,$24,$61 
00FCD0  0000268C000028D6          DC.B      $00,$00,$26,$8C,$00,$00,$28,$D6 
00FCD8  00002B4400002DD8          DC.B      $00,$00,$2B,$44,$00,$00,$2D,$D8 
00FCE0  0000309000003373          DC.B      $00,$00,$30,$90,$00,$00,$33,$73 
00FCE8  00003683000039C1          DC.B      $00,$00,$36,$83,$00,$00,$39,$C1 
00FCF0  00003D30000040D3          DC.B      $00,$00,$3D,$30,$00,$00,$40,$D3 
00FCF8  000044AF000048C4          DC.B      $00,$00,$44,$AF,$00,$00,$48,$C4 
00FD00  00004D18000051AD          DC.B      $00,$00,$4D,$18,$00,$00,$51,$AD 
00FD08  0000568900005BB0          DC.B      $00,$00,$56,$89,$00,$00,$5B,$B0 
00FD10  00006122000066E8          DC.B      $00,$00,$61,$22,$00,$00,$66,$E8 
00FD18  00006D0700007382          DC.B      $00,$00,$6D,$07,$00,$00,$73,$82 
00FD20  00007A61000081A7          DC.B      $00,$00,$7A,$61,$00,$00,$81,$A7 
00FD28  0000895E00009189          DC.B      $00,$00,$89,$5E,$00,$00,$91,$89 
00FD30  00009A300000A35B          DC.B      $00,$00,$9A,$30,$00,$00,$A3,$5B 
00FD38  0000AD120000B761          DC.B      $00,$00,$AD,$12,$00,$00,$B7,$61 
00FD40  0000C2440000CDD1          DC.B      $00,$00,$C2,$44,$00,$00,$CD,$D1 
00FD48  0000DA0E0000E705          DC.B      $00,$00,$DA,$0E,$00,$00,$E7,$05 
00FD50  0000F4C30001034F          DC.B      $00,$00,$F4,$C3,$00,$01,$03,$4F 
00FD58  000112BC00012312          DC.B      $00,$01,$12,$BC,$00,$01,$23,$12 
00FD60  00013461000146B6          DC.B      $00,$01,$34,$61,$00,$01,$46,$B6 
00FD68  00015A2500016EC3          DC.B      $00,$01,$5A,$25,$00,$01,$6E,$C3 
00FD70  0001848800019BA3          DC.B      $00,$01,$84,$88,$00,$01,$9B,$A3 
00FD78  0001B41C0001CE0B          DC.B      $00,$01,$B4,$1C,$00,$01,$CE,$0B 
00FD80  0001E9860002069F          DC.B      $00,$01,$E9,$86,$00,$02,$06,$9F 
00FD88  0002257800024624          DC.B      $00,$02,$25,$78,$00,$02,$46,$24 
00FD90  000268C300028D6D          DC.B      $00,$02,$68,$C3,$00,$02,$8D,$6D 
00FD98  0002B44A0002DD86          DC.B      $00,$02,$B4,$4A,$00,$02,$DD,$86 
00FDA0  0003091000033747          DC.B      $00,$03,$09,$10,$00,$03,$37,$47 
00FDA8  0003683900039C17          DC.B      $00,$03,$68,$39,$00,$03,$9C,$17 
00FDB0  0003D30C00040D3F          DC.B      $00,$03,$D3,$0C,$00,$04,$0D,$3F 
00FDB8  00044AF100048C49          DC.B      $00,$04,$4A,$F1,$00,$04,$8C,$49 
00FDC0  0004D18600051ADB          DC.B      $00,$04,$D1,$86,$00,$05,$1A,$DB 
00FDC8  000568940005BB0C          DC.B      $00,$05,$68,$94,$00,$05,$BB,$0C 
00FDD0  0006122000066E8E          DC.B      $00,$06,$12,$20,$00,$06,$6E,$8E 
00FDD8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FDE0  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FDE8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FDF0  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FDF8  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE00  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE08  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE10  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE18  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE20  0001000000010000          DC.B      $00,$01,$00,$00,$00,$01,$00,$00 
00FE28  0000                L02E1:DC.B      $00,$00 
00FE2A                      L02E2:DCB.W     62,0
00FEA6  0000                L02E3:DC.B      $00,$00 
00FEA8  0000                L02E4:DC.B      $00,$00 
00FEAA  0001                L02E5:DC.B      $00,$01 
00FEAC                      L02E6:DCB.W     2,0 
00FEB0                      L02E7:DCB.W     2,0 
00FEB4  53790000FEAA        __L02E8:
                                  SUBQ.W    #1,$0000FEAA         L02E5
00FEBA  66000070                  BNE       112(PC)              __L02EA
00FEBE  4A790000FE28              TST.W     $0000FE28            L02E1
00FEC4  67000054                  BEQ       84(PC)               __L02E9
00FEC8  53790000FE28              SUBQ.W    #1,$0000FE28         L02E1
00FECE  48E78080                  MOVEM.L   A0/D0,-(A7)         
00FED2  20790000FE2A              MOVEA.L   $0000FE2A,A0         L02E2
00FED8  33D80000FEA8              MOVE.W    (A0)+,$0000FEA8      L02E4
00FEDE  33D80000FEAA              MOVE.W    (A0)+,$0000FEAA      L02E5
00FEE4  5448                      ADDQ.W    #2,A0               
00FEE6  33D80000F4BA              MOVE.W    (A0)+,$0000F4BA      L029D
00FEEC  23D80000FEB0              MOVE.L    (A0)+,$0000FEB0      L02E7
00FEF2  23D80000FEAC              MOVE.L    (A0)+,$0000FEAC      L02E6
00FEF8  23D800000258              MOVE.L    (A0)+,$258.L        
00FEFE  23C80000FE2A              MOVE.L    A0,$0000FE2A         L02E2
00FF04  13F90000FEA8000171DB      MOVE.B    $0000FEA8,$000171DB  L02E4 L06F6
00FF0E  6100F6C0                  BSR       -2368(PC)            L02AA
00FF12  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
00FF16  60000228                  BRA       552(PC)              L0315
00FF1A  6100F7BE            __L02E9:
                                  BSR       -2114(PC)            L02B4
00FF1E  42390000F7EC              CLR.B     $0000F7EC            L02C4
00FF24  6100F7FC                  BSR       -2052(PC)            L02B5
00FF28  60000216                  BRA       534(PC)              L0315
00FF2C  6A06                __L02EA:
                                  BPL.S     6(PC)                L02EB
00FF2E  52790000FEAA              ADDQ.W    #1,$0000FEAA         L02E5
00FF34  23FAFF7A00000258    L02EB:MOVE.L    -134(PC),$258.L      L02E7
00FF3C  60000202                  BRA       514(PC)              L0315
00FF40  53790000FEAA        __L02EC:
                                  SUBQ.W    #1,$0000FEAA         L02E5
00FF46  6600001E                  BNE       30(PC)               __L02ED
00FF4A  3F00                      MOVE.W    D0,-(A7)            
00FF4C  303AF894                  MOVE.W    -1900(PC),D0         L02C1
00FF50  6100F3E6                  BSR       -3098(PC)            L028E
00FF54  301F                      MOVE.W    (A7)+,D0            
00FF56  33FC6018000100C8          MOVE.W    #$6018,$000100C8     L030B
00FF5E  06430080                  ADDI.W    #$80,D3             
00FF62  60000176                  BRA       374(PC)              L030D
00FF66  6A000008            __L02ED:
                                  BPL       8(PC)                L02EE
00FF6A  52790000FEAA              ADDQ.W    #1,$0000FEAA         L02E5
00FF70  21FAFF3E0258        L02EE:MOVE.L    -194(PC),$258.W      L02E7
00FF76  60000162                  BRA       354(PC)              L030D
00FF7A                      L02EF:DCB.W     2,0 
00FF7E  4BFA010F            __L02F0:
                                  LEA       271(PC),A5           L0301
00FF82  3B7C60100001              MOVE.W    #$6010,1(A5)        
00FF88  422DFEEB                  CLR.B     -277(A5)            
00FF8C  042D0040FEEE              SUBI.B    #$40,-274(A5)       
00FF92  6B0C                      BMI.S     12(PC)               __L02F1
00FF94  0615003F                  ADDI.B    #$3F,(A5)           
00FF98  0603003F                  ADDI.B    #$3F,D3             
00FF9C  60000104                  BRA       260(PC)              L0304
00FFA0  5203                __L02F1:
                                  ADDQ.B    #1,D3               
00FFA2  3B7C2A780025              MOVE.W    #$2A78,37(A5)       ;(#"*x")
00FFA8  3B7C6064FFD1              MOVE.W    #$6064,-47(A5)      ;(#"`d")
00FFAE  600000F2                  BRA       242(PC)              L0304
00FFB2  4BFA00DB            __L02F2:
                                  LEA       219(PC),A5           L0301
00FFB6  3B7C60100025              MOVE.W    #$6010,37(A5)       
00FFBC  422DFEED                  CLR.B     -275(A5)            
00FFC0  042D0040FEEE              SUBI.B    #$40,-274(A5)       
00FFC6  6B0C                      BMI.S     12(PC)               __L02F3
00FFC8  0603003F                  ADDI.B    #$3F,D3             
00FFCC  0615003F                  ADDI.B    #$3F,(A5)           
00FFD0  600000F4                  BRA       244(PC)              L030A
00FFD4  5203                __L02F3:
                                  ADDQ.B    #1,D3               
00FFD6  3B7C6064FFD1              MOVE.W    #$6064,-47(A5)      ;(#"`d")
00FFDC  600000E8                  BRA       232(PC)              L030A
00FFE0  4BFA00AD            __L02F4:
                                  LEA       173(PC),A5           L0301
00FFE4  3B7C60100013              MOVE.W    #$6010,19(A5)       
00FFEA  422DFEEC                  CLR.B     -276(A5)            
00FFEE  042D0040FEEE              SUBI.B    #$40,-274(A5)       
00FFF4  6B0C                      BMI.S     12(PC)               __L02F5
00FFF6  0603003F                  ADDI.B    #$3F,D3             
00FFFA  0615003F                  ADDI.B    #$3F,(A5)           
00FFFE  600000B4                  BRA       180(PC)              L0307
010002  5203                __L02F5:
                                  ADDQ.B    #1,D3               
010004  3B7C2A780025              MOVE.W    #$2A78,37(A5)       ;(#"*x")
01000A  3B7C6064FFD1              MOVE.W    #$6064,-47(A5)      ;(#"`d")
010010  600000A2                  BRA       162(PC)              L0307
010014  4AB8025C            L02F6:TST.L     $25C.W              
010018  6700FF64                  BEQ       -156(PC)             __L02F0
01001C  2A78025C                  MOVEA.L   $25C.W,A5           
010020  42B8025C            L02F7:CLR.L     $25C.W              
010024  21CD0240                  MOVE.L    A5,$240.W           
010028  D615                      ADD.B     (A5),D3             
01002A  5203                      ADDQ.B    #1,D3               
01002C  606A                      BRA.S     106(PC)              __L0302
01002E  4AB80260            __L02F8:
                                  TST.L     $260.W              
010032  6700FFAC                  BEQ       -84(PC)              __L02F4
010036  2A780260                  MOVEA.L   $260.W,A5           
01003A  42B80260            __L02F9:
                                  CLR.L     $260.W              
01003E  21CD0248                  MOVE.L    A5,$248.W           
010042  D615                      ADD.B     (A5),D3             
010044  5203                      ADDQ.B    #1,D3               
010046  6062                      BRA.S     98(PC)               __L0305
010048  4AB80264            L02FA:TST.L     $264.W              
01004C  6700FF64                  BEQ       -156(PC)             __L02F2
010050  2A780264                  MOVEA.L   $264.W,A5           
010054  42B80264            __L02FB:
                                  CLR.L     $264.W              
010058  21CD0250                  MOVE.L    A5,$250.W           
01005C  5203                      ADDQ.B    #1,D3               
01005E  D615                      ADD.B     (A5),D3             
010060  6064                      BRA.S     100(PC)              L030A
010062  6506                      BCS.S     6(PC)                __L02FD
010064  6018                      BRA.S     24(PC)               __L02FE
010066  4A15                __L02FC:
                                  TST.B     (A5)                
010068  6BDE                      BMI.S     -34(PC)              L02FA
01006A  0403003F            __L02FD:
                                  SUBI.B    #$3F,D3             
01006E  654C                      BCS.S     76(PC)               L0308
010070  163C00FF                  MOVE.B    #$FF,D3             
010074  6046                      BRA.S     70(PC)               L0308
010076  2A780250                  MOVEA.L   $250.W,A5           
01007A  D615                      ADD.B     (A5),D3             
01007C  65E8                      BCS.S     -24(PC)              __L02FC
01007E  0403003F            __L02FE:
                                  SUBI.B    #$3F,D3             
010082  6438                      BCC.S     56(PC)               L0308
010084  4203                      CLR.B     D3                  
010086  6034                      BRA.S     52(PC)               L0308
010088  608A                __L02FF:
                                  BRA.S     -118(PC)             L02F6
01008A  2F0D                L0300:MOVE.L    A5,-(A7)            
01008C  2F03                      MOVE.L    D3,-(A7)            
01008E  767F                      MOVEQ     #$7F,D3             
010090                      L0301 EQU       *-1 
010090  6010                      BRA.S     16(PC)               L0304
010092  0240D615                  ANDI.W    #$D615,D0           
010096  65F0                      BCS.S     -16(PC)              __L02FF
010098  06B8000100000242    __L0302:
                                  ADDI.L    #$10000,$242.W      
0100A0                      L0303 EQU       *-6 
0100A0  655E                      BCS.S     94(PC)               __L030F
0100A2  6010                L0304:BRA.S     16(PC)               L0307
0100A4  0248                      DC.B      $02,$48 
0100A6  D615                      ADD.B     (A5),D3             
0100A8  6584                      BCS.S     -124(PC)             __L02F8
0100AA  06B800010000024A    __L0305:
                                  ADDI.L    #$10000,$24A.W      
0100B2                      L0306 EQU       *-6 
0100B2  6552                      BCS.S     82(PC)               __L0310
0100B4  6010                L0307:BRA.S     16(PC)               L030A
0100B6  0250D615                  ANDI.W    #$D615,(A0)         
0100BA  658C                      BCS.S     -116(PC)             L02FA
0100BC  06B8000100000252    L0308:ADDI.L    #$10000,$252.W      
0100C4                      L0309 EQU       *-6 
0100C4  6546                      BCS.S     70(PC)               __L0311
0100C6  E74B                L030A:LSL.W     #3,D3               
0100C8  6016                L030B:BRA.S     22(PC)               L030E
0100CA  02583F04                  ANDI.W    #$3F04,(A0)+        
0100CE  4244                      CLR.W     D4                  
0100D0  181D                      MOVE.B    (A5)+,D4            
0100D2  673E                      BEQ.S     62(PC)               __L0312
0100D4  D644                      ADD.W     D4,D3               
0100D6  21CD0258            __L030C:
                                  MOVE.L    A5,$258.W           
0100DA  D643                L030D:ADD.W     D3,D3               
0100DC  D643                      ADD.W     D3,D3               
0100DE  381F                      MOVE.W    (A7)+,D4            
0100E0  024307F8            L030E:ANDI.W    #$7F8,D3            
0100E4  4BF88800                  LEA       $FFFF8800.W,A5       ; YM2149 Sound Chip - Read data/Register select
0100E8  2ABB3070                  MOVE.L    112(PC,D3.W),(A5)    L0318
0100EC  263B3068                  MOVE.L    104(PC,D3.W),D3      L0317
0100F0  07CD0000                  MOVEP.L   D3,0(A5)            
0100F4  261F                      MOVE.L    (A7)+,D3            
0100F6  2A5F                      MOVEA.L   (A7)+,A5            
0100F8  08B80005FA0F              BCLR      #5,$FFFFFA0F.W       ; Interrupt In-service A
0100FE  4E73                      RTE                           
010100  52780240            __L030F:
                                  ADDQ.W    #1,$240.W           
010104  609C                      BRA.S     -100(PC)             L0304
010106  52780248            __L0310:
                                  ADDQ.W    #1,$248.W           
01010A  60A8                      BRA.S     -88(PC)              L0307
01010C  52780250            __L0311:
                                  ADDQ.W    #1,$250.W           
010110  60B4                      BRA.S     -76(PC)              L030A
010112  BBFAFD98            __L0312:
                                  CMPA.L    -616(PC),A5          L02E6
010116  6BBE                      BMI.S     -66(PC)              __L030C
010118  6000FE26                  BRA       -474(PC)             __L02EC
01011C  2F0D                L0313:MOVE.L    A5,-(A7)            
01011E  2F03                      MOVE.L    D3,-(A7)            
010120  2A780258                  MOVEA.L   $258.W,A5           
010124  4243                      CLR.W     D3                  
010126  161D                      MOVE.B    (A5)+,D3            
010128  6722                      BEQ.S     34(PC)               __L0316
01012A  21CD0258            __L0314:
                                  MOVE.L    A5,$258.W           
01012E  E74B                      LSL.W     #3,D3               
010130  4BF88800                  LEA       $FFFF8800.W,A5       ; YM2149 Sound Chip - Read data/Register select
010134  2ABB3024                  MOVE.L    36(PC,D3.W),(A5)     L0318
010138  263B301C                  MOVE.L    28(PC,D3.W),D3       L0317
01013C  07CD0000                  MOVEP.L   D3,0(A5)            
010140  261F                L0315:MOVE.L    (A7)+,D3            
010142  2A5F                      MOVEA.L   (A7)+,A5            
010144  08B80005FA0F              BCLR      #5,$FFFFFA0F.W       ; Interrupt In-service A
01014A  4E73                      RTE                           
01014C  BBFAFD5E            __L0316:
                                  CMPA.L    -674(PC),A5          L02E6
010150  6BD8                      BMI.S     -40(PC)              __L0314
010152  6000FD60                  BRA       -672(PC)             __L02E8
010156  080E090D            L0317:DC.B      $08,$0E,$09,$0D 
01015A  0A0A0C0C080F0903    L0318:DC.B      $0A,$0A,$0C,$0C,$08,$0F,$09,$03 
010162  0A0A0000080F0903          DC.B      $0A,$0A,$00,$00,$08,$0F,$09,$03 
01016A  0A0A0000080F0903          DC.B      $0A,$0A,$00,$00,$08,$0F,$09,$03 
010172  0A0A0000080F0903          DC.B      $0A,$0A,$00,$00,$08,$0F,$09,$03 
01017A  0A0A0000080F0903          DC.B      $0A,$0A,$00,$00,$08,$0F,$09,$03 
010182  0A0A0000080F0903          DC.B      $0A,$0A,$00,$00,$08,$0F,$09,$03 
01018A  0A0A0000080E090D          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$0D 
010192  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
01019A  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101A2  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101AA  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101B2  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101BA  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101C2  0A0A0B0B080E090D          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0D 
0101CA  0A0A0A0A080E090D          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0D 
0101D2  0A0A0A0A080E090D          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0D 
0101DA  0A0A0A0A080E090D          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0D 
0101E2  0A0A0A0A080E090C          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0C 
0101EA  0A0A0C0C080E090D          DC.B      $0A,$0A,$0C,$0C,$08,$0E,$09,$0D 
0101F2  0A0A0000080D090D          DC.B      $0A,$0A,$00,$00,$08,$0D,$09,$0D 
0101FA  0A0A0D0D080D090D          DC.B      $0A,$0A,$0D,$0D,$08,$0D,$09,$0D 
010202  0A0A0D0D080D090D          DC.B      $0A,$0A,$0D,$0D,$08,$0D,$09,$0D 
01020A  0A0A0D0D080D090D          DC.B      $0A,$0A,$0D,$0D,$08,$0D,$09,$0D 
010212  0A0A0D0D080D090D          DC.B      $0A,$0A,$0D,$0D,$08,$0D,$09,$0D 
01021A  0A0A0D0D080D090D          DC.B      $0A,$0A,$0D,$0D,$08,$0D,$09,$0D 
010222  0A0A0D0D080E090C          DC.B      $0A,$0A,$0D,$0D,$08,$0E,$09,$0C 
01022A  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
010232  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
01023A  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
010242  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
01024A  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
010252  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
01025A  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
010262  0A0A0B0B080E090C          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0C 
01026A  0A0A0A0A080E090C          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0C 
010272  0A0A0A0A080E090C          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0C 
01027A  0A0A0A0A080E090C          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0C 
010282  0A0A0A0A080D090D          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0D 
01028A  0A0A0C0C080D090D          DC.B      $0A,$0A,$0C,$0C,$08,$0D,$09,$0D 
010292  0A0A0C0C080E090C          DC.B      $0A,$0A,$0C,$0C,$08,$0E,$09,$0C 
01029A  0A0A0909080E090C          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$0C 
0102A2  0A0A0909080E090C          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$0C 
0102AA  0A0A0505080E090C          DC.B      $0A,$0A,$05,$05,$08,$0E,$09,$0C 
0102B2  0A0A0000080E090C          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$0C 
0102BA  0A0A0000080E090B          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$0B 
0102C2  0A0A0B0B080E090B          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0B 
0102CA  0A0A0B0B080E090B          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0B 
0102D2  0A0A0B0B080E090B          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0B 
0102DA  0A0A0B0B080E090B          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0B 
0102E2  0A0A0A0A080E090B          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0B 
0102EA  0A0A0A0A080E090B          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0B 
0102F2  0A0A0A0A080D090D          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0D 
0102FA  0A0A0B0B080D090D          DC.B      $0A,$0A,$0B,$0B,$08,$0D,$09,$0D 
010302  0A0A0B0B080D090D          DC.B      $0A,$0A,$0B,$0B,$08,$0D,$09,$0D 
01030A  0A0A0B0B080E090B          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$0B 
010312  0A0A0909080E090B          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$0B 
01031A  0A0A0909080E090B          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$0B 
010322  0A0A0909080D090C          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0C 
01032A  0A0A0C0C080D090D          DC.B      $0A,$0A,$0C,$0C,$08,$0D,$09,$0D 
010332  0A0A0A0A080E090B          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$0B 
01033A  0A0A0707080E090B          DC.B      $0A,$0A,$07,$07,$08,$0E,$09,$0B 
010342  0A0A0000080E090B          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$0B 
01034A  0A0A0000080D090D          DC.B      $0A,$0A,$00,$00,$08,$0D,$09,$0D 
010352  0A0A0909080D090D          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0D 
01035A  0A0A0909080E090A          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$0A 
010362  0A0A0909080D090D          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0D 
01036A  0A0A0808080D090D          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$0D 
010372  0A0A0707080D090D          DC.B      $0A,$0A,$07,$07,$08,$0D,$09,$0D 
01037A  0A0A0404080D090D          DC.B      $0A,$0A,$04,$04,$08,$0D,$09,$0D 
010382  0A0A0000080E090A          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$0A 
01038A  0A0A0404080E0909          DC.B      $0A,$0A,$04,$04,$08,$0E,$09,$09 
010392  0A0A0909080E0909          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$09 
01039A  0A0A0909080D090C          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0C 
0103A2  0A0A0B0B080E0909          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$09 
0103AA  0A0A0808080E0909          DC.B      $0A,$0A,$08,$08,$08,$0E,$09,$09 
0103B2  0A0A0808080E0909          DC.B      $0A,$0A,$08,$08,$08,$0E,$09,$09 
0103BA  0A0A0707080E0908          DC.B      $0A,$0A,$07,$07,$08,$0E,$09,$08 
0103C2  0A0A0808080E0909          DC.B      $0A,$0A,$08,$08,$08,$0E,$09,$09 
0103CA  0A0A0101080C090C          DC.B      $0A,$0A,$01,$01,$08,$0C,$09,$0C 
0103D2  0A0A0C0C080D090C          DC.B      $0A,$0A,$0C,$0C,$08,$0D,$09,$0C 
0103DA  0A0A0A0A080E0908          DC.B      $0A,$0A,$0A,$0A,$08,$0E,$09,$08 
0103E2  0A0A0606080E0907          DC.B      $0A,$0A,$06,$06,$08,$0E,$09,$07 
0103EA  0A0A0707080E0908          DC.B      $0A,$0A,$07,$07,$08,$0E,$09,$08 
0103F2  0A0A0000080E0907          DC.B      $0A,$0A,$00,$00,$08,$0E,$09,$07 
0103FA  0A0A0505080E0906          DC.B      $0A,$0A,$05,$05,$08,$0E,$09,$06 
010402  0A0A0606080D090C          DC.B      $0A,$0A,$06,$06,$08,$0D,$09,$0C 
01040A  0A0A0909080E0905          DC.B      $0A,$0A,$09,$09,$08,$0E,$09,$05 
010412  0A0A0505080E0904          DC.B      $0A,$0A,$05,$05,$08,$0E,$09,$04 
01041A  0A0A0404080D090C          DC.B      $0A,$0A,$04,$04,$08,$0D,$09,$0C 
010422  0A0A0808080D090B          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$0B 
01042A  0A0A0B0B080E0900          DC.B      $0A,$0A,$0B,$0B,$08,$0E,$09,$00 
010432  0A0A0000080D090C          DC.B      $0A,$0A,$00,$00,$08,$0D,$09,$0C 
01043A  0A0A0606080D090C          DC.B      $0A,$0A,$06,$06,$08,$0D,$09,$0C 
010442  0A0A0505080D090C          DC.B      $0A,$0A,$05,$05,$08,$0D,$09,$0C 
01044A  0A0A0202080C090C          DC.B      $0A,$0A,$02,$02,$08,$0C,$09,$0C 
010452  0A0A0B0B080C090C          DC.B      $0A,$0A,$0B,$0B,$08,$0C,$09,$0C 
01045A  0A0A0B0B080D090B          DC.B      $0A,$0A,$0B,$0B,$08,$0D,$09,$0B 
010462  0A0A0A0A080D090B          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0B 
01046A  0A0A0A0A080D090B          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0B 
010472  0A0A0A0A080D090B          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0B 
01047A  0A0A0A0A080C090C          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$0C 
010482  0A0A0A0A080C090C          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$0C 
01048A  0A0A0A0A080C090C          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$0C 
010492  0A0A0A0A080D090B          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0B 
01049A  0A0A0909080D090B          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0B 
0104A2  0A0A0909080D090A          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0A 
0104AA  0A0A0A0A080D090A          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0A 
0104B2  0A0A0A0A080D090A          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0A 
0104BA  0A0A0A0A080C090C          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$0C 
0104C2  0A0A0909080C090C          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$0C 
0104CA  0A0A0909080C090C          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$0C 
0104D2  0A0A0909080D090B          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$0B 
0104DA  0A0A0606080C090B          DC.B      $0A,$0A,$06,$06,$08,$0C,$09,$0B 
0104E2  0A0A0B0B080C090C          DC.B      $0A,$0A,$0B,$0B,$08,$0C,$09,$0C 
0104EA  0A0A0808080D090B          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$0B 
0104F2  0A0A0000080D090B          DC.B      $0A,$0A,$00,$00,$08,$0D,$09,$0B 
0104FA  0A0A0000080C090C          DC.B      $0A,$0A,$00,$00,$08,$0C,$09,$0C 
010502  0A0A0707080C090C          DC.B      $0A,$0A,$07,$07,$08,$0C,$09,$0C 
01050A  0A0A0606080C090C          DC.B      $0A,$0A,$06,$06,$08,$0C,$09,$0C 
010512  0A0A0505080C090C          DC.B      $0A,$0A,$05,$05,$08,$0C,$09,$0C 
01051A  0A0A0303080C090C          DC.B      $0A,$0A,$03,$03,$08,$0C,$09,$0C 
010522  0A0A0101080C090B          DC.B      $0A,$0A,$01,$01,$08,$0C,$09,$0B 
01052A  0A0A0A0A080D090A          DC.B      $0A,$0A,$0A,$0A,$08,$0D,$09,$0A 
010532  0A0A0505080D090A          DC.B      $0A,$0A,$05,$05,$08,$0D,$09,$0A 
01053A  0A0A0404080D090A          DC.B      $0A,$0A,$04,$04,$08,$0D,$09,$0A 
010542  0A0A0202080D0909          DC.B      $0A,$0A,$02,$02,$08,$0D,$09,$09 
01054A  0A0A0808080D0909          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$09 
010552  0A0A0808080C090B          DC.B      $0A,$0A,$08,$08,$08,$0C,$09,$0B 
01055A  0A0A0909080C090B          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$0B 
010562  0A0A0909080D0908          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$08 
01056A  0A0A0808080B090B          DC.B      $0A,$0A,$08,$08,$08,$0B,$09,$0B 
010572  0A0A0B0B080D0909          DC.B      $0A,$0A,$0B,$0B,$08,$0D,$09,$09 
01057A  0A0A0505080C090B          DC.B      $0A,$0A,$05,$05,$08,$0C,$09,$0B 
010582  0A0A0808080D0909          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$09 
01058A  0A0A0202080D0908          DC.B      $0A,$0A,$02,$02,$08,$0D,$09,$08 
010592  0A0A0606080C090B          DC.B      $0A,$0A,$06,$06,$08,$0C,$09,$0B 
01059A  0A0A0707080D0907          DC.B      $0A,$0A,$07,$07,$08,$0D,$09,$07 
0105A2  0A0A0707080C090B          DC.B      $0A,$0A,$07,$07,$08,$0C,$09,$0B 
0105AA  0A0A0606080C090A          DC.B      $0A,$0A,$06,$06,$08,$0C,$09,$0A 
0105B2  0A0A0909080B090B          DC.B      $0A,$0A,$09,$09,$08,$0B,$09,$0B 
0105BA  0A0A0A0A080C090B          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$0B 
0105C2  0A0A0202080C090B          DC.B      $0A,$0A,$02,$02,$08,$0C,$09,$0B 
0105CA  0A0A0000080C090A          DC.B      $0A,$0A,$00,$00,$08,$0C,$09,$0A 
0105D2  0A0A0808080D0906          DC.B      $0A,$0A,$08,$08,$08,$0D,$09,$06 
0105DA  0A0A0404080D0905          DC.B      $0A,$0A,$04,$04,$08,$0D,$09,$05 
0105E2  0A0A0505080D0905          DC.B      $0A,$0A,$05,$05,$08,$0D,$09,$05 
0105EA  0A0A0404080C0909          DC.B      $0A,$0A,$04,$04,$08,$0C,$09,$09 
0105F2  0A0A0909080D0904          DC.B      $0A,$0A,$09,$09,$08,$0D,$09,$04 
0105FA  0A0A0303080B090B          DC.B      $0A,$0A,$03,$03,$08,$0B,$09,$0B 
010602  0A0A0909080C090A          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$0A 
01060A  0A0A0505080B090A          DC.B      $0A,$0A,$05,$05,$08,$0B,$09,$0A 
010612  0A0A0A0A080C0909          DC.B      $0A,$0A,$0A,$0A,$08,$0C,$09,$09 
01061A  0A0A0808080B090B          DC.B      $0A,$0A,$08,$08,$08,$0B,$09,$0B 
010622  0A0A0808080C090A          DC.B      $0A,$0A,$08,$08,$08,$0C,$09,$0A 
01062A  0A0A0000080C090A          DC.B      $0A,$0A,$00,$00,$08,$0C,$09,$0A 
010632  0A0A0000080C0909          DC.B      $0A,$0A,$00,$00,$08,$0C,$09,$09 
01063A  0A0A0707080B090B          DC.B      $0A,$0A,$07,$07,$08,$0B,$09,$0B 
010642  0A0A0707080C0909          DC.B      $0A,$0A,$07,$07,$08,$0C,$09,$09 
01064A  0A0A0606080B090B          DC.B      $0A,$0A,$06,$06,$08,$0B,$09,$0B 
010652  0A0A0606080B090A          DC.B      $0A,$0A,$06,$06,$08,$0B,$09,$0A 
01065A  0A0A0909080B090B          DC.B      $0A,$0A,$09,$09,$08,$0B,$09,$0B 
010662  0A0A0505080A090A          DC.B      $0A,$0A,$05,$05,$08,$0A,$09,$0A 
01066A  0A0A0A0A080B090B          DC.B      $0A,$0A,$0A,$0A,$08,$0B,$09,$0B 
010672  0A0A0202080B090A          DC.B      $0A,$0A,$02,$02,$08,$0B,$09,$0A 
01067A  0A0A0808080C0907          DC.B      $0A,$0A,$08,$08,$08,$0C,$09,$07 
010682  0A0A0707080C0908          DC.B      $0A,$0A,$07,$07,$08,$0C,$09,$08 
01068A  0A0A0404080C0907          DC.B      $0A,$0A,$04,$04,$08,$0C,$09,$07 
010692  0A0A0606080B0909          DC.B      $0A,$0A,$06,$06,$08,$0B,$09,$09 
01069A  0A0A0909080C0906          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$06 
0106A2  0A0A0606080A090A          DC.B      $0A,$0A,$06,$06,$08,$0A,$09,$0A 
0106AA  0A0A0909080C0907          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$07 
0106B2  0A0A0303080B090A          DC.B      $0A,$0A,$03,$03,$08,$0B,$09,$0A 
0106BA  0A0A0505080B0909          DC.B      $0A,$0A,$05,$05,$08,$0B,$09,$09 
0106C2  0A0A0808080B090A          DC.B      $0A,$0A,$08,$08,$08,$0B,$09,$0A 
0106CA  0A0A0303080A090A          DC.B      $0A,$0A,$03,$03,$08,$0A,$09,$0A 
0106D2  0A0A0808080B090A          DC.B      $0A,$0A,$08,$08,$08,$0B,$09,$0A 
0106DA  0A0A0000080B0909          DC.B      $0A,$0A,$00,$00,$08,$0B,$09,$09 
0106E2  0A0A0707080B0908          DC.B      $0A,$0A,$07,$07,$08,$0B,$09,$08 
0106EA  0A0A0808080A090A          DC.B      $0A,$0A,$08,$08,$08,$0A,$09,$0A 
0106F2  0A0A0707080A0909          DC.B      $0A,$0A,$07,$07,$08,$0A,$09,$09 
0106FA  0A0A0909080C0901          DC.B      $0A,$0A,$09,$09,$08,$0C,$09,$01 
010702  0A0A0101080A090A          DC.B      $0A,$0A,$01,$01,$08,$0A,$09,$0A 
01070A  0A0A0606080B0908          DC.B      $0A,$0A,$06,$06,$08,$0B,$09,$08 
010712  0A0A0707080A090A          DC.B      $0A,$0A,$07,$07,$08,$0A,$09,$0A 
01071A  0A0A0505080A0909          DC.B      $0A,$0A,$05,$05,$08,$0A,$09,$09 
010722  0A0A0808080A090A          DC.B      $0A,$0A,$08,$08,$08,$0A,$09,$0A 
01072A  0A0A0202080A090A          DC.B      $0A,$0A,$02,$02,$08,$0A,$09,$0A 
010732  0A0A0101080A090A          DC.B      $0A,$0A,$01,$01,$08,$0A,$09,$0A 
01073A  0A0A000008090909          DC.B      $0A,$0A,$00,$00,$08,$09,$09,$09 
010742  0A0A0909080A0908          DC.B      $0A,$0A,$09,$09,$08,$0A,$09,$08 
01074A  0A0A0808080B0908          DC.B      $0A,$0A,$08,$08,$08,$0B,$09,$08 
010752  0A0A0101080A0909          DC.B      $0A,$0A,$01,$01,$08,$0A,$09,$09 
01075A  0A0A0606080B0907          DC.B      $0A,$0A,$06,$06,$08,$0B,$09,$07 
010762  0A0A0404080A0909          DC.B      $0A,$0A,$04,$04,$08,$0A,$09,$09 
01076A  0A0A050508090909          DC.B      $0A,$0A,$05,$05,$08,$09,$09,$09 
010772  0A0A0808080A0909          DC.B      $0A,$0A,$08,$08,$08,$0A,$09,$09 
01077A  0A0A0303080A0908          DC.B      $0A,$0A,$03,$03,$08,$0A,$09,$08 
010782  0A0A0606080A0909          DC.B      $0A,$0A,$06,$06,$08,$0A,$09,$09 
01078A  0A0A000008090909          DC.B      $0A,$0A,$00,$00,$08,$09,$09,$09 
010792  0A0A070708090908          DC.B      $0A,$0A,$07,$07,$08,$09,$09,$08 
01079A  0A0A0808080A0908          DC.B      $0A,$0A,$08,$08,$08,$0A,$09,$08 
0107A2  0A0A040408090909          DC.B      $0A,$0A,$04,$04,$08,$09,$09,$09 
0107AA  0A0A0606080A0908          DC.B      $0A,$0A,$06,$06,$08,$0A,$09,$08 
0107B2  0A0A010108090909          DC.B      $0A,$0A,$01,$01,$08,$09,$09,$09 
0107BA  0A0A050508090908          DC.B      $0A,$0A,$05,$05,$08,$09,$09,$08 
0107C2  0A0A070708080908          DC.B      $0A,$0A,$07,$07,$08,$08,$09,$08 
0107CA  0A0A080808090909          DC.B      $0A,$0A,$08,$08,$08,$09,$09,$09 
0107D2  0A0A020208090908          DC.B      $0A,$0A,$02,$02,$08,$09,$09,$08 
0107DA  0A0A060608090909          DC.B      $0A,$0A,$06,$06,$08,$09,$09,$09 
0107E2  0A0A000008090907          DC.B      $0A,$0A,$00,$00,$08,$09,$09,$07 
0107EA  0A0A070708080908          DC.B      $0A,$0A,$07,$07,$08,$08,$09,$08 
0107F2  0A0A070708090907          DC.B      $0A,$0A,$07,$07,$08,$09,$09,$07 
0107FA  0A0A060608090908          DC.B      $0A,$0A,$06,$06,$08,$09,$09,$08 
010802  0A0A020208080908          DC.B      $0A,$0A,$02,$02,$08,$08,$09,$08 
01080A  0A0A060608090906          DC.B      $0A,$0A,$06,$06,$08,$09,$09,$06 
010812  0A0A060608080907          DC.B      $0A,$0A,$06,$06,$08,$08,$09,$07 
01081A  0A0A070708080908          DC.B      $0A,$0A,$07,$07,$08,$08,$09,$08 
010822  0A0A040408080907          DC.B      $0A,$0A,$04,$04,$08,$08,$09,$07 
01082A  0A0A060608080908          DC.B      $0A,$0A,$06,$06,$08,$08,$09,$08 
010832  0A0A020208070907          DC.B      $0A,$0A,$02,$02,$08,$07,$09,$07 
01083A  0A0A070708080906          DC.B      $0A,$0A,$07,$07,$08,$08,$09,$06 
010842  0A0A060608080907          DC.B      $0A,$0A,$06,$06,$08,$08,$09,$07 
01084A  0A0A040408070907          DC.B      $0A,$0A,$04,$04,$08,$07,$09,$07 
010852  0A0A060608080906          DC.B      $0A,$0A,$06,$06,$08,$08,$09,$06 
01085A  0A0A050508080906          DC.B      $0A,$0A,$05,$05,$08,$08,$09,$06 
010862  0A0A040408070906          DC.B      $0A,$0A,$04,$04,$08,$07,$09,$06 
01086A  0A0A060608070907          DC.B      $0A,$0A,$06,$06,$08,$07,$09,$07 
010872  0A0A040408080905          DC.B      $0A,$0A,$04,$04,$08,$08,$09,$05 
01087A  0A0A040408060906          DC.B      $0A,$0A,$04,$04,$08,$06,$09,$06 
010882  0A0A060608070906          DC.B      $0A,$0A,$06,$06,$08,$07,$09,$06 
01088A  0A0A040408070905          DC.B      $0A,$0A,$04,$04,$08,$07,$09,$05 
010892  0A0A050508060906          DC.B      $0A,$0A,$05,$05,$08,$06,$09,$06 
01089A  0A0A050508060906          DC.B      $0A,$0A,$05,$05,$08,$06,$09,$06 
0108A2  0A0A040408060905          DC.B      $0A,$0A,$04,$04,$08,$06,$09,$05 
0108AA  0A0A050508060906          DC.B      $0A,$0A,$05,$05,$08,$06,$09,$06 
0108B2  0A0A020208060905          DC.B      $0A,$0A,$02,$02,$08,$06,$09,$05 
0108BA  0A0A040408050905          DC.B      $0A,$0A,$04,$04,$08,$05,$09,$05 
0108C2  0A0A050508060905          DC.B      $0A,$0A,$05,$05,$08,$06,$09,$05 
0108CA  0A0A020208050905          DC.B      $0A,$0A,$02,$02,$08,$05,$09,$05 
0108D2  0A0A040408050904          DC.B      $0A,$0A,$04,$04,$08,$05,$09,$04 
0108DA  0A0A040408050905          DC.B      $0A,$0A,$04,$04,$08,$05,$09,$05 
0108E2  0A0A020208040904          DC.B      $0A,$0A,$02,$02,$08,$04,$09,$04 
0108EA  0A0A040408040904          DC.B      $0A,$0A,$04,$04,$08,$04,$09,$04 
0108F2  0A0A030308040904          DC.B      $0A,$0A,$03,$03,$08,$04,$09,$04 
0108FA  0A0A020208040903          DC.B      $0A,$0A,$02,$02,$08,$04,$09,$03 
010902  0A0A030308030903          DC.B      $0A,$0A,$03,$03,$08,$03,$09,$03 
01090A  0A0A030308030903          DC.B      $0A,$0A,$03,$03,$08,$03,$09,$03 
010912  0A0A020208030902          DC.B      $0A,$0A,$02,$02,$08,$03,$09,$02 
01091A  0A0A020208020902          DC.B      $0A,$0A,$02,$02,$08,$02,$09,$02 
010922  0A0A020208020902          DC.B      $0A,$0A,$02,$02,$08,$02,$09,$02 
01092A  0A0A010108010901          DC.B      $0A,$0A,$01,$01,$08,$01,$09,$01 
010932  0A0A010108020901          DC.B      $0A,$0A,$01,$01,$08,$02,$09,$01 
01093A  0A0A000008010901          DC.B      $0A,$0A,$00,$00,$08,$01,$09,$01 
010942  0A0A000008010900          DC.B      $0A,$0A,$00,$00,$08,$01,$09,$00 
01094A  0A0A000008000900          DC.B      $0A,$0A,$00,$00,$08,$00,$09,$00 
010952  0A0A0000                  DC.B      $0A,$0A,$00,$00 
010956  133F                L0319:DC.B      $13,$3F 
010958  41F9000170FA        __L031A:
                                  LEA       $000170FA,A0         L069D
01095E  3010                      MOVE.W    (A0),D0             
010960  6700001A                  BEQ       26(PC)               L031B
010964  6B000016                  BMI       22(PC)               L031B
010968  23FC000180540000AC7E      MOVE.L    #$00018054,$0000AC7E BUF_L072F_1028 L0019
010972  61000022                  BSR       34(PC)               __L031D
010976  4A40                      TST.W     D0                  
010978  6700000C                  BEQ       12(PC)               __L031C
01097C  41F9000170FA        L031B:LEA       $000170FA,A0         L069D
010982  4250                      CLR.W     (A0)                
010984  4E75                      RTS                           
010986  41F9000170E0        __L031C:
                                  LEA       $000170E0,A0         L_L0692
01098C  20B900017094              MOVE.L    $00017094,(A0)       L0674
010992  6000FFE8                  BRA       -24(PC)              L031B
010996  48E77FE0            __L031D:
                                  MOVEM.L   A0-A2/D1-D7,-(A7)   
01099A  42A7                      CLR.L     -(A7)               
01099C  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
0109A0  4E41                      TRAP      #1     				 ; GEMDOS                  
0109A2  DFFC00000006              ADDA.L    #6,A7               
0109A8  23C00001713C              MOVE.L    D0,$0001713C         L06BF
0109AE  4EB9000109DE              JSR       $000109DE            __L0320
0109B4  3E00                      MOVE.W    D0,D7               
0109B6  2F390001713C              MOVE.L    $0001713C,-(A7)      L06BF
0109BC  3F3C0020                  MOVE.W    #$20,-(A7)           ; Super - Switch supervisor mode
0109C0  4E41                      TRAP      #1     				 ; GEMDOS                  
0109C2  DFFC00000006              ADDA.L    #6,A7               
0109C8  3007                      MOVE.W    D7,D0               
0109CA  6700000A                  BEQ       10(PC)               __L031F
0109CE  4240                      CLR.W     D0                  
0109D0  4CDF07FE            __L031E:
                                  MOVEM.L   (A7)+,A0-A2/D1-D7   
0109D4  4E75                      RTS                           
0109D6  303C0001            __L031F:
                                  MOVE.W    #1,D0               
0109DA  6000FFF4                  BRA       -12(PC)              __L031E
0109DE  60000036            __L0320:
                                  BRA       54(PC)               L0324
0109E2  12345678            L0321:DC.B      $12,$34,$56,$78 
0109E6  535441525445522E    __L0322:
                                  DC.B      $53,$54,$41,$52,$54,$45,$52,$2E ; 'STARTER.'
0109EE  F0F1F2F3F4F5F6F7          DC.B      $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7 
0109F6  F8F9FAFBFCFDFEFF          DC.B      $F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF 
0109FE  535441525445522E    __L0323:
                                  DC.B      $53,$54,$41,$52,$54,$45,$52,$2E ; 'STARTER.'
010A06  F0F1F2F3F4F5F6F7          DC.B      $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7 
010A0E  F8F9FAFBFCFDFEFF          DC.B      $F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF 
010A16  48E77FFE            L0324:MOVEM.L   A0-A6/D1-D7,-(A7)   
010A1A  40E7                      MOVE      SR,-(A7)            
010A1C  33FCFFFF0000043E          MOVE.W    #$FFFF,$43E.L       
010A24  41F900FF8604              LEA       $FF8604.L,A0         ; FDC access/sector count
010A2A  43F900FF8800              LEA       $FF8800.L,A1         ; YM2149 Sound Chip - Read data/Register select
010A30  46FC2700                  MOVE      #$2700,SR           
010A34  12BC000E                  MOVE.B    #$E,(A1)            
010A38  1011                      MOVE.B    (A1),D0             
010A3A  024000F8                  ANDI.W    #$F8,D0             
010A3E  00400005                  ORI.W     #5,D0               
010A42  13400002                  MOVE.B    D0,2(A1)            
010A46  46FC2300                  MOVE      #$2300,SR           
010A4A  203C000022B7              MOVE.L    #$22B7,D0           
010A50  317C00820002              MOVE.W    #$82,2(A0)          
010A56  4EBA01E8                  JSR       488(PC)              L0333
010A5A  3F10                      MOVE.W    (A0),-(A7)          
010A5C  317C00860002              MOVE.W    #$86,2(A0)          
010A62  4EBA01DC                  JSR       476(PC)              L0333
010A66  30BC004F                  MOVE.W    #$4F,(A0)           
010A6A  4EBA01D4                  JSR       468(PC)              L0333
010A6E  317C00800002              MOVE.W    #$80,2(A0)          
010A74  4EBA01CA                  JSR       458(PC)              L0333
010A78  30BC0014                  MOVE.W    #$14,(A0)           
010A7C  4EBA01A2                  JSR       418(PC)              L0331
010A80  7400                      MOVEQ     #0,D2               
010A82  7602                      MOVEQ     #2,D3               
010A84  45F900017484              LEA       $00017484,A2         L071C
010A8A  4BEA0200                  LEA       512(A2),A5          
010A8E  4EBA010E            L0325:JSR       270(PC)              L032F
010A92  4EBA017C                  JSR       380(PC)              L0330
010A96  08010004                  BTST      #4,D1               
010A9A  661C                      BNE.S     28(PC)               L0326
010A9C  B88D                      CMP.L     A5,D4               
010A9E  6618                      BNE.S     24(PC)               L0326
010AA0  08010003                  BTST      #3,D1               
010AA4  6712                      BEQ.S     18(PC)               L0326
010AA6  49FAFF3E                  LEA       -194(PC),A4          __L0322
010AAA  264A                      MOVEA.L   A2,A3               
010AAC  B78C                      CMPM.L    (A4)+,(A3)+         
010AAE  66000008                  BNE       8(PC)                L0326
010AB2  B78C                      CMPM.L    (A4)+,(A3)+         
010AB4  6700000A                  BEQ       10(PC)               __L0327
010AB8  51CBFFD4            L0326:DBF       D3,-44(PC)           L0325
010ABC  600000AA                  BRA       170(PC)              L032E
010AC0  7600                __L0327:
                                  MOVEQ     #0,D3               
010AC2  5242                      ADDQ.W    #1,D2               
010AC4  4DFAFF38                  LEA       -200(PC),A6          __L0323
010AC8  4EBA00D4            __L0328:
                                  JSR       212(PC)              L032F
010ACC  4EBA0142                  JSR       322(PC)              L0330
010AD0  08010004                  BTST      #4,D1               
010AD4  6628                      BNE.S     40(PC)               L032A
010AD6  B88D                      CMP.L     A5,D4               
010AD8  6624                      BNE.S     36(PC)               L032A
010ADA  08010003                  BTST      #3,D1               
010ADE  671E                      BEQ.S     30(PC)               L032A
010AE0  264E                      MOVEA.L   A6,A3               
010AE2  284A                      MOVEA.L   A2,A4               
010AE4  7A05                      MOVEQ     #5,D5               
010AE6  221B                __L0329:
                                  MOVE.L    (A3)+,D1            
010AE8  B2AC0150                  CMP.L     336(A4),D1          
010AEC  6610                      BNE.S     16(PC)               L032A
010AEE  B2AC01A4                  CMP.L     420(A4),D1          
010AF2  660A                      BNE.S     10(PC)               L032A
010AF4  B29C                      CMP.L     (A4)+,D1            
010AF6  6606                      BNE.S     6(PC)                L032A
010AF8  51CDFFEC                  DBF       D5,-20(PC)           __L0329
010AFC  5243                      ADDQ.W    #1,D3               
010AFE  5242                L032A:ADDQ.W    #1,D2               
010B00  0C420045                  CMPI.W    #$45,D2             ;(#"E") 
010B04  6DC2                      BLT.S     -62(PC)              __L0328
010B06  0C43003E                  CMPI.W    #$3E,D3             ;(#">") 
010B0A  6D00005C                  BLT       92(PC)               L032E
010B0E  7445                      MOVEQ     #$45,D2             
010B10  7602                      MOVEQ     #2,D3               
010B12  45F900017484              LEA       $00017484,A2         L071C
010B18  4BEA0200                  LEA       512(A2),A5          
010B1C  4EBA0080                  JSR       128(PC)              __L032F
010B20  4EBA00EE                  JSR       238(PC)              L0330
010B24  08010004                  BTST      #4,D1               
010B28  663E                      BNE.S     62(PC)               L032E
010B2A  B88D                      CMP.L     A5,D4               
010B2C  663A                      BNE.S     58(PC)               L032E
010B2E  08010003                  BTST      #3,D1               
010B32  6734                      BEQ.S     52(PC)               L032E
010B34  264E                      MOVEA.L   A6,A3               
010B36  284A                      MOVEA.L   A2,A4               
010B38  7A05                      MOVEQ     #5,D5               
010B3A  221B                __L032B:
                                  MOVE.L    (A3)+,D1            
010B3C  B29C                      CMP.L     (A4)+,D1            
010B3E  6628                      BNE.S     40(PC)               L032E
010B40  51CDFFF8                  DBF       D5,-8(PC)            __L032B
010B44  284A                      MOVEA.L   A2,A4               
010B46  D9FC00000018              ADDA.L    #$18,A4             
010B4C  4245                      CLR.W     D5                  
010B4E  0C9C4E4E4E4E        __L032C:
                                  CMPI.L    #$4E4E4E4E,(A4)+    ;(#"NNNN")
010B54  6608                      BNE.S     8(PC)                __L032D
010B56  06450001                  ADDI.W    #1,D5               
010B5A  6000FFF2                  BRA       -14(PC)              __L032C
010B5E  0C450014            __L032D:
                                  CMPI.W    #$14,D5             
010B62  65000004                  BCS       4(PC)                L032E
010B66  7000                      MOVEQ     #0,D0               
010B68  45FAFE78            L032E:LEA       -392(PC),A2          L0321
010B6C  2480                      MOVE.L    D0,(A2)             
010B6E  317C00860002              MOVE.W    #$86,2(A0)          
010B74  4EBA00CA                  JSR       202(PC)              L0333
010B78  309F                      MOVE.W    (A7)+,(A0)          
010B7A  4EBA00C4                  JSR       196(PC)              L0333
010B7E  317C00800002              MOVE.W    #$80,2(A0)          
010B84  4EBA00BA                  JSR       186(PC)              L0333
010B88  30BC0014                  MOVE.W    #$14,(A0)           
010B8C  4EBA0092                  JSR       146(PC)              L0331
010B90  42790000043E              CLR.W     $43E.L              
010B96  46DF                      MOVE      (A7)+,SR            
010B98  4CDF7FFE                  MOVEM.L   (A7)+,A0-A6/D1-D7   
010B9C  4E75                      RTS                           
010B9E  220A                __L032F:
                                  MOVE.L    A2,D1               
010BA0  13C100FF860D              MOVE.B    D1,$FF860D.L        
010BA6  E059                      ROR.W     #8,D1               
010BA8  13C100FF860B              MOVE.B    D1,$FF860B.L        
010BAE  4841                      SWAP      D1                  
010BB0  13C100FF8609              MOVE.B    D1,$FF8609.L        
010BB6  317C00840002              MOVE.W    #$84,2(A0)          
010BBC  4EBA0082                  JSR       130(PC)              L0333
010BC0  3082                      MOVE.W    D2,(A0)             
010BC2  4EBA007C                  JSR       124(PC)              L0333
010BC6  317C00900002              MOVE.W    #$90,2(A0)          
010BCC  317C01900002              MOVE.W    #$190,2(A0)         
010BD2  317C00900002              MOVE.W    #$90,2(A0)          
010BD8  4EBA0066                  JSR       102(PC)              L0333
010BDC  30BC0001                  MOVE.W    #1,(A0)             
010BE0  4EBA005E                  JSR       94(PC)               L0333
010BE4  317C00800002              MOVE.W    #$80,2(A0)          
010BEA  4EBA0054                  JSR       84(PC)               L0333
010BEE  30BC0080                  MOVE.W    #$80,(A0)           
010BF2  4EBA002C                  JSR       44(PC)               L0331
010BF6  7800                      MOVEQ     #0,D4               
010BF8  183900FF8609              MOVE.B    $FF8609.L,D4        
010BFE  4844                      SWAP      D4                  
010C00  183900FF860B              MOVE.B    $FF860B.L,D4        
010C06  E14C                      LSL.W     #8,D4               
010C08  183900FF860D              MOVE.B    $FF860D.L,D4        
010C0E  4E75                      RTS                           
010C10  317C00800002        L0330:MOVE.W    #$80,2(A0)          
010C16  4EBA0028                  JSR       40(PC)               L0333
010C1A  3210                      MOVE.W    (A0),D1             
010C1C  4EFA0022                  JMP       34(PC)               L0333
010C20  4EBA001E            L0331:JSR       30(PC)               L0333
010C24  4E71                __L0332:
                                  NOP                           
010C26  4E71                      NOP                           
010C28  4E71                      NOP                           
010C2A  4E71                      NOP                           
010C2C  4E71                      NOP                           
010C2E  4E71                      NOP                           
010C30  4E71                      NOP                           
010C32  0839000500FFFA01          BTST      #5,$FFFA01.L         ; MFP 68901 - Multi Function Peripheral Chip
010C3A  6600FFE8                  BNE       -24(PC)              __L0332
010C3E  4E75                      RTS                           
010C40  7E22                L0333:MOVEQ     #$22,D7             
010C42  51CFFFFE            L0334:DBF       D7,-2(PC)            L0334
010C46  4E75                      RTS                           
010C48  4E71                      DC.B      $4E,$71 
010C4A  9362                __L0335:
                                  DC.B      $93,$62 
010C4C  0000AD99            __L0336:
                                  DC.B      $00,$00,$AD,$99 
010C50  207C00000000              MOVEA.L   #0,A0               
010C56  4A280009                  TST.B     9(A0)               
010C5A  6A000036                  BPL       54(PC)               L0337
010C5E  4A28000D                  TST.B     13(A0)              
010C62  6A00002E                  BPL       46(PC)               L0337
010C66  4A280011                  TST.B     17(A0)              
010C6A  6A000026                  BPL       38(PC)               L0337
010C6E  4A280025                  TST.B     37(A0)              
010C72  6A00001E                  BPL       30(PC)               L0337
010C76  4A280085                  TST.B     133(A0)             
010C7A  6A000016                  BPL       22(PC)               L0337
010C7E  4A2800B5                  TST.B     181(A0)             
010C82  6A00000E                  BPL       14(PC)               L0337
010C86  4A2800B9                  TST.B     185(A0)             
010C8A  6A000006                  BPL       6(PC)                L0337
010C8E  4E75                      RTS                           
010C90  145A                      DC.B      $14,$5A 
010C92  207C00FFFA01        L0337:MOVEA.L   #$FFFA01,A0          ; MFP 68901 - Multi Function Peripheral Chip
010C98  42280006                  CLR.B     6(A0)               
010C9C  42280008                  CLR.B     8(A0)               
010CA0  91C8                      SUBA.L    A0,A0               
010CA2  42A80084                  CLR.L     132(A0)             
010CA6  42A800B4                  CLR.L     180(A0)             
010CAA  42A800B8                  CLR.L     184(A0)             
010CAE  42A8042A                  CLR.L     1066(A0)            
010CB2  42A80426                  CLR.L     1062(A0)            
010CB6  42AFFFFC                  CLR.L     -4(A7)              
010CBA  42AFFFF8                  CLR.L     -8(A7)              
010CBE  429F                      CLR.L     (A7)+               
010CC0  429F                      CLR.L     (A7)+               
010CC2  429F                      CLR.L     (A7)+               
010CC4  429F                      CLR.L     (A7)+               
010CC6  429F                      CLR.L     (A7)+               
010CC8  429F                      CLR.L     (A7)+               
010CCA  429F                      CLR.L     (A7)+               
010CCC  6000C26C                  BRA       -15764(PC)           L015C
010CD0  4E75                      RTS                           

; =============================================================================
; copyright text
010CD2  CC272A6D2D6C2A73          DC.B      $CC,$27,$2A,$6D,$2D,$6C,$2A,$73 
010CDA  657571636F722A72          DC.B      $65,$75,$71,$63,$6F,$72,$2A,$72 
010CE2  61702A0056DCAB87          DC.B      $61,$70,$2A,$00,$56,$DC,$AB,$87 
010CEA  BFF40000636F7079          DC.B      $BF,$F4,$00,$00,$63,$6F,$70,$79 
010CF2  7269676874205369          DC.B      $72,$69,$67,$68,$74,$20,$53,$69 
010CFA  6C6D6172696C7320          DC.B      $6C,$6D,$61,$72,$69,$6C,$73,$20 
010D02  383873797374656D          DC.B      $38,$38,$73,$79,$73,$74,$65,$6D 
010D0A  653A20526F637175          DC.B      $65,$3A,$20,$52,$6F,$63,$71,$75 
010D12  6573204C6F756973          DC.B      $65,$73,$20,$4C,$6F,$75,$69,$73 
010D1A  2D4D617269650000          DC.B      $2D,$4D,$61,$72,$69,$65,$00,$00 


; =============================================================================
; OPCODE JUMP TABLE
; A2
; this table's address is (always ?) into a2 register
; read an opcode byte, double it into a word, add it to a2 (jump table address)
; get the 'offset' word located at this address, then jump to (a2 + offset)
010D22  045E046004620464    OPCODE_JUMP_TABLE:
                                  DC.B      $04,$5E,$04,$60,$04,$62,$04,$64 
010D2A  02E8046804760486          DC.B      $02,$E8,$04,$68,$04,$76,$04,$86 
010D32  049C04A404AE0580          DC.B      $04,$9C,$04,$A4,$04,$AE,$05,$80 
010D3A  0582058405860588          DC.B      $05,$82,$05,$84,$05,$86,$05,$88 
010D42  058A058C06240636          DC.B      $05,$8A,$05,$8C,$06,$24,$06,$36 
010D4A  064A06640676068A          DC.B      $06,$4A,$06,$64,$06,$76,$06,$8A 
010D52  06A406B606CA06E4          DC.B      $06,$A4,$06,$B6,$06,$CA,$06,$E4 
010D5A  06F6070A07240750          DC.B      $06,$F6,$07,$0A,$07,$24,$07,$50 
010D62  07560782078C078E          DC.B      $07,$56,$07,$82,$07,$8C,$07,$8E 
010D6A  092009300946097C          DC.B      $09,$20,$09,$30,$09,$46,$09,$7C 
010D72  099C2E9A09B405B4          DC.B      $09,$9C,$2E,$9A,$09,$B4,$05,$B4 
010D7A  05D605FA2EC62EFA          DC.B      $05,$D6,$05,$FA,$2E,$C6,$2E,$FA 
010D82  04BE04C805380548          DC.B      $04,$BE,$04,$C8,$05,$38,$05,$48 
010D8A  42782F2C045E045E          DC.B      $42,$78,$2F,$2C,$04,$5E,$04,$5E 
010D92  0DE80E160DE00DCC          DC.B      $0D,$E8,$0E,$16,$0D,$E0,$0D,$CC 
010D9A  0BFE0D022F562F46          DC.B      $0B,$FE,$0D,$02,$2F,$56,$2F,$46 
010DA2  0C100CA40CB40CB8          DC.B      $0C,$10,$0C,$A4,$0C,$B4,$0C,$B8 
010DAA  0CCA0CDA0D0C0E54          DC.B      $0C,$CA,$0C,$DA,$0D,$0C,$0E,$54 
010DB2  119811C2182417F4          DC.B      $11,$98,$11,$C2,$18,$24,$17,$F4 
010DBA  191C1936108C10AE          DC.B      $19,$1C,$19,$36,$10,$8C,$10,$AE 
010DC2  1858261226282794          DC.B      $18,$58,$26,$12,$26,$28,$27,$94 
010DCA  287A28B22904294A          DC.B      $28,$7A,$28,$B2,$29,$04,$29,$4A 
010DD2  29602E822E822E82          DC.B      $29,$60,$2E,$82,$2E,$82,$2E,$82 
010DDA  2E822E822E8219BC          DC.B      $2E,$82,$2E,$82,$2E,$82,$19,$BC 
010DE2  29842F723058304C          DC.B      $29,$84,$2F,$72,$30,$58,$30,$4C 
010DEA  30683060303E2E8E          DC.B      $30,$68,$30,$60,$30,$3E,$2E,$8E 
010DF2  31803278307036A0          DC.B      $31,$80,$32,$78,$30,$70,$36,$A0 
010DFA  36F637E03762307C          DC.B      $36,$F6,$37,$E0,$37,$62,$30,$7C 
010E02  3DD43E043E083E20          DC.B      $3D,$D4,$3E,$04,$3E,$08,$3E,$20 
010E0A  3E2C3E423E5A3E6C          DC.B      $3E,$2C,$3E,$42,$3E,$5A,$3E,$6C 
010E12  3E843E9C3EBC3EEC          DC.B      $3E,$84,$3E,$9C,$3E,$BC,$3E,$EC 
010E1A  3F403F703F983FAE          DC.B      $3F,$40,$3F,$70,$3F,$98,$3F,$AE 
010E22  3FC43FD8426E1554          DC.B      $3F,$C4,$3F,$D8,$42,$6E,$15,$54 
010E2A  4172419841CA4200          DC.B      $41,$72,$41,$98,$41,$CA,$42,$00 
010E32  421E42324244045E          DC.B      $42,$1E,$42,$32,$42,$44,$04,$5E 
010E3A  4342438C27D82822          DC.B      $43,$42,$43,$8C,$27,$D8,$28,$22 
010E42  1950425042642D54          DC.B      $19,$50,$42,$50,$42,$64,$2D,$54 
010E4A  2DAA3BCC3CC239F6          DC.B      $2D,$AA,$3B,$CC,$3C,$C2,$39,$F6 
010E52  39B011901182118A          DC.B      $39,$B0,$11,$90,$11,$82,$11,$8A 
010E5A  2E4E3D203DC01570          DC.B      $2E,$4E,$3D,$20,$3D,$C0,$15,$70 
010E62  15763DCC197026CA          DC.B      $15,$76,$3D,$CC,$19,$70,$26,$CA 
010E6A  270827521202121C          DC.B      $27,$08,$27,$52,$12,$02,$12,$1C 
010E72  123412481276125C          DC.B      $12,$34,$12,$48,$12,$76,$12,$5C 
010E7A  3BC026300BC00BE2          DC.B      $3B,$C0,$26,$30,$0B,$C0,$0B,$E2 
010E82  07FE07EE045E045E          DC.B      $07,$FE,$07,$EE,$04,$5E,$04,$5E 
010E8A  045E045E08EA08D8          DC.B      $04,$5E,$04,$5E,$08,$EA,$08,$D8 
010E92  090E08FC07903F84          DC.B      $09,$0E,$08,$FC,$07,$90,$3F,$84 
010E9A  357432C233B607C4          DC.B      $35,$74,$32,$C2,$33,$B6,$07,$C4 
010EA2  0E680EB40E8E0EDA          DC.B      $0E,$68,$0E,$B4,$0E,$8E,$0E,$DA 
010EAA  0F140FA40FDE155E          DC.B      $0F,$14,$0F,$A4,$0F,$DE,$15,$5E 
010EB2  0FFC0FFE2DFC2E82          DC.B      $0F,$FC,$0F,$FE,$2D,$FC,$2E,$82 
010EBA  2E823A583A4C15C2          DC.B      $2E,$82,$3A,$58,$3A,$4C,$15,$C2 
010EC2  157E35DA363E37CE          DC.B      $15,$7E,$35,$DA,$36,$3E,$37,$CE 
010ECA  483C3D1415CC3138          DC.B      $48,$3C,$3D,$14,$15,$CC,$31,$38 
010ED2  309648304836181C          DC.B      $30,$96,$48,$30,$48,$36,$18,$1C 
010EDA  185017EC0C9C1044          DC.B      $18,$50,$17,$EC,$0C,$9C,$10,$44 
010EE2  10002992584E              DC.B      $10,$00,$29,$92,$58,$4E 


; =============================================================================
; OPCODE JUMP TABLE
; A5
010EE8  422E4234423C424A    OPNAME_JUMP_TABLE:
                                  DC.B      $42,$2E,$42,$34,$42,$3C,$42,$4A 
010EF0  42584264429E427C          DC.B      $42,$58,$42,$64,$42,$9E,$42,$7C 
010EF8  428E42BA42C642D0          DC.B      $42,$8E,$42,$BA,$42,$C6,$42,$D0 
010F00  430442E642F6431E          DC.B      $43,$04,$42,$E6,$42,$F6,$43,$1E 
010F08  4332434443984362          DC.B      $43,$32,$43,$44,$43,$98,$43,$62 
010F10  437E43BE43E04400          DC.B      $43,$7E,$43,$BE,$43,$E0,$44,$00 
010F18  447E442C44564BC8          DC.B      $44,$7E,$44,$2C,$44,$56,$4B,$C8 
010F20  4BCE4BD400000000          DC.B      $4B,$CE,$4B,$D4,$00,$00,$00,$00 
010F28  4BC44532453A4542          DC.B      $4B,$C4,$45,$32,$45,$3A,$45,$42 
010F30  454A45544568457C          DC.B      $45,$4A,$45,$54,$45,$68,$45,$7C 
010F38  459045A445B845CC          DC.B      $45,$90,$45,$A4,$45,$B8,$45,$CC 
010F40  45D445DE45EC45F8          DC.B      $45,$D4,$45,$DE,$45,$EC,$45,$F8 
010F48  46004604460E4630          DC.B      $46,$00,$46,$04,$46,$0E,$46,$30 
010F50  464A464E467846A0          DC.B      $46,$4A,$46,$4E,$46,$78,$46,$A0 
010F58  46D82E3A467A46E6          DC.B      $46,$D8,$2E,$3A,$46,$7A,$46,$E6 
010F60  47D0469647DA47EE          DC.B      $47,$D0,$46,$96,$47,$DA,$47,$EE 
010F68  4820484448F64902          DC.B      $48,$20,$48,$44,$48,$F6,$49,$02 
010F70  4B8E4B0A4B204B36          DC.B      $4B,$8E,$4B,$0A,$4B,$20,$4B,$36 
010F78  4B4C4B624B784AE8          DC.B      $4B,$4C,$4B,$62,$4B,$78,$4A,$E8 
010F80  4AEE485C491C4910          DC.B      $4A,$EE,$48,$5C,$49,$1C,$49,$10 
010F88  493649EE49F846BC          DC.B      $49,$36,$49,$EE,$49,$F8,$46,$BC 
010F90  4A02                      DC.B      $4A,$02 



010F92                      STORE_JUMP_TABLE:
                                  DCB.W     3,0 
010F98  4B2E4B3A4B464B82          DC.B      $4B,$2E,$4B,$3A,$4B,$46,$4B,$82 
010FA0  4B5E4B704B9E4BA8          DC.B      $4B,$5E,$4B,$70,$4B,$9E,$4B,$A8 
010FA8  4BB24BE84BC84BD8          DC.B      $4B,$B2,$4B,$E8,$4B,$C8,$4B,$D8 
010FB0  4C024C144C264C7C          DC.B      $4C,$02,$4C,$14,$4C,$26,$4C,$7C 
010FB8  4C444C604CA24CC2          DC.B      $4C,$44,$4C,$60,$4C,$A2,$4C,$C2 
010FC0  4CE24D624D0E4D38          DC.B      $4C,$E2,$4D,$62,$4D,$0E,$4D,$38 
010FC8  4D964D9A4B2A              DC.B      $4D,$96,$4D,$9A,$4B,$2A 


; =============================================================================
; ADD_JUMP_TABLE 0x0 -  offset 0x0 -  address $010fce


; =============================================================================
; ADD_JUMP_TABLE 0x1 -  offset 0x0 -  address $010fce


; =============================================================================
; ADD_JUMP_TABLE 0x2 -  offset 0x0 -  address $010fce
010FCE                      ADD_JUMP_TABLE:
                                  DCB.W     3,0 
010FD4  4D764D824D8E4DD2          DC.B      $4D,$76,$4D,$82,$4D,$8E,$4D,$D2 
010FDC  4DAE4DC04DF64E00          DC.B      $4D,$AE,$4D,$C0,$4D,$F6,$4E,$00 
010FE4  4E0A4E484E284E38          DC.B      $4E,$0A,$4E,$48,$4E,$28,$4E,$38 
010FEC  4E6A4E7C4E8E              DC.B      $4E,$6A,$4E,$7C,$4E,$8E 
010FF2  4EEC4EB4                  DC.B 		$4E,$EC,$4E,$B4,$4E,$D0 ;JMP       20148(A4)           
010FF6  4ED0                      DC.B  	$4E,$D0 				;JMP       (A0)                
010FF8  4F1A4F3A4F5A4FE2          DC.B      $4F,$1A,$4F,$3A,$4F,$5A,$4F,$E2 
011000  4F8E4FB84D5A501E          DC.B      $4F,$8E,$4F,$B8,$4D,$5A,$50,$1E 
011008  4AEE                      DC.B      $4A,$EE 




; =============================================================================
; OPCODE 0x4 -  offset 0x2e8 -  address $01100a
; cbreakpt
; appelle "RTS", c'est tout
01100A  4EF90000AC2A              JMP       $0000AC2A            __L0002 ; RTS

; =============================================================================
; FN_TERMINATE_PROGRAM
011010  4EF90000B0E2        FN_TERMINATE_PROGRAM:
                                  JMP       $0000B0E2            SYS_TERMINATE_PROGRAM


; =============================================================================
011016  23CF0000AC2E        FN_START:
                                  MOVE.L    A7,$0000AC2E         ADDR_INITIAL_STACK
01101C  23FC00017D4400017146      MOVE.L    #$00017D44,$00017146 BUF_SCRIPT_NAME ADDR_SCRIPT_NAME
011026  23FC00017E440001714A      MOVE.L    #$00017E44,$0001714A BUF_CURRENT_PATH ADDR_CURRENT_PATH
011030  23FC00017F440001714E      MOVE.L    #$00017F44,$0001714E BUF_L072E_272 L06C4
01103A  23FC0000AD6E000171F0      MOVE.L    #$0000AD6E,$000171F0 L0026 L06FE
011044  23FC0000ACEE000171EC      MOVE.L    #$0000ACEE,$000171EC L0023 L06FD
01104E  23FC00017A8400017130      MOVE.L    #$00017A84,$00017130 L0723 L06BC
011058  23FC0001788400017172      MOVE.L    #$00017884,$00017172 L0722 L06D0
011062  4EB90000AF92              JSR       $0000AF92            FN_INIT_1
011068  4EB900016EFE              JSR       $00016EFE            FN_INIT_2
01106E  4EB900016004              JSR       $00016004            FN_INIT_3


011074  45F900010D22        __L033E:
                                  
                                  LEA       $00010D22,A2         OPCODE_JUMP_TABLE
01107A  4BF900010EE8              LEA       $00010EE8,A5         OPNAME_JUMP_TABLE
011080  4245                      CLR.W     D5                  
011082  2079000170A8              MOVEA.L   $000170A8,A0         L067A
011088  601A                      BRA.S     26(PC)               L0340

01108A  2079000170A8        L033F:
                                  MOVEA.L   $000170A8,A0         L067A
011090  3A305004                  MOVE.W    4(A0,D5.W),D5       
011094  6600000E                  BNE       14(PC)               L0340
011098  6100173A                  BSR       5946(PC)             __L03F8
01109C  6006                      BRA.S     6(PC)                L0340
01109E  4E71                      NOP                           
0110A0  6000FF6E                  BRA       -146(PC)             FN_TERMINATE_PROGRAM

0110A4  2C705000            L0340:
                                  MOVEA.L   0(A0,D5.W),A6       
0110A8  423900017188              CLR.B     $00017188            L06D9
0110AE  423900017107              CLR.B     $00017107            B_06A7
0110B4  49F900017440              LEA       $00017440,A4         L071B
0110BA  4A2EFFDC                  TST.B     -36(A6)             
0110BE  6A000038                  BPL       56(PC)               L0341
0110C2  082E0001FFDC              BTST      #1,-36(A6)          
0110C8  6600002E                  BNE       46(PC)               L0341
0110CC  206EFFEC                  MOVEA.L   -20(A6),A0          
0110D0  2268000A                  MOVEA.L   10(A0),A1           
0110D4  B3FC00000000              CMPA.L    #0,A1               
0110DA  6700001C                  BEQ       28(PC)               L0341
0110DE  47F0980A                  LEA       10(A0,A1.L),A3      
0110E2  382EFFF6                  MOVE.W    -10(A6),D4          
0110E6  33C400017144              MOVE.W    D4,$00017144         W_06C1
0110EC  6100162A                  BSR       5674(PC)             L03EF
0110F0  61000076                  BSR       118(PC)              FN_READ_OPCODE
0110F4  61001634                  BSR       5684(PC)             __L03F0
0110F8  4A2EFFFC            L0341:TST.B     -4(A6)              
0110FC  6700FF8C                  BEQ       -116(PC)             L033F
011100  6A000008                  BPL       8(PC)                __L0342
011104  1D7C0001FFFC              MOVE.B    #1,-4(A6)           
01110A  532EFFFF            __L0342:
                                  SUBQ.B    #1,-1(A6)           
01110E  6600FF7A                  BNE       -134(PC)             L033F
011112  61001604                  BSR       5636(PC)             L03EF
011116  266EFFF8                  MOVEA.L   -8(A6),A3           
01111A  382EFFF6                  MOVE.W    -10(A6),D4          
01111E  523900017107              ADDQ.B    #1,$00017107         B_06A7
011124  61000042                  BSR       66(PC)               FN_READ_OPCODE
011128  3D44FFF6                  MOVE.W    D4,-10(A6)          
01112C  2D4BFFF8                  MOVE.L    A3,-8(A6)           
011130  206EFFEC                  MOVEA.L   -20(A6),A0          
011134  22680006                  MOVEA.L   6(A0),A1            
011138  B3FC00000000              CMPA.L    #0,A1               
01113E  6700001A                  BEQ       26(PC)               __L0343
011142  423900017107              CLR.B     $00017107            B_06A7
011148  47F09806                  LEA       6(A0,A1.L),A3       
01114C  382EFFF6                  MOVE.W    -10(A6),D4          
011150  33C400017144              MOVE.W    D4,$00017144         W_06C1
011156  61000010                  BSR       16(PC)               FN_READ_OPCODE
01115A  610015CE            __L0343:
                                  BSR       5582(PC)             L03F0
01115E  1D6EFFFEFFFF              MOVE.B    -2(A6),-1(A6)       
011164  6000FF24                  BRA       -220(PC)             L033F


; =============================================================================
; Script opcodes are read here
011168  487AFFFE            FN_READ_OPCODE:
                                  PEA       -2(PC)               FN_READ_OPCODE
01116C  4240                      CLR.W     D0                  
; A2 pointe sur OPCODE_JUMP_TABLE
; A3 pointe sur l'opcode à lire dans D0 (puis est incrémenté)
; TODO: il faudrait tracer l'adresse pointée par A3 puis le byte lu dans D0
; chaque fois qu'on passe ici
; si on a l'adresse de début du script depack, on peut alors savoir quel opcode est lu
; à quelle adresse dasn le fichier script, donc savoir combien de bytes sont lus par opcode
; A priori l'adresse de début du script depack se trouve dans A1
01116E  101B                      MOVE.B    (A3)+,D0             
011170  D040                      ADD.W     D0,D0                ; build routine address from opcode
011172  30320000                  MOVE.W    0(A2,D0.W),D0        
011176  4EF20000                  JMP       0(A2,D0.W)           ; jump to matching routine
01117A  4E71                      NOP                           
01117C  6000FE92                  BRA       -366(PC)             FN_TERMINATE_PROGRAM


; =============================================================================
; OPCODE 0x0 -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0x36 -  offset 0x45e -  address $011180
; cclock

; =============================================================================
; OPCODE 0x37 -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0x8b -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0xb2 -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0xb3 -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0xb4 -  offset 0x45e -  address $011180
; cnul

; =============================================================================
; OPCODE 0xb5 -  offset 0x45e -  address $011180
; crunfilm
011180  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1 -  offset 0x460 -  address $011182
; cesc1
011182  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2 -  offset 0x462 -  address $011184
; cesc2
011184  4E75                      RTS                           


; =============================================================================
; OPCODE 0x3 -  offset 0x464 -  address $011186
; cesc3
011186  4E75                      RTS                           
011188  4E75                      RTS                           


; =============================================================================
; OPCODE 0x5 -  offset 0x468 -  address $01118a
; cjsr8
; param: 1 byte (lu dans D0)
; input: D4, A6
; cjsr8(u8 num_bytes)
01118A  101B                      MOVE.B    (A3)+,D0        ; lit le byte suivant dans D0
01118C  5944                      SUBQ.W    #4,D4           ; met l'adresse du pointeur de script (A3)     
01118E  2D8B4000                  MOVE.L    A3,0(A6,D4.W)   ;   à (A6 + D4 - 4) => adresse de retour ???
011192  4880                      EXT.W     D0              ; déplace le pointeur de script (A3)
011194  D6C0                      ADDA.W    D0,A3           ;   de D0 bytes
011196  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6 -  offset 0x476 -  address $011198
; cjsr16
; param: 2 bytes (lus dans D0)
; input: D4, A6
; cjsr16(u16 num_bytes)
011198  101B                      MOVE.B    (A3)+,D0            
01119A  E140                      ASL.W     #8,D0               
01119C  101B                      MOVE.B    (A3)+,D0
01119E  5944                      SUBQ.W    #4,D4               
0111A0  2D8B4000                  MOVE.L    A3,0(A6,D4.W)   ; (A6 + D4 - 4) => adresse de retour ???
0111A4  D6C0                      ADDA.W    D0,A3               
0111A6  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7 -  offset 0x486 -  address $0111a8
; cjsr24
; param: 3 bytes (lus dans D0)
; input: D4, A6
; cjsr24(u32 num_bytes)
0111A8  101B                      MOVE.B    (A3)+,D0      ; lit 1 byte     
0111AA  4880                      EXT.W     D0            ; extension de signe  
0111AC  4840                      SWAP      D0            ; place dans le word fort de D0      
0111AE  101B                      MOVE.B    (A3)+,D0      ; lit 2 bytes      
0111B0  E140                      ASL.W     #8,D0               
0111B2  101B                      MOVE.B    (A3)+,D0            
0111B4  5944                      SUBQ.W    #4,D4               
0111B6  2D8B4000                  MOVE.L    A3,0(A6,D4.W) ; (A6 + D4 - 4) => adresse de retour ???  
0111BA  D7C0                      ADDA.L    D0,A3               
0111BC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x8 -  offset 0x49c -  address $0111be
; cjmp8
; param: 1 byte (lu dans D0)
; cjmp8(u8 num_bytes)
0111BE  101B                      MOVE.B    (A3)+,D0     ; lit 1 byte       
0111C0  4880                      EXT.W     D0           ; extension de signe
0111C2  D6C0                      ADDA.W    D0,A3        ; déplace le pointeur de script de D0 bytes
0111C4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x9 -  offset 0x4a4 -  address $0111c6
; cjmp16
; param: 2 bytes (lu dans D0)
; cjmp16(u16 num_bytes)
0111C6  101B                      MOVE.B    (A3)+,D0            
0111C8  E140                      ASL.W     #8,D0               
0111CA  101B                      MOVE.B    (A3)+,D0            
0111CC  D6C0                      ADDA.W    D0,A3               
0111CE  4E75                      RTS                           


; =============================================================================
; OPCODE 0xa -  offset 0x4ae -  address $0111d0
; cjmp16
; param: 3 bytes (lu dans D0)
; cjmp32(u32 num_bytes)
0111D0  101B                      MOVE.B    (A3)+,D0            
0111D2  4880                      EXT.W     D0                  
0111D4  4840                      SWAP      D0                  
0111D6  101B                      MOVE.B    (A3)+,D0            
0111D8  E140                      ASL.W     #8,D0               
0111DA  101B                      MOVE.B    (A3)+,D0            
0111DC  D7C0                      ADDA.L    D0,A3               
0111DE  4E75                      RTS                           


; =============================================================================
; OPCODE 0x30 -  offset 0x4be -  address $0111e0
; cstart8
; param: 1 byte (lu dans D0)
; cstart8(u8)
0111E0  101B                      MOVE.B    (A3)+,D0            
0111E2  4880                      EXT.W     D0                  
0111E4  48C0                      EXT.L     D0                  
0111E6  6000000A                  BRA       10(PC)               OP_CSTART


; =============================================================================
; OPCODE 0x31 -  offset 0x4c8 -  address $0111ea
; cstart16
; param: 2 bytes (lu dans D0)
0111EA  101B                      MOVE.B    (A3)+,D0            
0111EC  E140                      ASL.W     #8,D0               
0111EE  101B                      MOVE.B    (A3)+,D0            
0111F0  48C0                      EXT.L     D0                  

0111F2  4A3900017107        OP_CSTART:
                                  TST.B     $00017107            B_06A7
0111F8  66000042                  BNE       66(PC)               __L0348
0111FC  4A6EFFF4                  TST.W     -12(A6)             
011200  66000020                  BNE       32(PC)               __L0346
011204  322EFFF6                  MOVE.W    -10(A6),D1          
011208  5941                      SUBQ.W    #4,D1               
01120A  3D41FFF4                  MOVE.W    D1,-12(A6)          
01120E  3D41FFF6                  MOVE.W    D1,-10(A6)          
011212  2DAEFFF81000              MOVE.L    -8(A6),0(A6,D1.W)   
011218  D08B                      ADD.L     A3,D0               
01121A  2D40FFF8                  MOVE.L    D0,-8(A6)           
01121E  6000000E                  BRA       14(PC)               __L0347
011222  3D6EFFF4FFF6        __L0346:
                                  MOVE.W    -12(A6),-10(A6)     
011228  D08B                      ADD.L     A3,D0               
01122A  2D40FFF8                  MOVE.L    D0,-8(A6)           
01122E  1D7C0001FFFC        __L0347:
                                  MOVE.B    #1,-4(A6)           
011234  1D7C0001FFFF              MOVE.B    #1,-1(A6)           
01123A  4E75                      RTS                           
01123C  4A6EFFF4            __L0348:
                                  TST.W     -12(A6)             
011240  66000010                  BNE       16(PC)               __L0349
011244  5944                      SUBQ.W    #4,D4               
011246  3D44FFF4                  MOVE.W    D4,-12(A6)          
01124A  2D8B4000                  MOVE.L    A3,0(A6,D4.W)       
01124E  D7C0                      ADDA.L    D0,A3               
011250  4E75                      RTS                           
011252  382EFFF4            __L0349:
                                  MOVE.W    -12(A6),D4          
011256  D7C0                      ADDA.L    D0,A3               
011258  4E75                      RTS                           


; =============================================================================
; OPCODE 0x32 -  offset 0x538 -  address $01125a
; cstart24
; param: 3 bytes (lu dans D0)
01125A  101B                      MOVE.B    (A3)+,D0            
01125C  4880                      EXT.W     D0                  
01125E  4840                      SWAP      D0                  
011260  101B                      MOVE.B    (A3)+,D0            
011262  E140                      ASL.W     #8,D0               
011264  101B                      MOVE.B    (A3)+,D0            
011266  6000FF8A                  BRA       -118(PC)             OP_CSTART


; =============================================================================
; OPCODE 0x33 -  offset 0x548 -  address $01126a
; cleave
01126A  4A3900017107              TST.B     $00017107            B_06A7
011270  6600001C                  BNE       28(PC)               __L034A
011274  322EFFF4                  MOVE.W    -12(A6),D1          
011278  67000034                  BEQ       52(PC)               OP_CRET
01127C  426EFFF4                  CLR.W     -12(A6)             
011280  2D761000FFF8              MOVE.L    0(A6,D1.W),-8(A6)   
011286  5841                      ADDQ.W    #4,D1               
011288  3D41FFF6                  MOVE.W    D1,-10(A6)          
01128C  4E75                      RTS                           
01128E  382EFFF4            __L034A:
                                  MOVE.W    -12(A6),D4          
011292  6700001A                  BEQ       26(PC)               OP_CRET
011296  426EFFF4                  CLR.W     -12(A6)             
01129A  26764000                  MOVEA.L   0(A6,D4.W),A3       
01129E  5844                      ADDQ.W    #4,D4               
0112A0  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb -  offset 0x580 -  address $0112a2
; cjsrabs
0112A2  4E75                      RTS                           


; =============================================================================
; OPCODE 0xc -  offset 0x582 -  address $0112a4
; cjmpabs
0112A4  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd -  offset 0x584 -  address $0112a6
; cjsrind16
0112A6  4E75                      RTS                           


; =============================================================================
; OPCODE 0xe -  offset 0x586 -  address $0112a8
; cjsrind24
0112A8  4E75                      RTS                           


; =============================================================================
; OPCODE 0xf -  offset 0x588 -  address $0112aa
; cjmpind16
0112AA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x10 -  offset 0x58a -  address $0112ac
; cjmpind24
0112AC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x11 -  offset 0x58c -  address $0112ae
; cret
0112AE  4A3900017107        OP_CRET:
                                  TST.B     $00017107            B_06A7
0112B4  6600000C                  BNE       12(PC)               __L034C
0112B8  B87900017144              CMP.W     $00017144,D4         W_06C1
0112BE  6C000716                  BGE       1814(PC)             OP_CSTOP
0112C2  26764000            __L034C:
                                  MOVEA.L   0(A6,D4.W),A3       
0112C6  B86EFFF4                  CMP.W     -12(A6),D4          
0112CA  66000006                  BNE       6(PC)                __L034D
0112CE  426EFFF4                  CLR.W     -12(A6)             
0112D2  5844                __L034D:
                                  ADDQ.W    #4,D4               
0112D4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2b -  offset 0x5b4 -  address $0112d6
; cloop8
; param: 1 byte
0112D6  23CB00017140              MOVE.L    A3,$00017140         DW_06C0
0112DC  528B                      ADDQ.L    #1,A3               
0112DE  7EFF                      MOVEQ     #$FF,D7             
0112E0  6100019A                  BSR       410(PC)              OP_CADD
0112E4  66000004                  BNE       4(PC)                __L034E
0112E8  4E75                      RTS                           
0112EA  267900017140        __L034E:
                                  MOVEA.L   $00017140,A3         DW_06C0
0112F0  101B                      MOVE.B    (A3)+,D0            
0112F2  4880                      EXT.W     D0                  
0112F4  D6C0                      ADDA.W    D0,A3               
0112F6  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2c -  offset 0x5d6 -  address $0112f8
; cloop16
; param: 2 bytes
0112F8  23CB00017140              MOVE.L    A3,$00017140         DW_06C0
0112FE  548B                      ADDQ.L    #2,A3               
011300  7EFF                      MOVEQ     #$FF,D7             
011302  61000178                  BSR       376(PC)              OP_CADD
011306  66000004                  BNE       4(PC)                __L034F
01130A  4E75                      RTS                           
01130C  267900017140        __L034F:
                                  MOVEA.L   $00017140,A3         DW_06C0
011312  101B                      MOVE.B    (A3)+,D0            
011314  E140                      ASL.W     #8,D0               
011316  101B                      MOVE.B    (A3)+,D0            
011318  D6C0                      ADDA.W    D0,A3               
01131A  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2d -  offset 0x5fa -  address $01131c
; cloop24
; param: 3 bytes
01131C  23CB00017140              MOVE.L    A3,$00017140         DW_06C0
011322  568B                      ADDQ.L    #3,A3               
011324  7EFF                      MOVEQ     #$FF,D7             
011326  61000154                  BSR       340(PC)              OP_CADD
01132A  66000004                  BNE       4(PC)                __L0350
01132E  4E75                      RTS                           
011330  267900017140        __L0350:
                                  MOVEA.L   $00017140,A3         DW_06C0
011336  101B                      MOVE.B    (A3)+,D0            
011338  4880                      EXT.W     D0                  
01133A  4840                      SWAP      D0                  
01133C  101B                      MOVE.B    (A3)+,D0            
01133E  E140                      ASL.W     #8,D0               
011340  101B                      MOVE.B    (A3)+,D0            
011342  D7C0                      ADDA.L    D0,A3               
011344  4E75                      RTS                           


; =============================================================================
; OPCODE 0x12 -  offset 0x624 -  address $011346
; cbz8
; param: 1 byte
011346  4A47                      TST.W     D7                  
011348  6600000A                  BNE       10(PC)               __L0351
01134C  101B                      MOVE.B    (A3)+,D0            
01134E  4880                      EXT.W     D0                  
011350  D6C0                      ADDA.W    D0,A3               
011352  4E75                      RTS                           
011354  528B                __L0351:
                                  ADDQ.L    #1,A3               
011356  4E75                      RTS                           


; =============================================================================
; OPCODE 0x13 -  offset 0x636 -  address $011358
; cbz16
011358  4A47                      TST.W     D7                  
01135A  6600000C                  BNE       12(PC)               __L0352
01135E  101B                      MOVE.B    (A3)+,D0            
011360  E140                      ASL.W     #8,D0               
011362  101B                      MOVE.B    (A3)+,D0            
011364  D6C0                      ADDA.W    D0,A3               
011366  4E75                      RTS                           
011368  548B                __L0352:
                                  ADDQ.L    #2,A3               
01136A  4E75                      RTS                           


; =============================================================================
; OPCODE 0x14 -  offset 0x64a -  address $01136c
; cbz24
01136C  4A47                      TST.W     D7                  
01136E  66000012                  BNE       18(PC)               __L0353
011372  101B                      MOVE.B    (A3)+,D0            
011374  4880                      EXT.W     D0                  
011376  4840                      SWAP      D0                  
011378  101B                      MOVE.B    (A3)+,D0            
01137A  E140                      ASL.W     #8,D0               
01137C  101B                      MOVE.B    (A3)+,D0            
01137E  D7C0                      ADDA.L    D0,A3               
011380  4E75                      RTS                           
011382  568B                __L0353:
                                  ADDQ.L    #3,A3               
011384  4E75                      RTS                           


; =============================================================================
; OPCODE 0x15 -  offset 0x664 -  address $011386
; cbnz8
011386  4A47                      TST.W     D7                  
011388  6700000A                  BEQ       10(PC)               __L0354
01138C  101B                      MOVE.B    (A3)+,D0            
01138E  4880                      EXT.W     D0                  
011390  D6C0                      ADDA.W    D0,A3               
011392  4E75                      RTS                           
011394  528B                __L0354:
                                  ADDQ.L    #1,A3               
011396  4E75                      RTS                           


; =============================================================================
; OPCODE 0x16 -  offset 0x676 -  address $011398
; cbnz16
011398  4A47                      TST.W     D7                  
01139A  6700000C                  BEQ       12(PC)               __L0355
01139E  101B                      MOVE.B    (A3)+,D0            
0113A0  E140                      ASL.W     #8,D0               
0113A2  101B                      MOVE.B    (A3)+,D0            
0113A4  D6C0                      ADDA.W    D0,A3               
0113A6  4E75                      RTS                           
0113A8  548B                __L0355:
                                  ADDQ.L    #2,A3               
0113AA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x17 -  offset 0x68a -  address $0113ac
; cbnz24
0113AC  4A47                      TST.W     D7                  
0113AE  67000012                  BEQ       18(PC)               __L0356
0113B2  101B                      MOVE.B    (A3)+,D0            
0113B4  4880                      EXT.W     D0                  
0113B6  4840                      SWAP      D0                  
0113B8  101B                      MOVE.B    (A3)+,D0            
0113BA  E140                      ASL.W     #8,D0               
0113BC  101B                      MOVE.B    (A3)+,D0            
0113BE  D7C0                      ADDA.L    D0,A3               
0113C0  4E75                      RTS                           
0113C2  568B                __L0356:
                                  ADDQ.L    #3,A3               
0113C4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x18 -  offset 0x6a4 -  address $0113c6
; cbeq8
0113C6  BE46                      CMP.W     D6,D7               
0113C8  6600000A                  BNE       10(PC)               __L0357
0113CC  101B                      MOVE.B    (A3)+,D0            
0113CE  4880                      EXT.W     D0                  
0113D0  D6C0                      ADDA.W    D0,A3               
0113D2  4E75                      RTS                           
0113D4  528B                __L0357:
                                  ADDQ.L    #1,A3               
0113D6  4E75                      RTS                           


; =============================================================================
; OPCODE 0x19 -  offset 0x6b6 -  address $0113d8
; cbeq16
0113D8  BE46                      CMP.W     D6,D7               
0113DA  6600000C                  BNE       12(PC)               __L0358
0113DE  101B                      MOVE.B    (A3)+,D0            
0113E0  E140                      ASL.W     #8,D0               
0113E2  101B                      MOVE.B    (A3)+,D0            
0113E4  D6C0                      ADDA.W    D0,A3               
0113E6  4E75                      RTS                           
0113E8  548B                __L0358:
                                  ADDQ.L    #2,A3               
0113EA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1a -  offset 0x6ca -  address $0113ec
; cbeq24
0113EC  BC47                      CMP.W     D7,D6               
0113EE  66000012                  BNE       18(PC)               __L0359
0113F2  101B                      MOVE.B    (A3)+,D0            
0113F4  4880                      EXT.W     D0                  
0113F6  4840                      SWAP      D0                  
0113F8  101B                      MOVE.B    (A3)+,D0            
0113FA  E140                      ASL.W     #8,D0               
0113FC  101B                      MOVE.B    (A3)+,D0            
0113FE  D7C0                      ADDA.L    D0,A3               
011400  4E75                      RTS                           
011402  568B                __L0359:
                                  ADDQ.L    #3,A3               
011404  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1b -  offset 0x6e4 -  address $011406
; cbne8
011406  BE46                      CMP.W     D6,D7               
011408  6700000A                  BEQ       10(PC)               __L035A
01140C  101B                      MOVE.B    (A3)+,D0            
01140E  4880                      EXT.W     D0                  
011410  D6C0                      ADDA.W    D0,A3               
011412  4E75                      RTS                           
011414  524B                __L035A:
                                  ADDQ.W    #1,A3               
011416  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1c -  offset 0x6f6 -  address $011418
; cbne16
011418  BE46                      CMP.W     D6,D7               
01141A  6700000C                  BEQ       12(PC)               __L035B
01141E  101B                      MOVE.B    (A3)+,D0            
011420  E140                      ASL.W     #8,D0               
011422  101B                      MOVE.B    (A3)+,D0            
011424  D6C0                      ADDA.W    D0,A3               
011426  4E75                      RTS                           
011428  548B                __L035B:
                                  ADDQ.L    #2,A3               
01142A  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1d -  offset 0x70a -  address $01142c
; cbne24
01142C  BE46                      CMP.W     D6,D7               
01142E  67000012                  BEQ       18(PC)               __L035C
011432  101B                      MOVE.B    (A3)+,D0            
011434  4880                      EXT.W     D0                  
011436  4840                      SWAP      D0                  
011438  101B                      MOVE.B    (A3)+,D0            
01143A  E140                      ASL.W     #8,D0               
01143C  101B                      MOVE.B    (A3)+,D0            
01143E  D7C0                      ADDA.L    D0,A3               
011440  4E75                      RTS                           
011442  568B                __L035C:
                                  ADDQ.L    #3,A3               
011444  4E75                      RTS                           


; =============================================================================
; OPCODE 0x1e -  offset 0x724 -  address $011446
; cstore
; param: 2+ bytes
; lit un opname et saute
; puis lit un STORE et saute
011446  61003CA6                  BSR       15526(PC)            FN_READ_OPNAME_SAVE_D7
01144A  207900017146        L035D:MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
011450  23F90001714E00017146      MOVE.L    $0001714E,$00017146  L06C4 ADDR_SCRIPT_NAME
01145A  23C80001714E              MOVE.L    A0,$0001714E         L06C4
011460  41F900010F92              LEA       $00010F92,A0         STORE_JUMP_TABLE
011466  4240                      CLR.W     D0                  
011468  101B                      MOVE.B    (A3)+,D0            
01146A  30300000                  MOVE.W    0(A0,D0.W),D0       
01146E  4EF00000                  JMP       0(A0,D0.W)          


; =============================================================================
; OPCODE 0x1f -  offset 0x750 -  address $011472
; ceval
011472  4EF9000150EE              JMP       $000150EE            FN_READ_OPNAME_SAVE_D7


; =============================================================================
; OPCODE 0x20 -  offset 0x756 -  address $011478
; cadd
011478  61003C74                  BSR       15476(PC)            FN_READ_OPNAME_SAVE_D7
01147C  207900017146        OP_CADD:
                                  MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
011482  23F90001714E00017146      MOVE.L    $0001714E,$00017146  L06C4 ADDR_SCRIPT_NAME
01148C  23C80001714E              MOVE.L    A0,$0001714E         L06C4
011492  41F900010FCE              LEA       $00010FCE,A0         ADD_JUMP_TABLE
011498  4240                      CLR.W     D0                  
01149A  101B                      MOVE.B    (A3)+,D0            
01149C  30300000                  MOVE.W    0(A0,D0.W),D0       
0114A0  4EF00000                  JMP       0(A0,D0.W)          


; =============================================================================
; OPCODE 0x21 -  offset 0x782 -  address $0114a4
; csub
0114A4  61003C48                  BSR       15432(PC)            FN_READ_OPNAME_SAVE_D7
0114A8  4447                      NEG.W     D7                  
0114AA  6000FFD0                  BRA       -48(PC)              OP_CADD


; =============================================================================
; OPCODE 0x22 -  offset 0x78c -  address $0114ae
; cmul
0114AE  4E75                      RTS                           


; =============================================================================
; OPCODE 0x23 -  offset 0x78e -  address $0114b0
; cdiv
0114B0  4E75                      RTS                           


; =============================================================================
; OPCODE 0xba -  offset 0x790 -  address $0114b2
; cfont
0114B2  61003C3C                  BSR       15420(PC)            FN_READ_OPNAME
0114B6  33C70001707A              MOVE.W    D7,$0001707A         L0668
0114BC  61003C32                  BSR       15410(PC)            FN_READ_OPNAME
0114C0  33C700017072              MOVE.W    D7,$00017072         L0664
0114C6  61003C28                  BSR       15400(PC)            FN_READ_OPNAME
0114CA  13C700017074              MOVE.B    D7,$00017074         L0665
0114D0  61003C1E                  BSR       15390(PC)            FN_READ_OPNAME
0114D4  13C700017076              MOVE.B    D7,$00017076         L0666
0114DA  61003C14                  BSR       15380(PC)            FN_READ_OPNAME
0114DE  33C700017078              MOVE.W    D7,$00017078         L0667
0114E4  4E75                      RTS                           


; =============================================================================
; OPCODE 0xbf -  offset 0x7c4 -  address $0114e6
; cnumput
0114E6  61003C08                  BSR       15368(PC)            FN_READ_OPNAME
0114EA  33C7000170EC              MOVE.W    D7,$000170EC         L0696
0114F0  61003BFE                  BSR       15358(PC)            FN_READ_OPNAME
0114F4  33C7000170EE              MOVE.W    D7,$000170EE         L0697
0114FA  61003BF4                  BSR       15348(PC)            FN_READ_OPNAME
0114FE  33C7000170F0              MOVE.W    D7,$000170F0         L0698
011504  61003BEA                  BSR       15338(PC)            FN_READ_OPNAME
011508  13C7000170F8              MOVE.B    D7,$000170F8         L069C
01150E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb1 -  offset 0x7ee -  address $011510
; csinput
011510  42390001707D              CLR.B     $0001707D            B_L066A
011516  61000018                  BSR       24(PC)               __L035F
01151A  6100FF2E                  BSR       -210(PC)             L035D
01151E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb0 -  offset 0x7fe -  address $011520
; cvinput
011520  42390001707D              CLR.B     $0001707D            B_L066A
011526  61000054                  BSR       84(PC)               __L0363
01152A  6100FF1E                  BSR       -226(PC)             L035D
01152E  4E75                      RTS                           
011530  207900017146        __L035F:
                                  MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
011536  610000AC            L0360:BSR       172(PC)              L0368
01153A  B03C000D                  CMP.B     #$D,D0              
01153E  67000014                  BEQ       20(PC)               __L0361
011542  B03C0008                  CMP.B     #8,D0               
011546  67000010                  BEQ       16(PC)               __L0362
01154A  10C0                      MOVE.B    D0,(A0)+            
01154C  610001AE                  BSR       430(PC)              L0371
011550  6000FFE4                  BRA       -28(PC)              L0360
011554  4218                __L0361:
                                  CLR.B     (A0)+               
011556  4E75                      RTS                           
011558  B1F900017146        __L0362:
                                  CMPA.L    $00017146,A0         ADDR_SCRIPT_NAME
01155E  6700FFD6                  BEQ       -42(PC)              L0360
011562  4220                      CLR.B     -(A0)               
011564  61000196                  BSR       406(PC)              L0371
011568  103C0020                  MOVE.B    #$20,D0             ;(#" ") 
01156C  6100018E                  BSR       398(PC)              L0371
011570  103C0008                  MOVE.B    #8,D0               
011574  61000186                  BSR       390(PC)              L0371
011578  6000FFBC                  BRA       -68(PC)              L0360
01157C  207900017146        __L0363:
                                  MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
011582  61000060            L0364:BSR       96(PC)               L0368
011586  B03C000D                  CMP.B     #$D,D0              
01158A  6700002C                  BEQ       44(PC)               __L0366
01158E  B03C0008                  CMP.B     #8,D0               
011592  6700002C                  BEQ       44(PC)               __L0367
011596  B03C002D                  CMP.B     #$2D,D0             ;(#"-") 
01159A  67000012                  BEQ       18(PC)               __L0365
01159E  B03C0030                  CMP.B     #$30,D0             ;(#"0") 
0115A2  6B00FFDE                  BMI       -34(PC)              L0364
0115A6  B03C0039                  CMP.B     #$39,D0             ;(#"9") 
0115AA  6200FFD6                  BHI       -42(PC)              L0364
0115AE  10C0                __L0365:
                                  MOVE.B    D0,(A0)+            
0115B0  6100014A                  BSR       330(PC)              L0371
0115B4  6000FFCC                  BRA       -52(PC)              L0364
0115B8  4218                __L0366:
                                  CLR.B     (A0)+               
0115BA  61004188                  BSR       16776(PC)            __L0578
0115BE  4E75                      RTS                           
0115C0  B1F900017146        __L0367:
                                  CMPA.L    $00017146,A0         ADDR_SCRIPT_NAME
0115C6  6700FFBA                  BEQ       -70(PC)              L0364
0115CA  4220                      CLR.B     -(A0)               
0115CC  6100012E                  BSR       302(PC)              L0371
0115D0  103C0020                  MOVE.B    #$20,D0             ;(#" ") 
0115D4  61000126                  BSR       294(PC)              L0371
0115D8  103C0008                  MOVE.B    #8,D0               
0115DC  6100011E                  BSR       286(PC)              L0371
0115E0  6000FFA0                  BRA       -96(PC)              L0364
0115E4  48E700E0            L0368:MOVEM.L   A0-A2,-(A7)         
0115E8  4EB90000B792        __L0369:
                                  JSR       $0000B792            L0067
0115EE  4A00                      TST.B     D0                  
0115F0  6B00FFF6                  BMI       -10(PC)              __L0369
0115F4  4CDF0700                  MOVEM.L   (A7)+,A0-A2         
0115F8  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb7 -  offset 0x8d8 -  address $0115fa
; cspicprint
0115FA  13FC00010001707D          MOVE.B    #1,$0001707D         B_L066A
011602  61003AF8                  BSR       15096(PC)            L054B
011606  61000070                  BSR       112(PC)              L036C
01160A  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb6 -  offset 0x8ea -  address $01160c
; cvpicprint
01160C  13FC00010001707D          MOVE.B    #1,$0001707D         B_L066A
011614  61003AD8                  BSR       15064(PC)            FN_READ_OPNAME_SAVE_D7
011618  6100007C                  BSR       124(PC)              L036F
01161C  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb9 -  offset 0x8fc -  address $01161e
; csputprint
01161E  13FC00020001707D          MOVE.B    #2,$0001707D         B_L066A
011626  61003AD4                  BSR       15060(PC)            L054B
01162A  6100004C                  BSR       76(PC)               L036C
01162E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xb8 -  offset 0x90e -  address $011630
; cvputprint
011630  13FC00020001707D          MOVE.B    #2,$0001707D         B_L066A
011638  61003AB4                  BSR       15028(PC)            FN_READ_OPNAME_SAVE_D7
01163C  61000058                  BSR       88(PC)               L036F
011640  4E75                      RTS                           


; =============================================================================
; OPCODE 0x24 -  offset 0x920 -  address $011642
; cvprint
011642  42390001707D              CLR.B     $0001707D            B_L066A
011648  61003AA4                  BSR       15012(PC)            FN_READ_OPNAME_SAVE_D7
01164C  61000048                  BSR       72(PC)               L036F
011650  4E75                      RTS                           


; =============================================================================
; OPCODE 0x25 -  offset 0x930 -  address $011652
; csprinti
; param: 1 byte
011652  42390001707D        __L036A:
                                  CLR.B     $0001707D            B_L066A
011658  101B                      MOVE.B    (A3)+,D0            
01165A  66000004                  BNE       4(PC)                __L036B
01165E  4E75                      RTS                           
011660  6100009A            __L036B:
                                  BSR       154(PC)              L0371
011664  6000FFEC                  BRA       -20(PC)              __L036A


; =============================================================================
; OPCODE 0x26 -  offset 0x946 -  address $011668
; csprinta
011668  42390001707D              CLR.B     $0001707D            B_L066A
01166E  61003A7E                  BSR       14974(PC)            FN_READ_OPNAME_SAVE_D7
011672  61000004                  BSR       4(PC)                L036C
011676  4E75                      RTS                           


011678  48E78080            L036C:MOVEM.L   A0/D0,-(A7)         
01167C  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
011682  1018                __L036D:
                                  MOVE.B    (A0)+,D0            
011684  6700000A                  BEQ       10(PC)               __L036E
011688  61000072                  BSR       114(PC)              L0371
01168C  6000FFF4                  BRA       -12(PC)              __L036D
011690  4CDF0101            __L036E:
                                  MOVEM.L   (A7)+,A0/D0         
011694  4E75                      RTS                           
011696  3007                L036F:MOVE.W    D7,D0               
011698  610001AA                  BSR       426(PC)              L037D
01169C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x27 -  offset 0x97c -  address $01169e
; clocate
01169E  61003A4E                  BSR       14926(PC)            FN_READ_OPNAME_SAVE_D7
0116A2  61003A3E                  BSR       14910(PC)            L0548
0116A6  13C700017092              MOVE.B    D7,$00017092         L0672
0116AC  13C600017093              MOVE.B    D6,$00017093         L0673
0116B2  3007                      MOVE.W    D7,D0               
0116B4  3206                      MOVE.W    D6,D1               
0116B6  4EB90000BC6E              JSR       $0000BC6E            SYS_VT52_SET_CURSOR
0116BC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x28 -  offset 0x99c -  address $0116be
; ctab
0116BE  61003A2E                  BSR       14894(PC)            FN_READ_OPNAME_SAVE_D7
0116C2  13C700017092              MOVE.B    D7,$00017092         L0672
0116C8  123900017093              MOVE.B    $00017093,D1         L0673
0116CE  4EB90000BC6E              JSR       $0000BC6E            SYS_VT52_SET_CURSOR
0116D4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2a -  offset 0x9b4 -  address $0116d6
; crandom
0116D6  61003A18                  BSR       14872(PC)            FN_READ_OPNAME
0116DA  3007                      MOVE.W    D7,D0               
0116DC  66000016                  BNE       22(PC)               __L0370
0116E0  4EB90000B7B8              JSR       $0000B7B8            SYS_RANDOM
0116E6  4A3900017209              TST.B     $00017209            L06FF
0116EC  67000006                  BEQ       6(PC)                __L0370
0116F0  303C0064                  MOVE.W    #$64,D0             ;(#"d") 
0116F4  33C000017136        __L0370:
                                  MOVE.W    D0,$00017136         L06BE
0116FA  4E75                      RTS                           
; =============================================================================


0116FC  4A390001707D        L0371:TST.B     $0001707D            B_L066A
011702  6700001C                  BEQ       28(PC)               __L0372
011706  0C3900010001707D          CMPI.B    #1,$0001707D         B_L066A
01170E  67000018                  BEQ       24(PC)               __L0373
011712  0C3900020001707D          CMPI.B    #2,$0001707D         B_L066A
01171A  6700009A                  BEQ       154(PC)              __L0378
01171E  4E75                      RTS                           
011720  4EB90000BC54        __L0372:
                                  JSR       $0000BC54            SYS_VT52_PUT_CHAR
011726  4E75                      RTS                           
011728  48E78180            __L0373:
                                  MOVEM.L   A0/D7/D0,-(A7)      
01172C  B03C000A                  CMP.B     #$A,D0              
011730  67000072                  BEQ       114(PC)              __L0377
011734  B03C000D                  CMP.B     #$D,D0              
011738  67000058                  BEQ       88(PC)               __L0376
01173C  B03C0020                  CMP.B     #$20,D0             ;(#" ") 
011740  6700003C                  BEQ       60(PC)               L0374
011744  4A7900017072              TST.W     $00017072            L0664
01174A  6B000040                  BMI       64(PC)               L0375
01174E  4247                      CLR.W     D7                  
011750  1E00                      MOVE.B    D0,D7               
011752  9E790001707A              SUB.W     $0001707A,D7         L0668
011758  6B000024                  BMI       36(PC)               L0374
01175C  BE7900017078              CMP.W     $00017078,D7         L0667
011762  6A00001A                  BPL       26(PC)               L0374
011766  DE7900017072              ADD.W     $00017072,D7         L0664
01176C  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011774  423900017100              CLR.B     $00017100            L06A1
01177A  61003590                  BSR       13712(PC)            __L052A
01177E  4247                L0374:CLR.W     D7                  
011780  1E3900017074              MOVE.B    $00017074,D7         L0665
011786  DF790001706A              ADD.W     D7,$0001706A         L065E
01178C  4CDF0181            L0375:MOVEM.L   (A7)+,A0/D7/D0      
011790  4E75                      RTS                           
011792  303900017074        __L0376:
                                  MOVE.W    $00017074,D0         L0665
011798  E248                      LSR.W     #1,D0               
01179A  33C00001706A              MOVE.W    D0,$0001706A         L065E
0117A0  6000FFEA                  BRA       -22(PC)              L0375
0117A4  4247                __L0377:
                                  CLR.W     D7                  
0117A6  1E3900017076              MOVE.B    $00017076,D7         L0666
0117AC  DF790001706C              ADD.W     D7,$0001706C         L065F
0117B2  6000FFD8                  BRA       -40(PC)              L0375
0117B6  48E78180            __L0378:
                                  MOVEM.L   A0/D7/D0,-(A7)      
0117BA  B03C000A                  CMP.B     #$A,D0              
0117BE  67000072                  BEQ       114(PC)              __L037C
0117C2  B03C000D                  CMP.B     #$D,D0              
0117C6  67000060                  BEQ       96(PC)               __L037B
0117CA  B03C0020                  CMP.B     #$20,D0             ;(#" ") 
0117CE  67000044                  BEQ       68(PC)               L0379
0117D2  4A7900017072              TST.W     $00017072            L0664
0117D8  6B000048                  BMI       72(PC)               L037A
0117DC  4247                      CLR.W     D7                  
0117DE  1E00                      MOVE.B    D0,D7               
0117E0  9E790001707A              SUB.W     $0001707A,D7         L0668
0117E6  6B00002C                  BMI       44(PC)               L0379
0117EA  BE7900017078              CMP.W     $00017078,D7         L0667
0117F0  6A000022                  BPL       34(PC)               L0379
0117F4  DE7900017072              ADD.W     $00017072,D7         L0664
0117FA  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011802  423900017100              CLR.B     $00017100            L06A1
011808  13FC000100017102          MOVE.B    #1,$00017102         L06A3
011810  610007A0                  BSR       1952(PC)             L03A7
011814  4247                L0379:CLR.W     D7                  
011816  1E3900017074              MOVE.B    $00017074,D7         L0665
01181C  DF79000170EC              ADD.W     D7,$000170EC         L0696
011822  4CDF0181            L037A:MOVEM.L   (A7)+,A0/D7/D0      
011826  4E75                      RTS                           
011828  4279000170EC        __L037B:
                                  CLR.W     $000170EC            L0696
01182E  6000FFF2                  BRA       -14(PC)              L037A
011832  4247                __L037C:
                                  CLR.W     D7                  
011834  1E3900017076              MOVE.B    $00017076,D7         L0666
01183A  9F79000170F0              SUB.W     D7,$000170F0         L0698
011840  6000FFE0                  BRA       -32(PC)              L037A
011844  48E78080            L037D:MOVEM.L   A0/D0,-(A7)         
011848  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01184E  61000016                  BSR       22(PC)               L0380
011852  1018                __L037E:
                                  MOVE.B    (A0)+,D0            
011854  6700000A                  BEQ       10(PC)               __L037F
011858  6100FEA2                  BSR       -350(PC)             L0371
01185C  6000FFF4                  BRA       -12(PC)              __L037E
011860  4CDF0101            __L037F:
                                  MOVEM.L   (A7)+,A0/D0         
011864  4E75                      RTS                           



011866  48E7F8C0            L0380:MOVEM.L   A0-A1/D0-D4,-(A7)   
01186A  48C0                      EXT.L     D0                  
01186C  7209                      MOVEQ     #9,D1               
01186E  4204                      CLR.B     D4                  
011870  2400                      MOVE.L    D0,D2               
011872  4A82                      TST.L     D2                  
011874  6C00000A                  BGE       10(PC)               __L0381
011878  103C002D                  MOVE.B    #$2D,D0             ;(#"-") 
01187C  10C0                      MOVE.B    D0,(A0)+            
01187E  4482                      NEG.L     D2                  
011880  43F9000118BA        __L0381:
                                  LEA       $000118BA,A1         DATA_0387
011886  2619                __L0382:
                                  MOVE.L    (A1)+,D3            
011888  70D0                      MOVEQ     #$D0,D0             
01188A  9483                __L0383:
                                  SUB.L     D3,D2               
01188C  5BC8FFFC                  DBMI      D0,-4(PC)            __L0383
011890  4400                      NEG.B     D0                  
011892  B03C0030                  CMP.B     #$30,D0             ;(#"0") 
011896  66000008                  BNE       8(PC)                __L0384
01189A  4A04                      TST.B     D4                  
01189C  67000006                  BEQ       6(PC)                __L0385
0118A0  7801                __L0384:
                                  MOVEQ     #1,D4               
0118A2  10C0                      MOVE.B    D0,(A0)+            
0118A4  D483                __L0385:
                                  ADD.L     D3,D2               
0118A6  51C9FFDE                  DBF       D1,-34(PC)           __L0382
0118AA  4A04                      TST.B     D4                  
0118AC  66000004                  BNE       4(PC)                __L0386
0118B0  10C0                      MOVE.B    D0,(A0)+            
0118B2  4210                __L0386:
                                  CLR.B     (A0)                
0118B4  4CDF031F                  MOVEM.L   (A7)+,A0-A1/D0-D4   
0118B8  4E75                      RTS                           


0118BA  3B9ACA0005F5E100    DATA_L0387:
                                  DC.B      $3B,$9A,$CA,$00,$05,$F5,$E1,$00 
0118C2  00989680000F4240          DC.B      $00,$98,$96,$80,$00,$0F,$42,$40 
0118CA  000186A000002710          DC.B      $00,$01,$86,$A0,$00,$00,$27,$10 
0118D2  000003E800000064          DC.B      $00,$00,$03,$E8,$00,$00,$00,$64 
0118DA  0000000A00000001          DC.B      $00,$00,$00,$0A,$00,$00,$00,$01 


; =============================================================================
; OPCODE 0xae -  offset 0xbc0 -  address $0118e2
; csettime
0118E2  6100380C                  BSR       14348(PC)            FN_READ_OPNAME
0118E6  48A70100                  MOVEM.W   D7,-(A7)            ; empile D7
0118EA  61003804                  BSR       14340(PC)            FN_READ_OPNAME
0118EE  48A70100                  MOVEM.W   D7,-(A7)            ; empile D7
0118F2  610037FC                  BSR       14332(PC)            FN_READ_OPNAME
0118F6  301F                      MOVE.W    (A7)+,D0            ; dépile dans D0
0118F8  321F                      MOVE.W    (A7)+,D1            ; dépile dans D1
0118FA  3407                      MOVE.W    D7,D2               ; "dépile" dans d2
0118FC  4EB90000B9D2              JSR       $0000B9D2            SYS_SET_TIME
011902  4E75                      RTS                           


; =============================================================================
; OPCODE 0xaf -  offset 0xbe2 -  address $011904
; cgettime
011904  4EB90000B9F4              JSR       $0000B9F4            SYS_GET_TIME
01190A  48A76000                  MOVEM.W   D1-D2,-(A7)         
01190E  3E00                      MOVE.W    D0,D7               
011910  6100FB38                  BSR       -1224(PC)            L035D
011914  3E1F                      MOVE.W    (A7)+,D7            
011916  6100FB32                  BSR       -1230(PC)            L035D
01191A  3E1F                      MOVE.W    (A7)+,D7            
01191C  6000FB2C                  BRA       -1236(PC)            L035D


; =============================================================================
; OPCODE 0x3c -  offset 0xbfe -  address $011920
; cwlive
011920  2F04                      MOVE.L    D4,-(A7)            
011922  61001A38                  BSR       6712(PC)             L0463
011926  48B900070001715E          MOVEM.W   D0-D2,$0001715E      L06CA
01192E  60000018                  BRA       24(PC)               __L0388


; =============================================================================
; OPCODE 0x40 -  offset 0xc10 -  address $011932
; clive
011932  48E70800                  MOVEM.L   D4,-(A7)            
011936  42790001715E              CLR.W     $0001715E            L06CA
01193C  427900017160              CLR.W     $00017160            L06CB
011942  427900017162              CLR.W     $00017162            L06CC
011948  141B                __L0388:
                                  MOVE.B    (A3)+,D2            
01194A  E142                      ASL.W     #8,D2               
01194C  141B                      MOVE.B    (A3)+,D2            
01194E  61004EB6                  BSR       20150(PC)            L060D
011952  67000006                  BEQ       6(PC)                __L0389
011956  7EFF                      MOVEQ     #$FF,D7             
011958  605A                      BRA.S     90(PC)               __L038A
01195A  61005006            __L0389:
                                  BSR       20486(PC)            L061D
01195E  3E01                      MOVE.W    D1,D7               
011960  216E00000000              MOVE.L    0(A6),0(A0)         
011966  216E00040004              MOVE.L    4(A6),4(A0)         
01196C  116E00090009              MOVE.B    9(A6),9(A0)         
011972  116E000A000A              MOVE.B    10(A6),10(A0)       
011978  116E000B000B              MOVE.B    11(A6),11(A0)       
01197E  316EFFF2FFD8              MOVE.W    -14(A6),-40(A0)     
011984  30390001715E              MOVE.W    $0001715E,D0         L06CA
01198A  D1680000                  ADD.W     D0,0(A0)            
01198E  303900017160              MOVE.W    $00017160,D0         L06CB
011994  D1680002                  ADD.W     D0,2(A0)            
011998  303900017162              MOVE.W    $00017162,D0         L06CC
01199E  D1680004                  ADD.W     D0,4(A0)            
0119A2  316EFFEAFFEA              MOVE.W    -22(A6),-22(A0)     
0119A8  316EFFDEFFDE              MOVE.W    -34(A6),-34(A0)     
0119AE  3168FFF2FFD6              MOVE.W    -14(A0),-42(A0)     
0119B4  4CDF0010            __L038A:
                                  MOVEM.L   (A7)+,D4            
0119B8  4EF90001144A              JMP       $0001144A            L035D


; =============================================================================
; OPCODE 0xde -  offset 0xc9c -  address $0119be
; cblast
0119BE  13FC000100017081          MOVE.B    #1,$00017081         B_066D


; =============================================================================
; OPCODE 0x41 -  offset 0xca4 -  address $0119c6
; ckill
0119C6  61003726                  BSR       14118(PC)            FN_READ_OPNAME_SAVE_D7
0119CA  3007                      MOVE.W    D7,D0               
0119CC  6A000004                  BPL       4(PC)                __L038B
0119D0  4E75                      RTS                           
0119D2  600050BA            __L038B:
                                  BRA       20666(PC)            L0622


; =============================================================================
; OPCODE 0x42 -  offset 0xcb4 -  address $0119d6
; cstop
0119D6  588F                OP_CSTOP:
                                  ADDQ.L    #4,A7               
0119D8  4E75                      RTS                           


; =============================================================================
; OPCODE 0x43 -  offset 0xcb8 -  address $0119da
; cstopret
0119DA  4A3900017107              TST.B     $00017107            B_06A7
0119E0  6700FFF4                  BEQ       -12(PC)              OP_CSTOP
0119E4  6100F8C8                  BSR       -1848(PC)            OP_CRET
0119E8  588F                      ADDQ.L    #4,A7               
0119EA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x44 -  offset 0xcca -  address $0119ec
; cexit
0119EC  4A45                      TST.W     D5                  
0119EE  66000006                  BNE       6(PC)                __L038D
0119F2  6000F61C                  BRA       -2532(PC)            FN_TERMINATE_PROGRAM
0119F6  3005                __L038D:
                                  MOVE.W    D5,D0               
0119F8  60005094                  BRA       20628(PC)            L0622


; =============================================================================
; OPCODE 0x45 -  offset 0xcda -  address $0119fc
; read a script file name, then open + decrunch file
; cload
0119FC  141B                      MOVE.B    (A3)+,D2            
0119FE  E142                      ASL.W     #8,D2               
011A00  141B                      MOVE.B    (A3)+,D2            
011A02  4A42                      TST.W     D2                  ; get script ID in D2 (ID 0 is main script)
011A04  67000010                  BEQ       16(PC)               __op45_is_main_script
011A08  204B                      MOVEA.L   A3,A0               
011A0A  61004892                  BSR       18578(PC)            FN_READ_FILE_NAME
011A0E  4A1B                __op45_move_ptr:
                                  TST.B     (A3)+                ; move script ptr to file name end
011A10  6600FFFC                  BNE       -4(PC)               __op45_move_ptr
011A14  4E75                      RTS                           
011A16  610036E4            __op45_is_main_script:
                                  BSR       14052(PC)            L054B
011A1A  61004602                  BSR       17922(PC)            __L05B9
011A1E  504F                      ADDQ.W    #8,A7               
011A20  6000F652                  BRA       -2478(PC)            __L033E


; =============================================================================
; OPCODE 0x3d -  offset 0xd02 -  address $011a24
; cunload
; param: 2 byte
011A24  141B                      MOVE.B    (A3)+,D2            
011A26  E142                      ASL.W     #8,D2               
011A28  141B                      MOVE.B    (A3)+,D2            
011A2A  60004D1C                  BRA       19740(PC)            OP_UNLOAD


; =============================================================================
; OPCODE 0x46 -  offset 0xd0c -  address $011a2e
; cdefsc
011A2E  101B                      MOVE.B    (A3)+,D0            
011A30  E140                      ASL.W     #8,D0               
011A32  101B                      MOVE.B    (A3)+,D0             ; move word following opcode into d0
011A34  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011A3A  08F000060000              BSET      #6,0(A0,D0.W)       
011A40  119B0001                  MOVE.B    (A3)+,1(A0,D0.W)     ; read byte at (a0 + d0)
011A44  721F                      MOVEQ     #$1F,D1             
011A46  43F00006                  LEA       6(A0,D0.W),A1       
011A4A  12DB                __L0390:
                                  MOVE.B    (A3)+,(A1)+         
011A4C  51C9FFFC                  DBF       D1,-4(PC)            __L0390 ; read 32 bytes from script
011A50  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
011A56  3239000170C6              MOVE.W    $000170C6,D1         W_L0683
011A5C  6700006A                  BEQ       106(PC)              __L0391 ; show error 10 ?
011A60  33F11004000170C6          MOVE.W    4(A1,D1.W),$000170C6 W_L0683
011A68  31810002                  MOVE.W    D1,2(A0,D0.W)       
011A6C  42711006                  CLR.W     6(A1,D1.W)          
011A70  42700004                  CLR.W     4(A0,D0.W)          
011A74  13B000011001              MOVE.B    1(A0,D0.W),1(A1,D1.W) 
011A7A  3430000E                  MOVE.W    14(A0,D0.W),D2      
011A7E  0242FFF0                  ANDI.W    #$FFF0,D2           
011A82  3382100C                  MOVE.W    D2,12(A1,D1.W)      
011A86  33B00010100E              MOVE.W    16(A0,D0.W),14(A1,D1.W) 
011A8C  33BC7FFF1010              MOVE.W    #$7FFF,16(A1,D1.W)  
011A92  3430000E                  MOVE.W    14(A0,D0.W),D2      
011A96  D4700012                  ADD.W     18(A0,D0.W),D2      
011A9A  0042000F                  ORI.W     #$F,D2              
011A9E  33821016                  MOVE.W    D2,22(A1,D1.W)      
011AA2  34300010                  MOVE.W    16(A0,D0.W),D2      
011AA6  D4700014                  ADD.W     20(A0,D0.W),D2      
011AAA  33821018                  MOVE.W    D2,24(A1,D1.W)      
011AAE  4270002A                  CLR.W     42(A0,D0.W)         
011AB2  4270002C                  CLR.W     44(A0,D0.W)         
011AB6  4270002E                  CLR.W     46(A0,D0.W)         
011ABA  61000328                  BSR       808(PC)              L039E
011ABE  41F00000                  LEA       0(A0,D0.W),A0       
011AC2  61000378                  BSR       888(PC)              L03A4
011AC6  4E75                      RTS                           
011AC8  203C0000000A        __L0391:
                                  MOVE.L    #$A,D0               ; show error 10, quit
011ACE  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


011AD4  48E74040            __L0392:
                                  MOVEM.L   A1/D1,-(A7)         
011AD8  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
011ADE  32280002                  MOVE.W    2(A0),D1            
011AE2  13A800011001              MOVE.B    1(A0),1(A1,D1.W)    
011AE8  4CDF0202                  MOVEM.L   (A7)+,A1/D1         
011AEC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x3b -  offset 0xdcc -  address $011aee
; cswitching
011AEE  4A390000AC76              TST.B     $0000AC76            L0015
011AF4  6600000A                  BNE       10(PC)               __L0393
011AF8  13FC00010001712D          MOVE.B    #1,$0001712D         L06BA
011B00  4E75                __L0393:
                                  RTS                           


; =============================================================================
; OPCODE 0x3a -  offset 0xde0 -  address $011b02
; cclipping
011B02  42390001712D              CLR.B     $0001712D            L06BA
011B08  4E75                      RTS                           


; =============================================================================
; OPCODE 0x38 -  offset 0xde8 -  address $011b0a
; cscmov
011B0A  610035E4                  BSR       13796(PC)            FN_READ_OPNAME
011B0E  3F07                      MOVE.W    D7,-(A7)            
011B10  610035DE                  BSR       13790(PC)            FN_READ_OPNAME
011B14  3F07                      MOVE.W    D7,-(A7)            
011B16  610035D8                  BSR       13784(PC)            FN_READ_OPNAME
011B1A  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011B20  D0EEFFEA                  ADDA.W    -22(A6),A0          
011B24  3147002E                  MOVE.W    D7,46(A0)           
011B28  315F002C                  MOVE.W    (A7)+,44(A0)        
011B2C  315F002A                  MOVE.W    (A7)+,42(A0)        
011B30  08E800070000              BSET      #7,0(A0)            
011B36  4E75                      RTS                           


; =============================================================================
; OPCODE 0x39 -  offset 0xe16 -  address $011b38
; cscset
011B38  610035B6                  BSR       13750(PC)            FN_READ_OPNAME
011B3C  3F07                      MOVE.W    D7,-(A7)            
011B3E  610035B0                  BSR       13744(PC)            FN_READ_OPNAME
011B42  3F07                      MOVE.W    D7,-(A7)            
011B44  610035AA                  BSR       13738(PC)            FN_READ_OPNAME
011B48  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011B4E  D0EEFFEA                  ADDA.W    -22(A6),A0          
011B52  9E68001A                  SUB.W     26(A0),D7           
011B56  3147002E                  MOVE.W    D7,46(A0)           
011B5A  3E1F                      MOVE.W    (A7)+,D7            
011B5C  9E680018                  SUB.W     24(A0),D7           
011B60  3147002C                  MOVE.W    D7,44(A0)           
011B64  3E1F                      MOVE.W    (A7)+,D7            
011B66  9E680016                  SUB.W     22(A0),D7           
011B6A  3147002A                  MOVE.W    D7,42(A0)           
011B6E  08E800070000              BSET      #7,0(A0)            
011B74  4E75                      RTS                           


; =============================================================================
; OPCODE 0x47 -  offset 0xe54 -  address $011b76
; cscreen
; param: 2 bytes
011B76  101B                      MOVE.B    (A3)+,D0            
011B78  E140                      ASL.W     #8,D0               
011B7A  101B                      MOVE.B    (A3)+,D0            
011B7C  B06EFFEA                  CMP.W     -22(A6),D0          
011B80  67000006                  BEQ       6(PC)                __L0394
011B84  3D40FFEA                  MOVE.W    D0,-22(A6)          
011B88  4E75                __L0394:
                                  RTS                           


; =============================================================================
; OPCODE 0xc0 -  offset 0xe68 -  address $011b8a
011B8A  61003564                  BSR       13668(PC)            FN_READ_OPNAME
011B8E  61003552                  BSR       13650(PC)            L0548
011B92  302EFFEA                  MOVE.W    -22(A6),D0          
011B96  67000016                  BEQ       22(PC)               __L0395
011B9A  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011BA0  3187000A                  MOVE.W    D7,10(A0,D0.W)      
011BA4  3186000C                  MOVE.W    D6,12(A0,D0.W)      
011BA8  08F000070000              BSET      #7,0(A0,D0.W)       
011BAE  4E75                __L0395:
                                  RTS                           


; =============================================================================
; OPCODE 0xc2 -  offset 0xe8e -  address $011bb0
011BB0  6100353E                  BSR       13630(PC)            FN_READ_OPNAME
011BB4  6100352C                  BSR       13612(PC)            L0548
011BB8  302EFFEA                  MOVE.W    -22(A6),D0          
011BBC  67000016                  BEQ       22(PC)               __L0396
011BC0  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011BC6  31870012                  MOVE.W    D7,18(A0,D0.W)      
011BCA  31860014                  MOVE.W    D6,20(A0,D0.W)      
011BCE  08F000070000              BSET      #7,0(A0,D0.W)       
011BD4  4E75                __L0396:
                                  RTS                           


; =============================================================================
; OPCODE 0xc1 -  offset 0xeb4 -  address $011bd6
011BD6  61003518                  BSR       13592(PC)            FN_READ_OPNAME
011BDA  61003506                  BSR       13574(PC)            L0548
011BDE  302EFFEA                  MOVE.W    -22(A6),D0          
011BE2  67000016                  BEQ       22(PC)               __L0397
011BE6  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011BEC  3187000E                  MOVE.W    D7,14(A0,D0.W)      
011BF0  31860010                  MOVE.W    D6,16(A0,D0.W)      
011BF4  08F000070000              BSET      #7,0(A0,D0.W)       
011BFA  4E75                __L0397:
                                  RTS                           


; =============================================================================
; OPCODE 0xc3 -  offset 0xeda -  address $011bfc
011BFC  4A2EFFEA                  TST.B     -22(A6)             
011C00  67000032                  BEQ       50(PC)               __L0398
011C04  610034EA                  BSR       13546(PC)            FN_READ_OPNAME
011C08  3F07                      MOVE.W    D7,-(A7)            
011C0A  610034E4                  BSR       13540(PC)            FN_READ_OPNAME
011C0E  3F07                      MOVE.W    D7,-(A7)            
011C10  610034DE                  BSR       13534(PC)            FN_READ_OPNAME
011C14  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011C1A  D0EEFFEA                  ADDA.W    -22(A6),A0          
011C1E  11470022                  MOVE.B    D7,34(A0)           
011C22  3E1F                      MOVE.W    (A7)+,D7            
011C24  11470021                  MOVE.B    D7,33(A0)           
011C28  3E1F                      MOVE.W    (A7)+,D7            
011C2A  11470020                  MOVE.B    D7,32(A0)           
011C2E  08E800070000              BSET      #7,0(A0)            
011C34  4E75                __L0398:
                                  RTS                           


; =============================================================================
; OPCODE 0xc4 -  offset 0xf14 -  address $011c36
011C36  4A2EFFEA                  TST.B     -22(A6)             
011C3A  67000036                  BEQ       54(PC)               __L0399
011C3E  610034B0                  BSR       13488(PC)            FN_READ_OPNAME
011C42  3F07                      MOVE.W    D7,-(A7)            
011C44  610034AA                  BSR       13482(PC)            FN_READ_OPNAME
011C48  3F07                      MOVE.W    D7,-(A7)            
011C4A  610034A4                  BSR       13476(PC)            FN_READ_OPNAME
011C4E  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011C54  D0EEFFEA                  ADDA.W    -22(A6),A0          
011C58  11470025                  MOVE.B    D7,37(A0)           
011C5C  3E1F                      MOVE.W    (A7)+,D7            
011C5E  11470024                  MOVE.B    D7,36(A0)           
011C62  3E1F                      MOVE.W    (A7)+,D7            
011C64  11470023                  MOVE.B    D7,35(A0)           
011C68  610001D2                  BSR       466(PC)              L03A4
011C6C  08E800070000              BSET      #7,0(A0)            
011C72  4E75                __L0399:
                                  RTS                           
011C74  61003478                  BSR       13432(PC)            FN_READ_OPNAME_SAVE_D7
011C78  3F07                      MOVE.W    D7,-(A7)            
011C7A  61003472                  BSR       13426(PC)            FN_READ_OPNAME_SAVE_D7
011C7E  3F07                      MOVE.W    D7,-(A7)            
011C80  6100346C                  BSR       13420(PC)            FN_READ_OPNAME_SAVE_D7
011C84  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011C8A  302EFFEA                  MOVE.W    -22(A6),D0          
011C8E  67000034                  BEQ       52(PC)               __L039A
011C92  3607                      MOVE.W    D7,D3               
011C94  341F                      MOVE.W    (A7)+,D2            
011C96  321F                      MOVE.W    (A7)+,D1            
011C98  18300020                  MOVE.B    32(A0,D0.W),D4      
011C9C  4884                      EXT.W     D4                  
011C9E  1A300021                  MOVE.B    33(A0,D0.W),D5      
011CA2  4885                      EXT.W     D5                  
011CA4  1C300022                  MOVE.B    34(A0,D0.W),D6      
011CA8  4886                      EXT.W     D6                  
011CAA  61000818                  BSR       2072(PC)             L03D7
011CAE  11440020                  MOVE.B    D4,32(A0)           
011CB2  11450021                  MOVE.B    D5,33(A0)           
011CB6  11460022                  MOVE.B    D6,34(A0)           
011CBA  61000180                  BSR       384(PC)              L03A4
011CBE  08E800070000              BSET      #7,0(A0)            
011CC4  4E75                __L039A:
                                  RTS                           


; =============================================================================
; OPCODE 0xc5 -  offset 0xfa4 -  address $011cc6
011CC6  61003428                  BSR       13352(PC)            FN_READ_OPNAME
011CCA  61003416                  BSR       13334(PC)            L0548
011CCE  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011CD4  302EFFEA                  MOVE.W    -22(A6),D0          
011CD8  67000024                  BEQ       36(PC)               __L039B
011CDC  D0C0                      ADDA.W    D0,A0               
011CDE  1147001D                  MOVE.B    D7,29(A0)           
011CE2  5346                      SUBQ.W    #1,D6               
011CE4  1146001E                  MOVE.B    D6,30(A0)           
011CE8  48E70080                  MOVEM.L   A0,-(A7)            
011CEC  61003402                  BSR       13314(PC)            FN_READ_OPNAME
011CF0  4CDF0100                  MOVEM.L   (A7)+,A0            
011CF4  1147001F                  MOVE.B    D7,31(A0)           
011CF8  08E800070000              BSET      #7,0(A0)            
011CFE  4E75                __L039B:
                                  RTS                           


; =============================================================================
; OPCODE 0xc6 -  offset 0xfde -  address $011d00
011D00  610033EE                  BSR       13294(PC)            FN_READ_OPNAME
011D04  302EFFEA                  MOVE.W    -22(A6),D0          
011D08  67000012                  BEQ       18(PC)               __L039C
011D0C  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011D12  1187001C                  MOVE.B    D7,28(A0,D0.W)      
011D16  08F000070000              BSET      #7,0(A0,D0.W)       
011D1C  4E75                __L039C:
                                  RTS                           


; =============================================================================
; OPCODE 0xc8 -  offset 0xffc -  address $011d1e
011D1E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xc9 -  offset 0xffe -  address $011d20
011D20  4E75                      RTS                           


; =============================================================================
; OPCODE 0xe0 -  offset 0x1000 -  address $011d22
011D22  610033CC                  BSR       13260(PC)            FN_READ_OPNAME
011D26  13C70001723C              MOVE.B    D7,$0001723C         L0715
011D2C  610033C2                  BSR       13250(PC)            FN_READ_OPNAME
011D30  33C70001723E              MOVE.W    D7,$0001723E         L0717
011D36  610033B8                  BSR       13240(PC)            FN_READ_OPNAME
011D3A  33C700017240              MOVE.W    D7,$00017240         L0718
011D40  610033AE                  BSR       13230(PC)            FN_READ_OPNAME
011D44  33C700017242              MOVE.W    D7,$00017242         L0719
011D4A  4A390001723C              TST.B     $0001723C            L0715
011D50  67000012                  BEQ       18(PC)               __L039D
011D54  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011D5A  D0EEFFEA                  ADDA.W    -22(A6),A0          
011D5E  08E800050000              BSET      #5,0(A0)            
011D64  4E75                __L039D:
                                  RTS                           


; =============================================================================
; OPCODE 0xdf -  offset 0x1044 -  address $011d66
011D66  1C390001720A              MOVE.B    $0001720A,D6         L0700
011D6C  3F06                      MOVE.W    D6,-(A7)            
011D6E  61003380                  BSR       13184(PC)            FN_READ_OPNAME
011D72  13C70001720A              MOVE.B    D7,$0001720A         L0700
011D78  61003376                  BSR       13174(PC)            FN_READ_OPNAME
011D7C  33C700017218              MOVE.W    D7,$00017218         L0705
011D82  6100336C                  BSR       13164(PC)            FN_READ_OPNAME
011D86  33C70001721A              MOVE.W    D7,$0001721A         L0706
011D8C  61003362                  BSR       13154(PC)            FN_READ_OPNAME
011D90  33C70001721C              MOVE.W    D7,$0001721C         L0707
011D96  61003358                  BSR       13144(PC)            FN_READ_OPNAME
011D9A  33C70001721E              MOVE.W    D7,$0001721E         L0708
011DA0  6100334E                  BSR       13134(PC)            FN_READ_OPNAME
011DA4  33C700017220              MOVE.W    D7,$00017220         L0709
011DAA  3C1F                      MOVE.W    (A7)+,D6            
011DAC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x4e -  offset 0x108c -  address $011dae
011DAE  101B                      MOVE.B    (A3)+,D0            
011DB0  E140                      ASL.W     #8,D0               
011DB2  101B                      MOVE.B    (A3)+,D0            
011DB4  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011DBA  08B000060000              BCLR      #6,0(A0,D0.W)       
011DC0  08F000070000              BSET      #7,0(A0,D0.W)       
011DC6  61000042                  BSR       66(PC)               __L03A0
011DCA  61000018                  BSR       24(PC)               L039E
011DCE  4E75                      RTS                           


; =============================================================================
; OPCODE 0x4f -  offset 0x10ae -  address $011dd0
011DD0  101B                      MOVE.B    (A3)+,D0            
011DD2  E140                      ASL.W     #8,D0               
011DD4  101B                      MOVE.B    (A3)+,D0            
011DD6  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
011DDC  08F000060000              BSET      #6,0(A0,D0.W)       
011DE2  4E75                      RTS                           


; =============================================================================
011DE4  42700004            L039E:
                                  CLR.W     4(A0,D0.W)          
011DE8  323900017104              MOVE.W    $00017104,D1         W_06A5
011DEE  6600000A                  BNE       10(PC)               __L039F
011DF2  33C000017104              MOVE.W    D0,$00017104         W_06A5
011DF8  4E75                      RTS                           
011DFA  3401                __L039F:
                                  MOVE.W    D1,D2               
011DFC  32302004                  MOVE.W    4(A0,D2.W),D1       
011E00  6600FFF8                  BNE       -8(PC)               __L039F
011E04  31802004                  MOVE.W    D0,4(A0,D2.W)       
011E08  4E75                      RTS                           
; =============================================================================


011E0A  343900017104        __L03A0:
                                  MOVE.W    $00017104,D2         W_06A5
011E10  67000028                  BEQ       40(PC)               __L03A3
011E14  B042                      CMP.W     D2,D0               
011E16  66000012                  BNE       18(PC)               __L03A2
011E1A  33F0000400017104          MOVE.W    4(A0,D0.W),$00017104 W_06A5
011E22  4E75                      RTS                           
011E24  3401                __L03A1:
                                  MOVE.W    D1,D2               
011E26  67000012                  BEQ       18(PC)               __L03A3
011E2A  32302004            __L03A2:
                                  MOVE.W    4(A0,D2.W),D1       
011E2E  B041                      CMP.W     D1,D0               
011E30  6600FFF2                  BNE       -14(PC)              __L03A1
011E34  31B000042004              MOVE.W    4(A0,D0.W),4(A0,D2.W) 
011E3A  4E75                __L03A3:RTS                           


; =============================================================================
011E3C  10280021            L03A4:
                                  MOVE.B    33(A0),D0           
011E40  4880                      EXT.W     D0                  
011E42  12280025                  MOVE.B    37(A0),D1           
011E46  4881                      EXT.W     D1                  
011E48  C1C1                      MULS      D1,D0               
011E4A  14280024                  MOVE.B    36(A0),D2           
011E4E  4882                      EXT.W     D2                  
011E50  16280022                  MOVE.B    34(A0),D3           
011E54  4883                      EXT.W     D3                  
011E56  C5C3                      MULS      D3,D2               
011E58  9042                      SUB.W     D2,D0               
011E5A  11400026                  MOVE.B    D0,38(A0)           
011E5E  10280022                  MOVE.B    34(A0),D0           
011E62  4880                      EXT.W     D0                  
011E64  12280023                  MOVE.B    35(A0),D1           
011E68  4881                      EXT.W     D1                  
011E6A  C1C1                      MULS      D1,D0               
011E6C  14280025                  MOVE.B    37(A0),D2           
011E70  4882                      EXT.W     D2                  
011E72  16280020                  MOVE.B    32(A0),D3           
011E76  4883                      EXT.W     D3                  
011E78  C5C3                      MULS      D3,D2               
011E7A  9042                      SUB.W     D2,D0               
011E7C  11400027                  MOVE.B    D0,39(A0)           
011E80  10280020                  MOVE.B    32(A0),D0           
011E84  4880                      EXT.W     D0                  
011E86  12280024                  MOVE.B    36(A0),D1           
011E8A  4881                      EXT.W     D1                  
011E8C  C1C1                      MULS      D1,D0               
011E8E  14280023                  MOVE.B    35(A0),D2           
011E92  4882                      EXT.W     D2                  
011E94  16280021                  MOVE.B    33(A0),D3           
011E98  4883                      EXT.W     D3                  
011E9A  C5C3                      MULS      D3,D2               
011E9C  9042                      SUB.W     D2,D0               
011E9E  11400028                  MOVE.B    D0,40(A0)           
011EA2  4E75                      RTS                           


; =============================================================================
; OPCODE 0x9a -  offset 0x1182 -  address $011ea4
011EA4  1D7C0001FFFD              MOVE.B    #1,-3(A6)           
011EAA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x9b -  offset 0x118a -  address $011eac
011EAC  422EFFFD                  CLR.B     -3(A6)              
011EB0  4E75                      RTS                           


; =============================================================================
; OPCODE 0x99 -  offset 0x1190 -  address $011eb2
011EB2  086E0000FFFD              BCHG      #0,-3(A6)           
011EB8  4E75                      RTS                           


; =============================================================================
; OPCODE 0x48 -  offset 0x1198 -  address $011eba
011EBA  423900017103              CLR.B     $00017103            L06A4
011EC0  13EEFFFD00017100          MOVE.B    -3(A6),$00017100     L06A1
011EC8  61003226            L03A5:BSR       12838(PC)            FN_READ_OPNAME
011ECC  42B9000170EC              CLR.L     $000170EC            L0696
011ED2  4279000170F0              CLR.W     $000170F0            L0698
011ED8  4239000170F8              CLR.B     $000170F8            L069C
011EDE  4EF900011FB2              JMP       $00011FB2            L03A7


; =============================================================================
; OPCODE 0x49 -  offset 0x11c2 -  address $011ee4
; cputnat
011EE4  423900017103              CLR.B     $00017103            L06A4
011EEA  13EEFFFD00017100          MOVE.B    -3(A6),$00017100     L06A1
011EF2  610031FA            L03A6:BSR       12794(PC)            FN_READ_OPNAME_SAVE_D7
011EF6  33C7000170EC              MOVE.W    D7,$000170EC         L0696
011EFC  610031F0                  BSR       12784(PC)            FN_READ_OPNAME_SAVE_D7
011F00  33C7000170EE              MOVE.W    D7,$000170EE         L0697
011F06  610031E6                  BSR       12774(PC)            FN_READ_OPNAME_SAVE_D7
011F0A  33C7000170F0              MOVE.W    D7,$000170F0         L0698
011F10  610031DC                  BSR       12764(PC)            FN_READ_OPNAME_SAVE_D7
011F14  610031CC                  BSR       12748(PC)            L0548
011F18  13C6000170F8              MOVE.B    D6,$000170F8         L069C
011F1E  4EF900011FB2              JMP       $00011FB2            L03A7


; =============================================================================
; OPCODE 0xa6 -  offset 0x1202 -  address $011f24
011F24  423900017103              CLR.B     $00017103            L06A4
011F2A  102EFFFD                  MOVE.B    -3(A6),D0           
011F2E  08400000                  BCHG      #0,D0               
011F32  13C000017100              MOVE.B    D0,$00017100         L06A1
011F38  6000FF8E                  BRA       -114(PC)             L03A5
011F3C  4E75                      RTS                           


; =============================================================================
; OPCODE 0xa7 -  offset 0x121c -  address $011f3e
011F3E  423900017103              CLR.B     $00017103            L06A4
011F44  102EFFFD                  MOVE.B    -3(A6),D0           
011F48  08400000                  BCHG      #0,D0               
011F4C  13C000017100              MOVE.B    D0,$00017100         L06A1
011F52  6000FF9E                  BRA       -98(PC)              L03A6


; =============================================================================
; OPCODE 0xa8 -  offset 0x1234 -  address $011f56
011F56  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011F5E  13EEFFFD00017100          MOVE.B    -3(A6),$00017100     L06A1
011F66  6000FF60                  BRA       -160(PC)             L03A5


; =============================================================================
; OPCODE 0xa9 -  offset 0x1248 -  address $011f6a
011F6A  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011F72  13EEFFFD00017100          MOVE.B    -3(A6),$00017100     L06A1
011F7A  6000FF76                  BRA       -138(PC)             L03A6


; =============================================================================
; OPCODE 0xab -  offset 0x125c -  address $011f7e
011F7E  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011F86  102EFFFD                  MOVE.B    -3(A6),D0           
011F8A  08400000                  BCHG      #0,D0               
011F8E  13C000017100              MOVE.B    D0,$00017100         L06A1
011F94  6000FF5C                  BRA       -164(PC)             L03A6


; =============================================================================
; OPCODE 0xaa -  offset 0x1276 -  address $011f98
011F98  13FC000100017103          MOVE.B    #1,$00017103         L06A4
011FA0  102EFFFD                  MOVE.B    -3(A6),D0           
011FA4  08400000                  BCHG      #0,D0               
011FA8  13C000017100              MOVE.B    D0,$00017100         L06A1
011FAE  6000FF18                  BRA       -232(PC)             L03A5
011FB2  423900017101        L03A7:CLR.B     $00017101            L06A2
011FB8  6100050C            __L03A8:
                                  BSR       1292(PC)             L03D8
011FBC  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
011FC2  1010                      MOVE.B    (A0),D0             
011FC4  6A00009A                  BPL       154(PC)              __L03AD
011FC8  B03C00FE                  CMP.B     #$FE,D0             
011FCC  67001F50                  BEQ       8016(PC)             L04C8
011FD0  4243                      CLR.W     D3                  
011FD2  16280001                  MOVE.B    1(A0),D3            
011FD6  5488                      ADDQ.L    #2,A0               
011FD8  6000007E                  BRA       126(PC)              __L03AC
011FDC  3F3900017100        L03A9:MOVE.W    $00017100,-(A7)      L06A1
011FE2  3F39000170EC              MOVE.W    $000170EC,-(A7)      L0696
011FE8  3F39000170EE              MOVE.W    $000170EE,-(A7)      L0697
011FEE  3F39000170F0              MOVE.W    $000170F0,-(A7)      L0698
011FF4  13FC000100017101          MOVE.B    #1,$00017101         L06A2
011FFC  3E18                      MOVE.W    (A0)+,D7            
011FFE  3018                      MOVE.W    (A0)+,D0            
012000  4A3900017100              TST.B     $00017100            L06A1
012006  67000004                  BEQ       4(PC)                __L03AA
01200A  4440                      NEG.W     D0                  
01200C  D179000170EC        __L03AA:
                                  ADD.W     D0,$000170EC         L0696
012012  3018                      MOVE.W    (A0)+,D0            
012014  D179000170EE              ADD.W     D0,$000170EE         L0697
01201A  3018                      MOVE.W    (A0)+,D0            
01201C  D179000170F0              ADD.W     D0,$000170F0         L0698
012022  4A47                      TST.W     D7                  
012024  6A00000E                  BPL       14(PC)               __L03AB
012028  02477FFF                  ANDI.W    #$7FFF,D7           
01202C  0879000000017100          BCHG      #0,$00017100         L06A1
012034  3F03                __L03AB:
                                  MOVE.W    D3,-(A7)            
012036  2F08                      MOVE.L    A0,-(A7)            
012038  6100FF7E                  BSR       -130(PC)             __L03A8
01203C  205F                      MOVEA.L   (A7)+,A0            
01203E  361F                      MOVE.W    (A7)+,D3            
012040  33DF000170F0              MOVE.W    (A7)+,$000170F0      L0698
012046  33DF000170EE              MOVE.W    (A7)+,$000170EE      L0697
01204C  33DF000170EC              MOVE.W    (A7)+,$000170EC      L0696
012052  33DF00017100              MOVE.W    (A7)+,$00017100      L06A1
012058  51CBFF82            __L03AC:
                                  DBF       D3,-126(PC)          L03A9
01205C  600000D8                  BRA       216(PC)              __L03B6
012060  4A3900017102        __L03AD:
                                  TST.B     $00017102            L06A3
012066  6600000C                  BNE       12(PC)               __L03AE
01206A  4A3900017101              TST.B     $00017101            L06A2
012070  67000026                  BEQ       38(PC)               __L03B1
012074  6100012A            __L03AE:
                                  BSR       298(PC)              L03BD
012078  6700000A                  BEQ       10(PC)               L03B0
01207C  61000194            __L03AF:
                                  BSR       404(PC)              L03C5
012080  60000030                  BRA       48(PC)               L03B4
012084  10311000            L03B0:MOVE.B    0(A1,D1.W),D0       
012088  67000022                  BEQ       34(PC)               __L03B3
01208C  610000E6                  BSR       230(PC)              __L03BA
012090  6700FFF2                  BEQ       -14(PC)              L03B0
012094  6000FFE6                  BRA       -26(PC)              __L03AF
012098  61000106            __L03B1:
                                  BSR       262(PC)              L03BD
01209C  67000006                  BEQ       6(PC)                __L03B2
0120A0  61000170                  BSR       368(PC)              __L03C5
0120A4  4A311000            __L03B2:
                                  TST.B     0(A1,D1.W)          
0120A8  6B000008                  BMI       8(PC)                L03B4
0120AC  13BC00021000        __L03B3:
                                  MOVE.B    #2,0(A1,D1.W)       
0120B2  61000412            L03B4:BSR       1042(PC)             L03D8
0120B6  23821012                  MOVE.L    D2,18(A1,D1.W)      
0120BA  13B9000171001012          MOVE.B    $00017100,18(A1,D1.W)  L06A1
0120C2  33AEFFD6101E              MOVE.W    -42(A6),30(A1,D1.W) 
0120C8  13AEFFD41022              MOVE.B    -44(A6),34(A1,D1.W) 
0120CE  13AEFFD31023              MOVE.B    -45(A6),35(A1,D1.W) 
0120D4  13AEFFD51020              MOVE.B    -43(A6),32(A1,D1.W) 
0120DA  13AEFFD11021              MOVE.B    -47(A6),33(A1,D1.W) 
0120E0  13AEFFDB101C              MOVE.B    -37(A6),28(A1,D1.W) 
0120E6  6B000026                  BMI       38(PC)               L03B5
0120EA  13AEFFD9101D              MOVE.B    -39(A6),29(A1,D1.W) 
0120F0  13AEFFDA101C              MOVE.B    -38(A6),28(A1,D1.W) 
0120F6  6A000016                  BPL       22(PC)               L03B5
0120FA  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
012100  D0EEFFEA                  ADDA.W    -22(A6),A0          
012104  4231101D                  CLR.B     29(A1,D1.W)         
012108  13A8001F101C              MOVE.B    31(A0),28(A1,D1.W)  
01210E  48A70C00            L03B5:MOVEM.W   D4-D5,-(A7)         
012112  4CB90070000170E4          MOVEM.W   $000170E4,D4-D6      L0693
01211A  D879000170EC              ADD.W     $000170EC,D4         L0696
012120  DA79000170EE              ADD.W     $000170EE,D5         L0697
012126  DC79000170F0              ADD.W     $000170F0,D6         L0698
01212C  48B100701016              MOVEM.W   D4-D6,22(A1,D1.W)   
012132  4C9F0030                  MOVEM.W   (A7)+,D4-D5         
012136  4A3900017101        __L03B6:
                                  TST.B     $00017101            L06A2
01213C  6600002E                  BNE       46(PC)               L03B9
012140  6100005E                  BSR       94(PC)               __L03BD
012144  6700000C                  BEQ       12(PC)               L03B8
012148  4E75                      RTS                           
01214A  61000028            __L03B7:
                                  BSR       40(PC)               L03BA
01214E  6600001C                  BNE       28(PC)               L03B9
012152  4A311000            L03B8:TST.B     0(A1,D1.W)          
012156  6600FFF2                  BNE       -14(PC)              __L03B7
01215A  61000440                  BSR       1088(PC)             L03E2
01215E  4A41                      TST.W     D1                  
012160  6700000A                  BEQ       10(PC)               L03B9
012164  61000018                  BSR       24(PC)               __L03BB
012168  6700FFE8                  BEQ       -24(PC)              L03B8
01216C  423900017102        L03B9:CLR.B     $00017102            L06A3
012172  4E75                      RTS                           
012174  3401                __L03BA:
                                  MOVE.W    D1,D2               
012176  32311004                  MOVE.W    4(A1,D1.W),D1       
01217A  6700001E                  BEQ       30(PC)               L03BC
01217E  30311002            __L03BB:
                                  MOVE.W    2(A1,D1.W),D0       
012182  B06EFFEA                  CMP.W     -22(A6),D0          
012186  66000012                  BNE       18(PC)               L03BC
01218A  10311001                  MOVE.B    1(A1,D1.W),D0       
01218E  B039000170F8              CMP.B     $000170F8,D0         L069C
012194  66000004                  BNE       4(PC)                L03BC
012198  4E75                      RTS                           
01219A  BEFC0000            L03BC:CMPA.W    #0,A7               
01219E  4E75                      RTS                           
0121A0  4242                __L03BD:
                                  CLR.W     D2                  
0121A2  322EFFE8                  MOVE.W    -24(A6),D1          
0121A6  67000034                  BEQ       52(PC)               L03C0
0121AA  362EFFEA                  MOVE.W    -22(A6),D3          
0121AE  1039000170F8              MOVE.B    $000170F8,D0         L069C
0121B4  6000000C                  BRA       12(PC)               __L03BF
0121B8  3401                L03BE:MOVE.W    D1,D2               
0121BA  32312004                  MOVE.W    4(A1,D2.W),D1       
0121BE  6700001C                  BEQ       28(PC)               L03C0
0121C2  B6711002            __L03BF:
                                  CMP.W     2(A1,D1.W),D3       
0121C6  6D00FFF0                  BLT       -16(PC)              L03BE
0121CA  66000010                  BNE       16(PC)               L03C0
0121CE  B0311001                  CMP.B     1(A1,D1.W),D0       
0121D2  6D00FFE4                  BLT       -28(PC)              L03BE
0121D6  66000004                  BNE       4(PC)                L03C0
0121DA  4E75                      RTS                           
0121DC  BEFC0000            L03C0:CMPA.W    #0,A7               
0121E0  4E75                      RTS                           
0121E2  4242                __L03C1:
                                  CLR.W     D2                  
0121E4  322EFFE8                  MOVE.W    -24(A6),D1          
0121E8  67000022                  BEQ       34(PC)               L03C4
0121EC  362EFFEA                  MOVE.W    -22(A6),D3          
0121F0  6000000C                  BRA       12(PC)               __L03C3
0121F4  3401                __L03C2:
                                  MOVE.W    D1,D2               
0121F6  32312004                  MOVE.W    4(A1,D2.W),D1       
0121FA  67000010                  BEQ       16(PC)               L03C4
0121FE  B6711002            __L03C3:
                                  CMP.W     2(A1,D1.W),D3       
012202  6D00FFF0                  BLT       -16(PC)              __L03C2
012206  66000004                  BNE       4(PC)                L03C4
01220A  4E75                      RTS                           
01220C  BEFC0000            L03C4:CMPA.W    #0,A7               
012210  4E75                      RTS                           
012212  3039000170C6        __L03C5:
                                  MOVE.W    $000170C6,D0         W_L0683
012218  67000050                  BEQ       80(PC)               __L03C8
01221C  33F10004000170C6          MOVE.W    4(A1,D0.W),$000170C6 W_L0683
012224  4A42                      TST.W     D2                  
012226  6600000A                  BNE       10(PC)               __L03C6
01222A  3D40FFE8                  MOVE.W    D0,-24(A6)          
01222E  60000006                  BRA       6(PC)                __L03C7
012232  33802004            __L03C6:
                                  MOVE.W    D0,4(A1,D2.W)       
012236  33810004            __L03C7:
                                  MOVE.W    D1,4(A1,D0.W)       
01223A  3200                      MOVE.W    D0,D1               
01223C  33AEFFEA1002              MOVE.W    -22(A6),2(A1,D1.W)  
012242  13B9000170F81001          MOVE.B    $000170F8,1(A1,D1.W) L069C
01224A  13BC00FF1000              MOVE.B    #$FF,0(A1,D1.W)     
012250  302EFFEA                  MOVE.W    -22(A6),D0          
012254  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
01225A  30300002                  MOVE.W    2(A0,D0.W),D0       
01225E  33B100061006              MOVE.W    6(A1,D0.W),6(A1,D1.W) 
012264  33810006                  MOVE.W    D1,6(A1,D0.W)       
012268  4E75                      RTS                           
01226A  203C0000000A        __L03C8:
                                  MOVE.L    #$A,D0              
012270  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x83 -  offset 0x1554 -  address $012276
012276  61002E78                  BSR       11896(PC)            FN_READ_OPNAME
01227A  3D47FFD6                  MOVE.W    D7,-42(A6)          
01227E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xc7 -  offset 0x155e -  address $012280
012280  61002E6E                  BSR       11886(PC)            FN_READ_OPNAME
012284  1D47FFDA                  MOVE.B    D7,-38(A6)          
012288  61002E66                  BSR       11878(PC)            FN_READ_OPNAME
01228C  1D47FFD9                  MOVE.B    D7,-39(A6)          
012290  4E75                      RTS                           


; =============================================================================
; OPCODE 0x9f -  offset 0x1570 -  address $012292
012292  422EFFDB                  CLR.B     -37(A6)             
012296  4E75                      RTS                           


; =============================================================================
; OPCODE 0xa0 -  offset 0x1576 -  address $012298
012298  1D7C00FFFFDB              MOVE.B    #$FF,-37(A6)        
01229E  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd0 -  offset 0x157e -  address $0122a0
0122A0  61002E4E                  BSR       11854(PC)            FN_READ_OPNAME
0122A4  103C0004                  MOVE.B    #4,D0               
0122A8  08070004                  BTST      #4,D7               
0122AC  66000028                  BNE       40(PC)               L03C9
0122B0  103C0003                  MOVE.B    #3,D0               
0122B4  08070003                  BTST      #3,D7               
0122B8  6600001C                  BNE       28(PC)               L03C9
0122BC  103C0002                  MOVE.B    #2,D0               
0122C0  08070002                  BTST      #2,D7               
0122C4  66000010                  BNE       16(PC)               L03C9
0122C8  103C0001                  MOVE.B    #1,D0               
0122CC  08070001                  BTST      #1,D7               
0122D0  66000004                  BNE       4(PC)                L03C9
0122D4  4200                      CLR.B     D0                  
0122D6  1D40FFD4            L03C9:MOVE.B    D0,-44(A6)          
0122DA  61002E14                  BSR       11796(PC)            FN_READ_OPNAME
0122DE  1D47FFD3                  MOVE.B    D7,-45(A6)          
0122E2  4E75                      RTS                           


; =============================================================================
; OPCODE 0xcf -  offset 0x15c2 -  address $0122e4
0122E4  61002E0A                  BSR       11786(PC)            FN_READ_OPNAME
0122E8  1D47FFD5                  MOVE.B    D7,-43(A6)          
0122EC  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd6 -  offset 0x15cc -  address $0122ee
0122EE  61002E00                  BSR       11776(PC)            FN_READ_OPNAME
0122F2  1D47FFD1                  MOVE.B    D7,-47(A6)          
0122F6  61002DF8                  BSR       11768(PC)            FN_READ_OPNAME
0122FA  61002DF4                  BSR       11764(PC)            FN_READ_OPNAME
0122FE  61002DF0                  BSR       11760(PC)            FN_READ_OPNAME
012302  4E75                      RTS                           
012304  48E73FA0            __L03CA:
                                  MOVEM.L   A2/A0/D2-D7,-(A7)   
012308  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
01230E  30311002                  MOVE.W    2(A1,D1.W),D0       
012312  41F00000                  LEA       0(A0,D0.W),A0       
012316  13F110120001715C          MOVE.B    18(A1,D1.W),$0001715C  L06C9
01231E  4CB100701016              MOVEM.W   22(A1,D1.W),D4-D6   
012324  98680016                  SUB.W     22(A0),D4           
012328  9A680018                  SUB.W     24(A0),D5           
01232C  9C68001A                  SUB.W     26(A0),D6           
012330  61000126                  BSR       294(PC)              __L03D6
012334  4243                      CLR.W     D3                  
012336  1028001D                  MOVE.B    29(A0),D0           
01233A  6B00004A                  BMI       74(PC)               L03CE
01233E  4A31101C                  TST.B     28(A1,D1.W)         
012342  6B000042                  BMI       66(PC)               L03CE
012346  48C4                      EXT.L     D4                  
012348  48C5                      EXT.L     D5                  
01234A  E1A4                      ASL.L     D0,D4               
01234C  E1A5                      ASL.L     D0,D5               
01234E  4A46                      TST.W     D6                  
012350  66000006                  BNE       6(PC)                __L03CB
012354  1C3C0001                  MOVE.B    #1,D6               
012358  89C6                __L03CB:
                                  DIVS      D6,D4               
01235A  8BC6                      DIVS      D6,D5               
01235C  3606                      MOVE.W    D6,D3               
01235E  1031101C                  MOVE.B    28(A1,D1.W),D0      
012362  E063                      ASR.W     D0,D3               
012364  1031101D                  MOVE.B    29(A1,D1.W),D0      
012368  4880                      EXT.W     D0                  
01236A  D640                      ADD.W     D0,D3               
01236C  6A000008                  BPL       8(PC)                __L03CC
012370  4243                      CLR.W     D3                  
012372  60000012                  BRA       18(PC)               L03CE
012376  1028001E            __L03CC:
                                  MOVE.B    30(A0),D0           
01237A  4880                      EXT.W     D0                  
01237C  B640                      CMP.W     D0,D3               
01237E  63000004                  BLS       4(PC)                __L03CD
012382  3600                      MOVE.W    D0,D3               
012384  E543                __L03CD:
                                  ASL.W     #2,D3               
012386  2448                L03CE:MOVEA.L   A0,A2               
012388  1028001C                  MOVE.B    28(A0),D0           
01238C  E064                      ASR.W     D0,D4               
01238E  E065                      ASR.W     D0,D5               
012390  D868000A                  ADD.W     10(A0),D4           
012394  DA68000C                  ADD.W     12(A0),D5           
012398  24311012                  MOVE.L    18(A1,D1.W),D2      
01239C  028200FFFFFF              ANDI.L    #$FFFFFF,D2         
0123A2  48C3                      EXT.L     D3                  
0123A4  D483                      ADD.L     D3,D2               
0123A6  2042                      MOVEA.L   D2,A0               
0123A8  D1D0                      ADDA.L    (A0),A0             
0123AA  0C100003                  CMPI.B    #3,(A0)             
0123AE  66000038                  BNE       56(PC)               __L03D2
0123B2  4A311012                  TST.B     18(A1,D1.W)         
0123B6  6700000A                  BEQ       10(PC)               __L03CF
0123BA  98680004                  SUB.W     4(A0),D4            
0123BE  60000006                  BRA       6(PC)                __L03D0
0123C2  D8680004            __L03CF:
                                  ADD.W     4(A0),D4            
0123C6  DA680006            __L03D0:
                                  ADD.W     6(A0),D5            
0123CA  30280002                  MOVE.W    2(A0),D0            
0123CE  4A280001                  TST.B     1(A0)               
0123D2  6700000A                  BEQ       10(PC)               __L03D1
0123D6  087900000001715C          BCHG      #0,$0001715C         L06C9
0123DE  E540                __L03D1:
                                  ASL.W     #2,D0               
0123E0  48C0                      EXT.L     D0                  
0123E2  D480                      ADD.L     D0,D2               
0123E4  2042                      MOVEA.L   D2,A0               
0123E6  D1D0                      ADDA.L    (A0),A0             
0123E8  10311022            __L03D2:
                                  MOVE.B    34(A1,D1.W),D0      
0123EC  67000036                  BEQ       54(PC)               __L03D4
0123F0  4A311023                  TST.B     35(A1,D1.W)         
0123F4  6B00002A                  BMI       42(PC)               L03D3
0123F8  E44B                      LSR.W     #2,D3               
0123FA  B6311023                  CMP.B     35(A1,D1.W),D3      
0123FE  63000020                  BLS       32(PC)               L03D3
012402  1E2A001E                  MOVE.B    30(A2),D7           
012406  9E311023                  SUB.B     35(A1,D1.W),D7      
01240A  4887                      EXT.W     D7                  
01240C  6B000012                  BMI       18(PC)               L03D3
012410  6700000E                  BEQ       14(PC)               L03D3
012414  96311023                  SUB.B     35(A1,D1.W),D3      
012418  4883                      EXT.W     D3                  
01241A  4880                      EXT.W     D0                  
01241C  C0C3                      MULU      D3,D0               
01241E  80C7                      DIVU      D7,D0               
012420  E06C                L03D3:LSR.W     D0,D4               
012422  E16C                      LSL.W     D0,D4               
012424  23C200017152        __L03D4:
                                  MOVE.L    D2,$00017152         L06C5
01242A  30280002                  MOVE.W    2(A0),D0            
01242E  E248                      LSR.W     #1,D0               
012430  9840                      SUB.W     D0,D4               
012432  30280004                  MOVE.W    4(A0),D0            
012436  E248                      LSR.W     #1,D0               
012438  9A40                      SUB.W     D0,D5               
01243A  48B9007000017156          MOVEM.W   D4-D6,$00017156      L06C6
012442  4CDF05FC                  MOVEM.L   (A7)+,A2/A0/D2-D7   
012446  4E75                      RTS                           
012448  3F05                __L03D5:
                                  MOVE.W    D5,-(A7)            
01244A  10280024                  MOVE.B    36(A0),D0           
01244E  4880                      EXT.W     D0                  
012450  CBC0                      MULS      D0,D5               
012452  9A46                      SUB.W     D6,D5               
012454  3C1F                      MOVE.W    (A7)+,D6            
012456  4E75                      RTS                           
012458  4A280001            __L03D6:
                                  TST.B     1(A0)               
01245C  6A00FFEA                  BPL       -22(PC)              __L03D5
012460  48E7F000                  MOVEM.L   D0-D3,-(A7)         
012464  3204                      MOVE.W    D4,D1               
012466  3405                      MOVE.W    D5,D2               
012468  3606                      MOVE.W    D6,D3               
01246A  18280020                  MOVE.B    32(A0),D4           
01246E  4884                      EXT.W     D4                  
012470  C9C1                      MULS      D1,D4               
012472  10280021                  MOVE.B    33(A0),D0           
012476  4880                      EXT.W     D0                  
012478  C1C2                      MULS      D2,D0               
01247A  D840                      ADD.W     D0,D4               
01247C  10280022                  MOVE.B    34(A0),D0           
012480  4880                      EXT.W     D0                  
012482  C1C3                      MULS      D3,D0               
012484  D840                      ADD.W     D0,D4               
012486  1A280023                  MOVE.B    35(A0),D5           
01248A  4885                      EXT.W     D5                  
01248C  CBC1                      MULS      D1,D5               
01248E  10280024                  MOVE.B    36(A0),D0           
012492  4880                      EXT.W     D0                  
012494  C1C2                      MULS      D2,D0               
012496  DA40                      ADD.W     D0,D5               
012498  10280025                  MOVE.B    37(A0),D0           
01249C  4880                      EXT.W     D0                  
01249E  C1C3                      MULS      D3,D0               
0124A0  DA40                      ADD.W     D0,D5               
0124A2  1C280026                  MOVE.B    38(A0),D6           
0124A6  4886                      EXT.W     D6                  
0124A8  CDC1                      MULS      D1,D6               
0124AA  10280027                  MOVE.B    39(A0),D0           
0124AE  4880                      EXT.W     D0                  
0124B0  C1C2                      MULS      D2,D0               
0124B2  DC40                      ADD.W     D0,D6               
0124B4  10280028                  MOVE.B    40(A0),D0           
0124B8  4880                      EXT.W     D0                  
0124BA  C1C3                      MULS      D3,D0               
0124BC  DC40                      ADD.W     D0,D6               
0124BE  4CDF000F                  MOVEM.L   (A7)+,D0-D3         
0124C2  4E75                      RTS                           
0124C4  4E75                L03D7:RTS                           
0124C6  4A3900017103        L03D8:TST.B     $00017103            L06A4
0124CC  66000030                  BNE       48(PC)               __L03DA
0124D0  2F01                __L03D9:
                                  MOVE.L    D1,-(A7)            
0124D2  206EFFEC                  MOVEA.L   -20(A6),A0          
0124D6  D1E8000E                  ADDA.L    14(A0),A0           
0124DA  D1E80000                  ADDA.L    0(A0),A0            
0124DE  3207                      MOVE.W    D7,D1               
0124E0  D241                      ADD.W     D1,D1               
0124E2  D241                      ADD.W     D1,D1               
0124E4  41F01000                  LEA       0(A0,D1.W),A0       
0124E8  2408                      MOVE.L    A0,D2               
0124EA  D1D0                      ADDA.L    (A0),A0             
0124EC  2008                      MOVE.L    A0,D0               
0124EE  221F                      MOVE.L    (A7)+,D1            
0124F0  4E75                      RTS                           
0124F2  203C0000000F              MOVE.L    #$F,D0              
0124F8  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
0124FE  2F0E                __L03DA:
                                  MOVE.L    A6,-(A7)            
012500  2C79000170E0              MOVEA.L   $000170E0,A6         L_L0692
012506  6100FFC8                  BSR       -56(PC)              __L03D9
01250A  2C5F                      MOVEA.L   (A7)+,A6            
01250C  4E75                      RTS                           


; =============================================================================
; OPCODE 0xdd -  offset 0x17ec -  address $01250e
01250E  13FC000100017081          MOVE.B    #1,$00017081         B_066D


; =============================================================================
; OPCODE 0x4b -  offset 0x17f4 -  address $012516
; cerasen
012516  61002BD6                  BSR       11222(PC)            FN_READ_OPNAME_SAVE_D7
01251A  13C7000170F8              MOVE.B    D7,$000170F8         L069C
012520  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
012526  6100FC78            __L03DB:
                                  BSR       -904(PC)             L03BD
01252A  6700000A                  BEQ       10(PC)               __L03DC
01252E  423900017081              CLR.B     $00017081            B_066D
012534  4E75                      RTS                           
012536  61000064            __L03DC:
                                  BSR       100(PC)              L03E2
01253A  6000FFEA                  BRA       -22(PC)              __L03DB


; =============================================================================
; OPCODE 0xdb -  offset 0x181c -  address $01253e
01253E  13FC000100017081          MOVE.B    #1,$00017081         B_066D


; =============================================================================
; OPCODE 0x4a -  offset 0x1824 -  address $012546
012546  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
01254C  6100FC94                  BSR       -876(PC)             __L03C1
012550  66000018                  BNE       24(PC)               L03DE
012554  61000046            __L03DD:
                                  BSR       70(PC)               L03E2
012558  4A41                      TST.W     D1                  
01255A  6700000E                  BEQ       14(PC)               L03DE
01255E  302EFFEA                  MOVE.W    -22(A6),D0          
012562  B0711002                  CMP.W     2(A1,D1.W),D0       
012566  6700FFEC                  BEQ       -20(PC)              __L03DD
01256A  423900017081        L03DE:CLR.B     $00017081            B_066D
012570  4E75                      RTS                           


; =============================================================================
; OPCODE 0xdc -  offset 0x1850 -  address $012572
012572  13FC000100017081          MOVE.B    #1,$00017081         B_066D


; =============================================================================
; OPCODE 0x50 -  offset 0x1858 -  address $01257a
01257A  2279000170C2        __L03DF:
                                  MOVEA.L   $000170C2,A1         L_L0682
012580  4242                      CLR.W     D2                  
012582  322EFFE8                  MOVE.W    -24(A6),D1          
012586  6700000C            __L03E0:
                                  BEQ       12(PC)               L03E1
01258A  61000010                  BSR       16(PC)               L03E2
01258E  4A41                      TST.W     D1                  
012590  6600FFF4                  BNE       -12(PC)              __L03E0
012594  423900017081        L03E1:CLR.B     $00017081            B_066D
01259A  4E75                      RTS                           
01259C  4A3900017081        L03E2:TST.B     $00017081            B_066D
0125A2  6600002A                  BNE       42(PC)               L03E4
0125A6  10311000                  MOVE.B    0(A1,D1.W),D0       
0125AA  6B000022                  BMI       34(PC)               L03E4
0125AE  13BC00011000              MOVE.B    #1,0(A1,D1.W)       
0125B4  4A42                      TST.W     D2                  
0125B6  6600000C                  BNE       12(PC)               __L03E3
0125BA  3D711004FFE8              MOVE.W    4(A1,D1.W),-24(A6)  
0125C0  60000034                  BRA       52(PC)               L03E7
0125C4  33B110042004        __L03E3:
                                  MOVE.W    4(A1,D1.W),4(A1,D2.W) 
0125CA  6000002A                  BRA       42(PC)               L03E7
0125CE  4A42                L03E4:TST.W     D2                  
0125D0  6600000C                  BNE       12(PC)               __L03E5
0125D4  3D711004FFE8              MOVE.W    4(A1,D1.W),-24(A6)  
0125DA  60000008                  BRA       8(PC)                __L03E6
0125DE  33B110042004        __L03E5:
                                  MOVE.W    4(A1,D1.W),4(A1,D2.W) 
0125E4  33B9000170C61004    __L03E6:
                                  MOVE.W    $000170C6,4(A1,D1.W) W_L0683
0125EC  33C1000170C6              MOVE.W    D1,$000170C6         W_L0683
0125F2  61000014                  BSR       20(PC)               __L03E9
0125F6  4A42                L03E7:TST.W     D2                  
0125F8  66000008                  BNE       8(PC)                __L03E8
0125FC  322EFFE8                  MOVE.W    -24(A6),D1          
012600  4E75                      RTS                           
012602  32312004            __L03E8:
                                  MOVE.W    4(A1,D2.W),D1       
012606  4E75                      RTS                           
012608  30311002            __L03E9:
                                  MOVE.W    2(A1,D1.W),D0       
01260C  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
012612  B2700002                  CMP.W     2(A0,D0.W),D1       
012616  6600000A                  BNE       10(PC)               __L03EA
01261A  31B110060002              MOVE.W    6(A1,D1.W),2(A0,D0.W) 
012620  4E75                      RTS                           
012622  30300002            __L03EA:
                                  MOVE.W    2(A0,D0.W),D0       
012626  60000006                  BRA       6(PC)                __L03EC
01262A  30310006            __L03EB:
                                  MOVE.W    6(A1,D0.W),D0       
01262E  B2710006            __L03EC:
                                  CMP.W     6(A1,D0.W),D1       
012632  6600FFF6                  BNE       -10(PC)              __L03EB
012636  33B110060006              MOVE.W    6(A1,D1.W),6(A1,D0.W) 
01263C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x4c -  offset 0x191c -  address $01263e
01263E  61002AB0                  BSR       10928(PC)            FN_READ_OPNAME
012642  3D470000                  MOVE.W    D7,0(A6)            
012646  61002AA8                  BSR       10920(PC)            FN_READ_OPNAME
01264A  3D470002                  MOVE.W    D7,2(A6)            
01264E  61002AA0                  BSR       10912(PC)            FN_READ_OPNAME
012652  3D470004                  MOVE.W    D7,4(A6)            
012656  4E75                      RTS                           


; =============================================================================
; OPCODE 0x4d -  offset 0x1936 -  address $012658
012658  61002A96                  BSR       10902(PC)            FN_READ_OPNAME
01265C  DF6E0000                  ADD.W     D7,0(A6)            
012660  61002A8E                  BSR       10894(PC)            FN_READ_OPNAME
012664  DF6E0002                  ADD.W     D7,2(A6)            
012668  61002A86                  BSR       10886(PC)            FN_READ_OPNAME
01266C  DF6E0004                  ADD.W     D7,4(A6)            
012670  4E75                      RTS                           


; =============================================================================
; OPCODE 0x90 -  offset 0x1950 -  address $012672
012672  102E0009                  MOVE.B    9(A6),D0            
012676  4880                      EXT.W     D0                  
012678  D16E0000                  ADD.W     D0,0(A6)            
01267C  102E000A                  MOVE.B    10(A6),D0           
012680  4880                      EXT.W     D0                  
012682  D16E0002                  ADD.W     D0,2(A6)            
012686  102E000B                  MOVE.B    11(A6),D0           
01268A  4880                      EXT.W     D0                  
01268C  D16E0004                  ADD.W     D0,4(A6)            
012690  4E75                      RTS                           


; =============================================================================
; OPCODE 0xa2 -  offset 0x1970 -  address $012692
012692  61000CC8                  BSR       3272(PC)             L0463
012696  D16E0000                  ADD.W     D0,0(A6)            
01269A  D36E0002                  ADD.W     D1,2(A6)            
01269E  D56E0004                  ADD.W     D2,4(A6)            
0126A2  4E75                      RTS                           
0126A4  61002A4A                  BSR       10826(PC)            FN_READ_OPNAME
0126A8  3F07                      MOVE.W    D7,-(A7)            
0126AA  61002A44                  BSR       10820(PC)            FN_READ_OPNAME
0126AE  3F07                      MOVE.W    D7,-(A7)            
0126B0  61002A3E                  BSR       10814(PC)            FN_READ_OPNAME
0126B4  321F                      MOVE.W    (A7)+,D1            
0126B6  341F                      MOVE.W    (A7)+,D2            
0126B8  3607                      MOVE.W    D7,D3               
0126BA  182E0009                  MOVE.B    9(A6),D4            
0126BE  4884                      EXT.W     D4                  
0126C0  1A2E000A                  MOVE.B    10(A6),D5           
0126C4  4885                      EXT.W     D5                  
0126C6  1C2E000B                  MOVE.B    11(A6),D6           
0126CA  4886                      EXT.W     D6                  
0126CC  6100FDF6                  BSR       -522(PC)             L03D7
0126D0  1D440009                  MOVE.B    D4,9(A6)            
0126D4  1D45000A                  MOVE.B    D5,10(A6)           
0126D8  1D46000B                  MOVE.B    D6,11(A6)           
0126DC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x5f -  offset 0x19bc -  address $0126de
0126DE  61002A10                  BSR       10768(PC)            FN_READ_OPNAME
0126E2  4A6EFFE0                  TST.W     -32(A6)             
0126E6  67000026                  BEQ       38(PC)               __L03EE
0126EA  206EFFEC                  MOVEA.L   -20(A6),A0          
0126EE  D0EEFFE0                  ADDA.W    -32(A6),A0          
0126F2  5288                      ADDQ.L    #1,A0               
0126F4  3007                __L03ED:
                                  MOVE.W    D7,D0               
0126F6  D040                      ADD.W     D0,D0               
0126F8  D047                      ADD.W     D7,D0               
0126FA  1D7000000009              MOVE.B    0(A0,D0.W),9(A6)    
012700  1D700001000A              MOVE.B    1(A0,D0.W),10(A6)   
012706  1D700002000B              MOVE.B    2(A0,D0.W),11(A6)   
01270C  4E75                      RTS                           
01270E  207C00015015        __L03EE:
                                  MOVEA.L   #$00015015,A0        L0541
012714  6000FFDE                  BRA       -34(PC)              __L03ED
012718  23EE0000000170E4    L03EF:MOVE.L    0(A6),$000170E4      L0693
012720  23EE0004000170E8          MOVE.L    4(A6),$000170E8      L0695
012728  4E75                      RTS                           
01272A  48E7FFE0            __L03F0:
                                  MOVEM.L   A0-A2/D0-D7,-(A7)   
01272E  382E0000                  MOVE.W    0(A6),D4            
012732  9879000170E4              SUB.W     $000170E4,D4         L0693
012738  3A2E0002                  MOVE.W    2(A6),D5            
01273C  9A79000170E6              SUB.W     $000170E6,D5         L0694
012742  3C2E0004                  MOVE.W    4(A6),D6            
012746  9C79000170E8              SUB.W     $000170E8,D6         L0695
01274C  66000012                  BNE       18(PC)               L03F1
012750  4A44                      TST.W     D4                  
012752  6600000C                  BNE       12(PC)               L03F1
012756  4A45                      TST.W     D5                  
012758  66000006                  BNE       6(PC)                L03F1
01275C  60000032                  BRA       50(PC)               L03F4
012760  2279000170C2        L03F1:MOVEA.L   $000170C2,A1         L_L0682
012766  322EFFE8                  MOVE.W    -24(A6),D1          
01276A  67000024                  BEQ       36(PC)               L03F4
01276E  4A311000            __L03F2:
                                  TST.B     0(A1,D1.W)          
012772  66000008                  BNE       8(PC)                __L03F3
012776  13BC00021000              MOVE.B    #2,0(A1,D1.W)       
01277C  D9711016            __L03F3:
                                  ADD.W     D4,22(A1,D1.W)      
012780  DB711018                  ADD.W     D5,24(A1,D1.W)      
012784  DD71101A                  ADD.W     D6,26(A1,D1.W)      
012788  32311004                  MOVE.W    4(A1,D1.W),D1       
01278C  6600FFE0                  BNE       -32(PC)              __L03F2
012790  4CDF07FF            L03F4:MOVEM.L   (A7)+,A0-A2/D0-D7   
012794  4E75                      RTS                           
012796  4CAA0070002A        L03F5:MOVEM.W   42(A2),D4-D6        
01279C  D96A0016                  ADD.W     D4,22(A2)           
0127A0  DB6A0018                  ADD.W     D5,24(A2)           
0127A4  DD6A001A                  ADD.W     D6,26(A2)           
0127A8  3203                      MOVE.W    D3,D1               
0127AA  32311006            L03F6:MOVE.W    6(A1,D1.W),D1       
0127AE  6700001C                  BEQ       28(PC)               __L03F7
0127B2  4A311021                  TST.B     33(A1,D1.W)         
0127B6  6B00FFF2                  BMI       -14(PC)              L03F6
0127BA  4A311000                  TST.B     0(A1,D1.W)          
0127BE  6600FFEA                  BNE       -22(PC)              L03F6
0127C2  13BC00021000              MOVE.B    #2,0(A1,D1.W)       
0127C8  6000FFE0                  BRA       -32(PC)              L03F6
0127CC  08AA00070000        __L03F7:
                                  BCLR      #7,0(A2)            
0127D2  4E75                      RTS                           
0127D4  4A390001707C        __L03F8:
                                  TST.B     $0001707C            B_IS_AMIGA
0127DA  67000008                  BEQ       8(PC)                __L03F9
0127DE  4EB90000B640              JSR       $0000B640            __L0054
0127E4  4A390001712D        __L03F9:
                                  TST.B     $0001712D            L06BA
0127EA  6700001E                  BEQ       30(PC)               L03FA
0127EE  4EB90000B642              JSR       $0000B642            L0055
0127F4  4A39000170FE              TST.B     $000170FE            L06A0
0127FA  6700000E                  BEQ       14(PC)               L03FA
0127FE  4239000170FE              CLR.B     $000170FE            L06A0
012804  4EB90000EF74              JSR       $0000EF74            __L0268
01280A  10390001718A        L03FA:MOVE.B    $0001718A,D0         L06DB
012810  B0390001718B        __L03FB:
                                  CMP.B     $0001718B,D0         L06DC
012816  6200FFF8                  BHI       -8(PC)               __L03FB
01281A  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
01281E  42390001718B              CLR.B     $0001718B            L06DC
012824  4A390001712D              TST.B     $0001712D            L06BA
01282A  6700007A                  BEQ       122(PC)              __L0401
01282E  4A39000170CF              TST.B     $000170CF            L0688
012834  6B000060                  BMI       96(PC)               __L03FF
012838  4A39000170D1        __L03FC:
                                  TST.B     $000170D1            L068A
01283E  6600FFF8                  BNE       -8(PC)               __L03FC
012842  5239000170D0              ADDQ.B    #1,$000170D0         L0689
012848  67000008                  BEQ       8(PC)                __L03FD
01284C  5339000170D0              SUBQ.B    #1,$000170D0         L0689
012852  4EB90001301A        __L03FD:
                                  JSR       $0001301A            L0447
012858  13FC0001000170D1          MOVE.B    #1,$000170D1         L068A
012860  0C390002000170CF          CMPI.B    #2,$000170CF         L0688
012868  67000008                  BEQ       8(PC)                __L03FE
01286C  4EB90000D2CA              JSR       $0000D2CA            L0180
012872  4239000170D1        __L03FE:
                                  CLR.B     $000170D1            L068A
012878  13FC00FF000170D0          MOVE.B    #$FF,$000170D0       L0689
012880  4A39000170CF              TST.B     $000170CF            L0688
012886  66000014                  BNE       20(PC)               L0400
01288A  13FC00FF000170CF          MOVE.B    #$FF,$000170CF       L0688
012892  60000008                  BRA       8(PC)                L0400
012896  4EB90001301A        __L03FF:
                                  JSR       $0001301A            L0447
01289C  23FC00017A8400017130L0400:MOVE.L    #$00017A84,$00017130 L0723 L06BC
0128A6  42390001712F        __L0401:
                                  CLR.B     $0001712F            L06BB
0128AC  23F90000AC5200017224      MOVE.L    $0000AC52,$00017224  L000A L070B
0128B6  23F90001723200017228      MOVE.L    $00017232,$00017228  L0710 L070C
0128C0  23F9000172360001722C      MOVE.L    $00017236,$0001722C  L0712 L070E
0128CA  33F90001723A00017230      MOVE.W    $0001723A,$00017230  L0714 L070F
0128D4  303900017104              MOVE.W    $00017104,D0         W_06A5
0128DA  60000006                  BRA       6(PC)                __L0403
0128DE  302A0004            L0402:MOVE.W    4(A2),D0            
0128E2  67000022            __L0403:
                                  BEQ       34(PC)               L0404
0128E6  2479000170E0              MOVEA.L   $000170E0,A2         L_L0692
0128EC  45F20000                  LEA       0(A2,D0.W),A2       
0128F0  082A00060000              BTST      #6,0(A2)            
0128F6  6600FFE6                  BNE       -26(PC)              L0402
0128FA  362A0002                  MOVE.W    2(A2),D3            
0128FE  61000088                  BSR       136(PC)              __L0408
012902  6000FFDA                  BRA       -38(PC)              L0402
012906  4239000170FD        L0404:CLR.B     $000170FD            L069F
01290C  4A390001712D              TST.B     $0001712D            L06BA
012912  66000006                  BNE       6(PC)                L0405
012916  6000006A                  BRA       106(PC)              L0407
01291A  4A390001712F        L0405:TST.B     $0001712F            L06BB
012920  67000060                  BEQ       96(PC)               L0407
012924  4A39000170CF              TST.B     $000170CF            L0688
01292A  6700001A                  BEQ       26(PC)               L0406
01292E  6B000016                  BMI       22(PC)               L0406
012932  5239000170D0              ADDQ.B    #1,$000170D0         L0689
012938  6700000C                  BEQ       12(PC)               L0406
01293C  5339000170D0              SUBQ.B    #1,$000170D0         L0689
012942  6000FFD6                  BRA       -42(PC)              L0405
012946  20790000AC4E        L0406:MOVEA.L   $0000AC4E,A0         L0009
01294C  22790000AC52              MOVEA.L   $0000AC52,A1         L000A
012952  23C90000AC4E              MOVE.L    A1,$0000AC4E         L0009
012958  23C80000AC52              MOVE.L    A0,$0000AC52         L000A
01295E  4EB90000B64E              JSR       $0000B64E            L0056
012964  4A39000170CF              TST.B     $000170CF            L0688
01296A  67000016                  BEQ       22(PC)               L0407
01296E  6B000012                  BMI       18(PC)               L0407
012972  13FC0001000170D1          MOVE.B    #1,$000170D1         L068A
01297A  13FC00FF000170D0          MOVE.B    #$FF,$000170D0       L0689
012982  4CDF7FFF            L0407:MOVEM.L   (A7)+,A0-A6/D0-D7   
012986  4E75                      RTS                           
012988  48E7FFFE            __L0408:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
01298C  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
012992  4A39000170FD              TST.B     $000170FD            L069F
012998  6600000A                  BNE       10(PC)               __L0409
01299C  4A2A0000                  TST.B     0(A2)               
0129A0  6A000006                  BPL       6(PC)                __L040A
0129A4  6100FDF0            __L0409:
                                  BSR       -528(PC)             L03F5
0129A8  42390001720B        __L040A:
                                  CLR.B     $0001720B            L0701
0129AE  082A00020001              BTST      #2,1(A2)            
0129B4  6700000A                  BEQ       10(PC)               __L040B
0129B8  13FC00010001720B          MOVE.B    #1,$0001720B         L0701
0129C0  42390001723D        __L040B:
                                  CLR.B     $0001723D            L0716
0129C6  082A00050000              BTST      #5,0(A2)            
0129CC  6700001C                  BEQ       28(PC)               L040C
0129D0  13FC00010001723D          MOVE.B    #1,$0001723D         L0716
0129D8  53390001723C              SUBQ.B    #1,$0001723C         L0715
0129DE  6600000A                  BNE       10(PC)               L040C
0129E2  13FC00FF0001723D          MOVE.B    #$FF,$0001723D       L0716
0129EA  082A00040001        L040C:BTST      #4,1(A2)            
0129F0  66000078                  BNE       120(PC)              __L0410
0129F4  3203                      MOVE.W    D3,D1               
0129F6  3401                L040D:MOVE.W    D1,D2               
0129F8  32311006                  MOVE.W    6(A1,D1.W),D1       
0129FC  67000024                  BEQ       36(PC)               L040E
012A00  10311000                  MOVE.B    0(A1,D1.W),D0       
012A04  6700FFF0                  BEQ       -16(PC)              L040D
012A08  423900017106              CLR.B     $00017106            L06A6
012A0E  B03C0002                  CMP.B     #2,D0               
012A12  67000262                  BEQ       610(PC)              __L0420
012A16  B03C00FF                  CMP.B     #$FF,D0             
012A1A  67000244                  BEQ       580(PC)              __L041F
012A1E  60000274                  BRA       628(PC)              __L0422
012A22  4A390001723D        L040E:TST.B     $0001723D            L0716
012A28  6A00003A                  BPL       58(PC)               __L040F
012A2C  08AA00050000              BCLR      #5,0(A2)            
012A32  33EA000E00017108          MOVE.W    14(A2),$00017108     L06A8
012A3A  33EA00100001710A          MOVE.W    16(A2),$0001710A     L06A9
012A42  302A000E                  MOVE.W    14(A2),D0           
012A46  D06A0012                  ADD.W     18(A2),D0           
012A4A  33C00001710C              MOVE.W    D0,$0001710C         L06AA
012A50  302A0010                  MOVE.W    16(A2),D0           
012A54  D06A0014                  ADD.W     20(A2),D0           
012A58  33C00001710E              MOVE.W    D0,$0001710E         L06AB
012A5E  4EB90000EA86              JSR       $0000EA86            __L0247
012A64  4CDF7FFF            __L040F:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
012A68  4E75                      RTS                           
012A6A  302A000E            __L0410:
                                  MOVE.W    14(A2),D0           
012A6E  322A0010                  MOVE.W    16(A2),D1           
012A72  33C000017124              MOVE.W    D0,$00017124         L06B5
012A78  33C100017126              MOVE.W    D1,$00017126         L06B6
012A7E  33C000017108              MOVE.W    D0,$00017108         L06A8
012A84  33C10001710A              MOVE.W    D1,$0001710A         L06A9
012A8A  D06A0012                  ADD.W     18(A2),D0           
012A8E  D26A0014                  ADD.W     20(A2),D1           
012A92  33C000017128              MOVE.W    D0,$00017128         L06B7
012A98  33C10001712A              MOVE.W    D1,$0001712A         L06B8
012A9E  33C00001710C              MOVE.W    D0,$0001710C         L06AA
012AA4  33C10001710E              MOVE.W    D1,$0001710E         L06AB
012AAA  907900017108              SUB.W     $00017108,D0         L06A8
012AB0  5240                      ADDQ.W    #1,D0               
012AB2  E448                      LSR.W     #2,D0               
012AB4  33C000017110              MOVE.W    D0,$00017110         L06AC
012ABA  082A00060001              BTST      #6,1(A2)            
012AC0  66000008                  BNE       8(PC)                __L0411
012AC4  4EB90000E4F4              JSR       $0000E4F4            L021D
012ACA  3203                __L0411:
                                  MOVE.W    D3,D1               
012ACC  3401                L0412:MOVE.W    D1,D2               
012ACE  32311006                  MOVE.W    6(A1,D1.W),D1       
012AD2  67000016                  BEQ       22(PC)               __L0413
012AD6  10311000                  MOVE.B    0(A1,D1.W),D0       
012ADA  6700FFF0                  BEQ       -16(PC)              L0412
012ADE  B03C0001                  CMP.B     #1,D0               
012AE2  670000F2                  BEQ       242(PC)              __L041B
012AE6  600000BA                  BRA       186(PC)              __L041A
012AEA  48E71020            __L0413:
                                  MOVEM.L   A2/D3,-(A7)         
012AEE  363900017104              MOVE.W    $00017104,D3         W_06A5
012AF4  6000000C                  BRA       12(PC)               __L0415
012AF8  2079000170E0        L0414:MOVEA.L   $000170E0,A0         L_L0692
012AFE  36303004                  MOVE.W    4(A0,D3.W),D3       
012B02  6700007C            __L0415:
                                  BEQ       124(PC)              L0419
012B06  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
012B0C  083000063000              BTST      #6,0(A0,D3.W)       
012B12  6600FFE4                  BNE       -28(PC)              L0414
012B16  083000033001              BTST      #3,1(A0,D3.W)       
012B1C  67000006                  BEQ       6(PC)                __L0416
012B20  45F03000                  LEA       0(A0,D3.W),A2       
012B24  32303002            __L0416:
                                  MOVE.W    2(A0,D3.W),D1       
012B28  6700FFCE                  BEQ       -50(PC)              L0414
012B2C  48E75080                  MOVEM.L   A0/D3/D1,-(A7)      
012B30  61000524                  BSR       1316(PC)             L044A
012B34  4CDF010A                  MOVEM.L   (A7)+,A0/D3/D1      
012B38  4A390001712C              TST.B     $0001712C            L06B9
012B3E  6700FFB8                  BEQ       -72(PC)              L0414
012B42  32311006            L0417:MOVE.W    6(A1,D1.W),D1       
012B46  6700FFB0                  BEQ       -80(PC)              L0414
012B4A  41F11000                  LEA       0(A1,D1.W),A0       
012B4E  4A280000                  TST.B     0(A0)               
012B52  6B00FFEE                  BMI       -18(PC)              L0417
012B56  4A280008                  TST.B     8(A0)               
012B5A  6B00FFE6                  BMI       -26(PC)              L0417
012B5E  4A680010                  TST.W     16(A0)              
012B62  6B00FFDE                  BMI       -34(PC)              L0417
012B66  48E75060                  MOVEM.L   A1-A2/D3/D1,-(A7)   
012B6A  4EB90000D3C0              JSR       $0000D3C0            L0192
012B70  4CDF060A                  MOVEM.L   (A7)+,A1-A2/D3/D1   
012B74  13FC00010001712F          MOVE.B    #1,$0001712F         L06BB
012B7C  6000FFC4            __L0418:
                                  BRA       -60(PC)              L0417
012B80  4CDF0408            L0419:MOVEM.L   (A7)+,A2/D3         
012B84  4A390001712D              TST.B     $0001712D            L06BA
012B8A  6600FE96                  BNE       -362(PC)             L040E
012B8E  4A390001723D              TST.B     $0001723D            L0716
012B94  6600FE8C                  BNE       -372(PC)             L040E
012B98  4EB90000E10E              JSR       $0000E10E            L0203
012B9E  6000FE82                  BRA       -382(PC)             L040E
012BA2  6100F760            __L041A:
                                  BSR       -2208(PC)            L03CA
012BA6  33B110062006              MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012BAC  23B9000171521008          MOVE.L    $00017152,8(A1,D1.W) L06C5
012BB4  23B900017156100C          MOVE.L    $00017156,12(A1,D1.W)  L06C6
012BBC  33B90001715A1010          MOVE.W    $0001715A,16(A1,D1.W)  L06C8
012BC4  13B90001715C1008          MOVE.B    $0001715C,8(A1,D1.W) L06C9
012BCC  6100022A                  BSR       554(PC)              L0431
012BD0  3202                      MOVE.W    D2,D1               
012BD2  6000FEF8                  BRA       -264(PC)             L0412
012BD6  33B110062006        __L041B:
                                  MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012BDC  33B9000170C61004          MOVE.W    $000170C6,4(A1,D1.W) W_L0683
012BE4  33C1000170C6              MOVE.W    D1,$000170C6         W_L0683
012BEA  3202                      MOVE.W    D2,D1               
012BEC  6000FEDE                  BRA       -290(PC)             L0412
012BF0  6100F712                  BSR       -2286(PC)            __L03CA
012BF4  33B110062006        __L041C:
                                  MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012BFA  23B9000171521008    __L041D:
                                  MOVE.L    $00017152,8(A1,D1.W) L06C5
012C02  23B900017156100C          MOVE.L    $00017156,12(A1,D1.W)  L06C6
012C0A  33B90001715A1010          MOVE.W    $0001715A,16(A1,D1.W)  L06C8
012C12  13B90001715C1008          MOVE.B    $0001715C,8(A1,D1.W) L06C9
012C1A  610001DC                  BSR       476(PC)              L0431
012C1E  61000244                  BSR       580(PC)              L0435
012C22  3202                      MOVE.W    D2,D1               
012C24  6000FDD0                  BRA       -560(PC)             L040D
012C28  6100F6DA            __L041E:
                                  BSR       -2342(PC)            L03CA
012C2C  610004DC                  BSR       1244(PC)             __L0453
012C30  6600FFC2                  BNE       -62(PC)              __L041C
012C34  33B110062006              MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012C3A  61000228                  BSR       552(PC)              L0435
012C3E  6000FFBA                  BRA       -70(PC)              __L041D
012C42  33B110062006              MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012C48  33B9000170C61004          MOVE.W    $000170C6,4(A1,D1.W) W_L0683
012C50  33C1000170C6              MOVE.W    D1,$000170C6         W_L0683
012C56  6100020C                  BSR       524(PC)              L0435
012C5A  3202                      MOVE.W    D2,D1               
012C5C  6000FD98                  BRA       -616(PC)             L040D
012C60  48E76800            __L041F:
                                  MOVEM.L   D4/D1-D2,-(A7)      
012C64  61000104                  BSR       260(PC)              L042B
012C68  61000052                  BSR       82(PC)               L0424
012C6C  4A44                      TST.W     D4                  
012C6E  6A0000A6                  BPL       166(PC)              L0427
012C72  60000032                  BRA       50(PC)               __L0423
012C76  48E76800            __L0420:
                                  MOVEM.L   D4/D1-D2,-(A7)      
012C7A  610000EE                  BSR       238(PC)              L042B
012C7E  4A44                      TST.W     D4                  
012C80  6B00000A                  BMI       10(PC)               __L0421
012C84  61000088                  BSR       136(PC)              L0426
012C88  6000008C                  BRA       140(PC)              L0427
012C8C  4CDF0016            __L0421:
                                  MOVEM.L   (A7)+,D4/D1-D2      
012C90  6000FF96                  BRA       -106(PC)             __L041E
012C94  48E76800            __L0422:
                                  MOVEM.L   D4/D1-D2,-(A7)      
012C98  610000D0                  BSR       208(PC)              L042B
012C9C  61000054                  BSR       84(PC)               __L0425
012CA0  4A44                      TST.W     D4                  
012CA2  6A000072                  BPL       114(PC)              L0427
012CA6  4CDF0016            __L0423:
                                  MOVEM.L   (A7)+,D4/D1-D2      
012CAA  13FC000100017106          MOVE.B    #1,$00017106         L06A6
012CB2  610001B0                  BSR       432(PC)              L0435
012CB6  3202                      MOVE.W    D2,D1               
012CB8  6000FD3C                  BRA       -708(PC)             L040D
012CBC  6100F646            L0424:BSR       -2490(PC)            L03CA
012CC0  33B110062006              MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012CC6  23B9000171521008          MOVE.L    $00017152,8(A1,D1.W) L06C5
012CCE  23B900017156100C          MOVE.L    $00017156,12(A1,D1.W)  L06C6
012CD6  33B90001715A1010          MOVE.W    $0001715A,16(A1,D1.W)  L06C8
012CDE  13B90001715C1008          MOVE.B    $0001715C,8(A1,D1.W) L06C9
012CE6  610000A8                  BSR       168(PC)              __L042C
012CEA  6100010C                  BSR       268(PC)              L0431
012CEE  3202                      MOVE.W    D2,D1               
012CF0  4E75                      RTS                           
012CF2  6100009C            __L0425:
                                  BSR       156(PC)              L042C
012CF6  33B110062006              MOVE.W    6(A1,D1.W),6(A1,D2.W) 
012CFC  33B9000170C61004          MOVE.W    $000170C6,4(A1,D1.W) W_L0683
012D04  33C1000170C6              MOVE.W    D1,$000170C6         W_L0683
012D0A  3202                      MOVE.W    D2,D1               
012D0C  4E75                      RTS                           
012D0E  61000080            L0426:BSR       128(PC)              L042C
012D12  6000FFA8                  BRA       -88(PC)              L0424
012D16  3401                L0427:MOVE.W    D1,D2               
012D18  32311006                  MOVE.W    6(A1,D1.W),D1       
012D1C  67000036                  BEQ       54(PC)               __L042A
012D20  B871101E                  CMP.W     30(A1,D1.W),D4      
012D24  6600FFF0                  BNE       -16(PC)              L0427
012D28  10311000                  MOVE.B    0(A1,D1.W),D0       
012D2C  6700FFE8                  BEQ       -24(PC)              L0427
012D30  6A00000A                  BPL       10(PC)               __L0428
012D34  6100FF86                  BSR       -122(PC)             L0424
012D38  6000FFDC                  BRA       -36(PC)              L0427
012D3C  B03C0002            __L0428:
                                  CMP.B     #2,D0               
012D40  6600000A                  BNE       10(PC)               __L0429
012D44  6100FFC8                  BSR       -56(PC)              L0426
012D48  6000FFCC                  BRA       -52(PC)              L0427
012D4C  6100FFA4            __L0429:
                                  BSR       -92(PC)              L0425
012D50  6000FFC4                  BRA       -60(PC)              L0427
012D54  4CDF0016            __L042A:
                                  MOVEM.L   (A7)+,D4/D1-D2      
012D58  13FC000100017106          MOVE.B    #1,$00017106         L06A6
012D60  61000102                  BSR       258(PC)              L0435
012D64  3202                      MOVE.W    D2,D1               
012D66  6000FC8E                  BRA       -882(PC)             L040D
012D6A  3831101E            L042B:MOVE.W    30(A1,D1.W),D4      
012D6E  33FC7FFF00017116          MOVE.W    #$7FFF,$00017116     L06AE
012D76  33FC7FFF00017118          MOVE.W    #$7FFF,$00017118     L06AF
012D7E  33FC80000001711A          MOVE.W    #$8000,$0001711A     L06B0
012D86  33FC80000001711C          MOVE.W    #$8000,$0001711C     L06B1
012D8E  4E75                      RTS                           
012D90  4A711010            __L042C:
                                  TST.W     16(A1,D1.W)         
012D94  6B000060                  BMI       96(PC)               L0430
012D98  20311008                  MOVE.L    8(A1,D1.W),D0       
012D9C  028000FFFFFF              ANDI.L    #$FFFFFF,D0         
012DA2  2040                      MOVEA.L   D0,A0               
012DA4  D1D0                      ADDA.L    (A0),A0             
012DA6  3031100C                  MOVE.W    12(A1,D1.W),D0      
012DAA  B07900017116              CMP.W     $00017116,D0         L06AE
012DB0  6E000008                  BGT       8(PC)                __L042D
012DB4  33C000017116              MOVE.W    D0,$00017116         L06AE
012DBA  D0680002            __L042D:
                                  ADD.W     2(A0),D0            
012DBE  B0790001711A              CMP.W     $0001711A,D0         L06B0
012DC4  6D000008                  BLT       8(PC)                __L042E
012DC8  33C00001711A              MOVE.W    D0,$0001711A         L06B0
012DCE  3031100E            __L042E:
                                  MOVE.W    14(A1,D1.W),D0      
012DD2  B07900017118              CMP.W     $00017118,D0         L06AF
012DD8  6E000008                  BGT       8(PC)                __L042F
012DDC  33C000017118              MOVE.W    D0,$00017118         L06AF
012DE2  D0680004            __L042F:
                                  ADD.W     4(A0),D0            
012DE6  B0790001711C              CMP.W     $0001711C,D0         L06B1
012DEC  6D000008                  BLT       8(PC)                L0430
012DF0  33C00001711C              MOVE.W    D0,$0001711C         L06B1
012DF6  4E75                L0430:RTS                           
012DF8  48A70F00            L0431:MOVEM.W   D4-D7,-(A7)         
012DFC  3A311010                  MOVE.W    16(A1,D1.W),D5      
012E00  1C311020                  MOVE.B    32(A1,D1.W),D6      
012E04  1E311001                  MOVE.B    1(A1,D1.W),D7       
012E08  3003                      MOVE.W    D3,D0               
012E0A  3800                L0432:MOVE.W    D0,D4               
012E0C  30310006                  MOVE.W    6(A1,D0.W),D0       
012E10  67000038                  BEQ       56(PC)               L0433
012E14  4A310000                  TST.B     0(A1,D0.W)          
012E18  6B00FFF0                  BMI       -16(PC)              L0432
012E1C  BA710010                  CMP.W     16(A1,D0.W),D5      
012E20  6D00FFE8                  BLT       -24(PC)              L0432
012E24  6E000024                  BGT       36(PC)               L0433
012E28  BC310020                  CMP.B     32(A1,D0.W),D6      
012E2C  6D00FFDC                  BLT       -36(PC)              L0432
012E30  6E000018                  BGT       24(PC)               L0433
012E34  BE310001                  CMP.B     1(A1,D0.W),D7       
012E38  6D00FFD0                  BLT       -48(PC)              L0432
012E3C  6E00000C                  BGT       12(PC)               L0433
012E40  0C3100FF1000              CMPI.B    #$FF,0(A1,D1.W)     
012E46  6600FFC2                  BNE       -62(PC)              L0432
012E4A  33801006            L0433:MOVE.W    D0,6(A1,D1.W)       
012E4E  33814006                  MOVE.W    D1,6(A1,D4.W)       
012E52  42311000                  CLR.B     0(A1,D1.W)          
012E56  B444                      CMP.W     D4,D2               
012E58  66000004                  BNE       4(PC)                __L0434
012E5C  3401                      MOVE.W    D1,D2               
012E5E  4C9F00F0            __L0434:
                                  MOVEM.W   (A7)+,D4-D7         
012E62  4E75                      RTS                           
012E64  082A00060000        L0435:BTST      #6,0(A2)            
012E6A  67000004                  BEQ       4(PC)                __L0436
012E6E  4E75                      RTS                           
012E70  48E7FFFE            __L0436:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
012E74  423900017122              CLR.B     $00017122            L06B4
012E7A  61000384                  BSR       900(PC)              __L0459
012E7E  66000156                  BNE       342(PC)              L0444
012E82  363900017104              MOVE.W    $00017104,D3         W_06A5
012E88  6000000C                  BRA       12(PC)               __L0438
012E8C  2079000170E0        L0437:MOVEA.L   $000170E0,A0         L_L0692
012E92  36303004                  MOVE.W    4(A0,D3.W),D3       
012E96  67000088            __L0438:
                                  BEQ       136(PC)              L043B
012E9A  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
012EA0  083000063000              BTST      #6,0(A0,D3.W)       
012EA6  6600FFE4                  BNE       -28(PC)              L0437
012EAA  32303002                  MOVE.W    2(A0,D3.W),D1       
012EAE  6700FFDC                  BEQ       -36(PC)              L0437
012EB2  48E75080                  MOVEM.L   A0/D3/D1,-(A7)      
012EB6  6100019E                  BSR       414(PC)              L044A
012EBA  4CDF010A                  MOVEM.L   (A7)+,A0/D3/D1      
012EBE  4A390001712C              TST.B     $0001712C            L06B9
012EC4  6700FFC6                  BEQ       -58(PC)              L0437
012EC8  083000063001              BTST      #6,1(A0,D3.W)       
012ECE  66000008                  BNE       8(PC)                __L0439
012ED2  4EB90000E4F4              JSR       $0000E4F4            L021D
012ED8  083000033001        __L0439:
                                  BTST      #3,1(A0,D3.W)       
012EDE  67000002                  BEQ       2(PC)                L043A
012EE2  32311006            L043A:MOVE.W    6(A1,D1.W),D1       
012EE6  6700FFA4                  BEQ       -92(PC)              L0437
012EEA  41F11000                  LEA       0(A1,D1.W),A0       
012EEE  4A280000                  TST.B     0(A0)               
012EF2  6B00FFEE                  BMI       -18(PC)              L043A
012EF6  4A280008                  TST.B     8(A0)               
012EFA  6B00FFE6                  BMI       -26(PC)              L043A
012EFE  4A680010                  TST.W     16(A0)              
012F02  6B00FFDE                  BMI       -34(PC)              L043A
012F06  48E75860                  MOVEM.L   A1-A2/D3-D4/D1,-(A7)
012F0A  4EB90000D3C0              JSR       $0000D3C0            L0192
012F10  4CDF061A                  MOVEM.L   (A7)+,A1-A2/D3-D4/D1
012F14  13FC00010001712F          MOVE.B    #1,$0001712F         L06BB
012F1C  6000FFC4                  BRA       -60(PC)              L043A
012F20  4A39000170CF        L043B:TST.B     $000170CF            L0688
012F26  6B000046                  BMI       70(PC)               L043F
012F2A  0C390002000170CF          CMPI.B    #2,$000170CF         L0688
012F32  6700003A                  BEQ       58(PC)               L043F
012F36  4A390001712D              TST.B     $0001712D            L06BA
012F3C  66000030                  BNE       48(PC)               L043F
012F40  5239000170D0        __L043C:
                                  ADDQ.B    #1,$000170D0         L0689
012F46  6700000C                  BEQ       12(PC)               __L043D
012F4A  5339000170D0              SUBQ.B    #1,$000170D0         L0689
012F50  6000FFEE                  BRA       -18(PC)              __L043C
012F54  4A390000AC76        __L043D:
                                  TST.B     $0000AC76            L0015
012F5A  6700000C                  BEQ       12(PC)               __L043E
012F5E  4EB90000D2CA              JSR       $0000D2CA            L0180
012F64  60000008                  BRA       8(PC)                L043F
012F68  4EB90000CF42        __L043E:
                                  JSR       $0000CF42            L015F
012F6E  082A00050001        L043F:BTST      #5,1(A2)            
012F74  66000034                  BNE       52(PC)               L0442
012F78  4A390001712D              TST.B     $0001712D            L06BA
012F7E  66000016                  BNE       22(PC)               __L0440
012F82  4A390001723D              TST.B     $0001723D            L0716
012F88  66000020                  BNE       32(PC)               L0442
012F8C  4EB90000E10E              JSR       $0000E10E            L0203
012F92  60000016                  BRA       22(PC)               L0442
012F96  4A390001723D        __L0440:
                                  TST.B     $0001723D            L0716
012F9C  66000006                  BNE       6(PC)                __L0441
012FA0  6100003A                  BSR       58(PC)               __L0445
012FA4  4CDF7FFF            __L0441:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
012FA8  4E75                      RTS                           
012FAA  4A390001712D        L0442:TST.B     $0001712D            L06BA
012FB0  66000024                  BNE       36(PC)               L0444
012FB4  4A390000AC76              TST.B     $0000AC76            L0015
012FBA  67000012                  BEQ       18(PC)               L0443
012FBE  4A39000170CF              TST.B     $000170CF            L0688
012FC4  6B000008                  BMI       8(PC)                L0443
012FC8  4EB90000CF82              JSR       $0000CF82            __L0161
012FCE  13FC00FF000170D0    L0443:MOVE.B    #$FF,$000170D0       L0689
012FD6  4CDF7FFF            L0444:MOVEM.L   (A7)+,A0-A6/D0-D7   
012FDA  4E75                      RTS                           
012FDC  207900017130        __L0445:
                                  MOVEA.L   $00017130,A0         L06BC
012FE2  30F900017108              MOVE.W    $00017108,(A0)+      L06A8
012FE8  30F900017110              MOVE.W    $00017110,(A0)+      L06AC
012FEE  30F90001710A              MOVE.W    $0001710A,(A0)+      L06A9
012FF4  30F90001710E              MOVE.W    $0001710E,(A0)+      L06AB
012FFA  23C800017130              MOVE.L    A0,$00017130         L06BC
013000  B1FC00017D04              CMPA.L    #$00017D04,A0        L0724
013006  6E000004                  BGT       4(PC)                __L0446
01300A  4E75                      RTS                           
01300C  203C00000016        __L0446:
                                  MOVE.L    #$16,D0             
013012  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
013018  4E75                      RTS                           
01301A  48E7FFFE            L0447:MOVEM.L   A0-A6/D0-D7,-(A7)   
01301E  207C00017A84              MOVEA.L   #$00017A84,A0        L0723
013024  60000020                  BRA       32(PC)               __L0449
013028  33D800017108        __L0448:
                                  MOVE.W    (A0)+,$00017108      L06A8
01302E  33D800017110              MOVE.W    (A0)+,$00017110      L06AC
013034  33D80001710A              MOVE.W    (A0)+,$0001710A      L06A9
01303A  33D80001710E              MOVE.W    (A0)+,$0001710E      L06AB
013040  4EB90000E0F4              JSR       $0000E0F4            __L0202
013046  B1F900017130        __L0449:
                                  CMPA.L    $00017130,A0         L06BC
01304C  6D00FFDA                  BLT       -38(PC)              __L0448
013050  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
013054  4E75                      RTS                           
013056  42390001712C        L044A:CLR.B     $0001712C            L06B9
01305C  41F11000                  LEA       0(A1,D1.W),A0       
013060  3028000C                  MOVE.W    12(A0),D0           
013064  B0790001710C              CMP.W     $0001710C,D0         L06AA
01306A  6F000004                  BLE       4(PC)                __L044B
01306E  4E75                      RTS                           
013070  3228000E            __L044B:
                                  MOVE.W    14(A0),D1           
013074  B2790001710E              CMP.W     $0001710E,D1         L06AB
01307A  6F000004                  BLE       4(PC)                __L044C
01307E  4E75                      RTS                           
013080  34280016            __L044C:
                                  MOVE.W    22(A0),D2           
013084  B47900017108              CMP.W     $00017108,D2         L06A8
01308A  6C000004                  BGE       4(PC)                __L044D
01308E  4E75                      RTS                           
013090  36280018            __L044D:
                                  MOVE.W    24(A0),D3           
013094  B6790001710A              CMP.W     $0001710A,D3         L06A9
01309A  6C000004                  BGE       4(PC)                __L044E
01309E  4E75                      RTS                           
0130A0  3800                __L044E:
                                  MOVE.W    D0,D4               
0130A2  B07900017108              CMP.W     $00017108,D0         L06A8
0130A8  6C000008                  BGE       8(PC)                __L044F
0130AC  383900017108              MOVE.W    $00017108,D4         L06A8
0130B2  3A01                __L044F:
                                  MOVE.W    D1,D5               
0130B4  B2790001710A              CMP.W     $0001710A,D1         L06A9
0130BA  6C000008                  BGE       8(PC)                __L0450
0130BE  3A390001710A              MOVE.W    $0001710A,D5         L06A9
0130C4  3C02                __L0450:
                                  MOVE.W    D2,D6               
0130C6  B4790001710C              CMP.W     $0001710C,D2         L06AA
0130CC  6F000008                  BLE       8(PC)                __L0451
0130D0  3C390001710C              MOVE.W    $0001710C,D6         L06AA
0130D6  3E03                __L0451:
                                  MOVE.W    D3,D7               
0130D8  B6790001710E              CMP.W     $0001710E,D3         L06AB
0130DE  6F000008                  BLE       8(PC)                __L0452
0130E2  3E390001710E              MOVE.W    $0001710E,D7         L06AB
0130E8  33C400017124        __L0452:
                                  MOVE.W    D4,$00017124         L06B5
0130EE  33C500017126              MOVE.W    D5,$00017126         L06B6
0130F4  33C600017128              MOVE.W    D6,$00017128         L06B7
0130FA  33C70001712A              MOVE.W    D7,$0001712A         L06B8
013100  13FC00010001712C          MOVE.B    #1,$0001712C         L06B9
013108  4E75                      RTS                           
01310A  48E77F00            __L0453:
                                  MOVEM.L   D1-D7,-(A7)         
01310E  41F11000                  LEA       0(A1,D1.W),A0       
013112  20280008                  MOVE.L    8(A0),D0            
013116  028000FFFFFF              ANDI.L    #$FFFFFF,D0         
01311C  2C40                      MOVEA.L   D0,A6               
01311E  DDD6                      ADDA.L    (A6),A6             
013120  2A7900017152              MOVEA.L   $00017152,A5         L06C5
013126  DBD5                      ADDA.L    (A5),A5             
013128  4A680010                  TST.W     16(A0)              
01312C  6B0000C6                  BMI       198(PC)              L0458
013130  4A790001715A              TST.W     $0001715A            L06C8
013136  6B0000BC                  BMI       188(PC)              L0458
01313A  3028000C                  MOVE.W    12(A0),D0           
01313E  383900017156              MOVE.W    $00017156,D4         L06C6
013144  D86D0002                  ADD.W     2(A5),D4            
013148  B044                      CMP.W     D4,D0               
01314A  6E0000A8                  BGT       168(PC)              L0458
01314E  3228000E                  MOVE.W    14(A0),D1           
013152  383900017158              MOVE.W    $00017158,D4         L06C7
013158  D86D0004                  ADD.W     4(A5),D4            
01315C  B244                      CMP.W     D4,D1               
01315E  6E000094                  BGT       148(PC)              L0458
013162  D06E0002                  ADD.W     2(A6),D0            
013166  B07900017156              CMP.W     $00017156,D0         L06C6
01316C  6D000086                  BLT       134(PC)              L0458
013170  D26E0004                  ADD.W     4(A6),D1            
013174  B27900017158              CMP.W     $00017158,D1         L06C7
01317A  6D000078                  BLT       120(PC)              L0458
01317E  3828000C                  MOVE.W    12(A0),D4           
013182  B87900017156              CMP.W     $00017156,D4         L06C6
013188  6F000008                  BLE       8(PC)                __L0454
01318C  383900017156              MOVE.W    $00017156,D4         L06C6
013192  3A28000E            __L0454:
                                  MOVE.W    14(A0),D5           
013196  BA7900017158              CMP.W     $00017158,D5         L06C7
01319C  6F000008                  BLE       8(PC)                __L0455
0131A0  3A3900017158              MOVE.W    $00017158,D5         L06C7
0131A6  3C00                __L0455:
                                  MOVE.W    D0,D6               
0131A8  343900017156              MOVE.W    $00017156,D2         L06C6
0131AE  D46D0002                  ADD.W     2(A5),D2            
0131B2  BC42                      CMP.W     D2,D6               
0131B4  6C000004                  BGE       4(PC)                __L0456
0131B8  3C02                      MOVE.W    D2,D6               
0131BA  3E01                __L0456:
                                  MOVE.W    D1,D7               
0131BC  363900017158              MOVE.W    $00017158,D3         L06C7
0131C2  D66D0004                  ADD.W     4(A5),D3            
0131C6  BE43                      CMP.W     D3,D7               
0131C8  6C000004                  BGE       4(PC)                __L0457
0131CC  3E03                      MOVE.W    D3,D7               
0131CE  33C400017116        __L0457:
                                  MOVE.W    D4,$00017116         L06AE
0131D4  33C500017118              MOVE.W    D5,$00017118         L06AF
0131DA  33C60001711A              MOVE.W    D6,$0001711A         L06B0
0131E0  33C70001711C              MOVE.W    D7,$0001711C         L06B1
0131E6  13FC000100017106          MOVE.B    #1,$00017106         L06A6
0131EE  4CDF00FE                  MOVEM.L   (A7)+,D1-D7         
0131F2  4E75                      RTS                           
0131F4  423900017106        L0458:CLR.B     $00017106            L06A6
0131FA  4CDF00FE                  MOVEM.L   (A7)+,D1-D7         
0131FE  4E75                      RTS                           
013200  48E75020            __L0459:
                                  MOVEM.L   A2/D3/D1,-(A7)      
013204  4A3900017106              TST.B     $00017106            L06A6
01320A  66000036                  BNE       54(PC)               __L045A
01320E  4A711010                  TST.W     16(A1,D1.W)         
013212  6B00006E                  BMI       110(PC)              L045B
013216  20311008                  MOVE.L    8(A1,D1.W),D0       
01321A  028000FFFFFF              ANDI.L    #$FFFFFF,D0         
013220  2040                      MOVEA.L   D0,A0               
013222  D1D0                      ADDA.L    (A0),A0             
013224  4CB10030100C              MOVEM.W   12(A1,D1.W),D4-D5   
01322A  48B9003000017116          MOVEM.W   D4-D5,$00017116      L06AE
013232  D8680002                  ADD.W     2(A0),D4            
013236  DA680004                  ADD.W     4(A0),D5            
01323A  48B900300001711A          MOVEM.W   D4-D5,$0001711A      L06B0
013242  45F13000            __L045A:
                                  LEA       0(A1,D3.W),A2       
013246  303900017116              MOVE.W    $00017116,D0         L06AE
01324C  342A0016                  MOVE.W    22(A2),D2           
013250  B042                      CMP.W     D2,D0               
013252  6E00002E                  BGT       46(PC)               L045B
013256  323900017118              MOVE.W    $00017118,D1         L06AF
01325C  362A0018                  MOVE.W    24(A2),D3           
013260  B243                      CMP.W     D3,D1               
013262  6E00001E                  BGT       30(PC)               L045B
013266  30390001711A              MOVE.W    $0001711A,D0         L06B0
01326C  B06A000C                  CMP.W     12(A2),D0           
013270  6D000010                  BLT       16(PC)               L045B
013274  32390001711C              MOVE.W    $0001711C,D1         L06B1
01327A  B26A000E                  CMP.W     14(A2),D1           
01327E  6C00000A                  BGE       10(PC)               __L045C
013282  BEFC0000            L045B:CMPA.W    #0,A7               
013286  60000086                  BRA       134(PC)              __L0461
01328A  383900017116        __L045C:
                                  MOVE.W    $00017116,D4         L06AE
013290  B86A000C                  CMP.W     12(A2),D4           
013294  6C000006                  BGE       6(PC)                __L045D
013298  382A000C                  MOVE.W    12(A2),D4           
01329C  3A3900017118        __L045D:
                                  MOVE.W    $00017118,D5         L06AF
0132A2  BA6A000E                  CMP.W     14(A2),D5           
0132A6  6C000006                  BGE       6(PC)                __L045E
0132AA  3A2A000E                  MOVE.W    14(A2),D5           
0132AE  3C390001711A        __L045E:
                                  MOVE.W    $0001711A,D6         L06B0
0132B4  BC42                      CMP.W     D2,D6               
0132B6  6F000004                  BLE       4(PC)                __L045F
0132BA  3C02                      MOVE.W    D2,D6               
0132BC  3E390001711C        __L045F:
                                  MOVE.W    $0001711C,D7         L06B1
0132C2  BE43                      CMP.W     D3,D7               
0132C4  6F000004                  BLE       4(PC)                __L0460
0132C8  3E03                      MOVE.W    D3,D7               
0132CA  0244FFF0            __L0460:
                                  ANDI.W    #$FFF0,D4           
0132CE  0046000F                  ORI.W     #$F,D6              
0132D2  33C400017108              MOVE.W    D4,$00017108         L06A8
0132D8  33C50001710A              MOVE.W    D5,$0001710A         L06A9
0132DE  33C60001710C              MOVE.W    D6,$0001710C         L06AA
0132E4  33C70001710E              MOVE.W    D7,$0001710E         L06AB
0132EA  9C44                      SUB.W     D4,D6               
0132EC  5246                      ADDQ.W    #1,D6               
0132EE  E44E                      LSR.W     #2,D6               
0132F0  33C600017110              MOVE.W    D6,$00017110         L06AC
0132F6  CAFC00A0                  MULU      #$A0,D5             
0132FA  DAB90000AC4E              ADD.L     $0000AC4E,D5         L0009
013300  E24C                      LSR.W     #1,D4               
013302  48C4                      EXT.L     D4                  
013304  DA84                      ADD.L     D4,D5               
013306  23C500017112              MOVE.L    D5,$00017112         L06AD
01330C  B040                      CMP.W     D0,D0               
01330E  4CDF040A            __L0461:
                                  MOVEM.L   (A7)+,A2/D3/D1      
013312  4E75                      RTS                           
013314  206EFFEC            __L0462:
                                  MOVEA.L   -20(A6),A0          
013318  D1E8000E                  ADDA.L    14(A0),A0           
01331C  D1E80006                  ADDA.L    6(A0),A0            
013320  DE47                      ADD.W     D7,D7               
013322  D0F07000                  ADDA.W    0(A0,D7.W),A0       
013326  4E75                      RTS                           
013328  203C00000010              MOVE.L    #$10,D0             
01332E  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x51 -  offset 0x2612 -  address $013334
013334  61001DBA                  BSR       7610(PC)             FN_READ_OPNAME
013338  3D47FFE6                  MOVE.W    D7,-26(A6)          
01333C  4E75                      RTS                           
01333E  203C00000012              MOVE.L    #$12,D0             
013344  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x52 -  offset 0x2628 -  address $01334a
01334A  3D7CFFFFFFE6              MOVE.W    #$FFFF,-26(A6)      
013350  4E75                      RTS                           


; =============================================================================
; OPCODE 0xad -  offset 0x2630 -  address $013352
013352  61001D9C                  BSR       7580(PC)             FN_READ_OPNAME
013356  3D47FFE6                  MOVE.W    D7,-26(A6)          
01335A  4E75                      RTS                           
01335C  61001D92            L0463:BSR       7570(PC)             FN_READ_OPNAME
013360  6100FFB2                  BSR       -78(PC)              __L0462
013364  1010                      MOVE.B    (A0),D0             
013366  6700004A                  BEQ       74(PC)               __L0467
01336A  6B000074                  BMI       116(PC)              __L0468
01336E  B03C0001                  CMP.B     #1,D0               
013372  6700001E                  BEQ       30(PC)               __L0466
013376  5488                      ADDQ.L    #2,A0               
013378  4C900007                  MOVEM.W   (A0),D0-D2          
01337C  0C2E0015FFD2        L0464:CMPI.B    #$15,-46(A6)        
013382  6D00000C                  BLT       12(PC)               L0465
013386  4A2EFFFD                  TST.B     -3(A6)              
01338A  67000004                  BEQ       4(PC)                L0465
01338E  4440                      NEG.W     D0                  
013390  4E75                L0465:RTS                           
013392  2F03                __L0466:
                                  MOVE.L    D3,-(A7)            
013394  5888                      ADDQ.L    #4,A0               
013396  4C980007                  MOVEM.W   (A0)+,D0-D2         
01339A  3618                      MOVE.W    (A0)+,D3            
01339C  E243                      ASR.W     #1,D3               
01339E  D043                      ADD.W     D3,D0               
0133A0  3618                      MOVE.W    (A0)+,D3            
0133A2  E243                      ASR.W     #1,D3               
0133A4  D243                      ADD.W     D3,D1               
0133A6  3618                      MOVE.W    (A0)+,D3            
0133A8  E243                      ASR.W     #1,D3               
0133AA  D443                      ADD.W     D3,D2               
0133AC  261F                      MOVE.L    (A7)+,D3            
0133AE  6000FFCC                  BRA       -52(PC)              L0464
0133B2  2F03                __L0467:
                                  MOVE.L    D3,-(A7)            
0133B4  5888                      ADDQ.L    #4,A0               
0133B6  1018                      MOVE.B    (A0)+,D0            
0133B8  4880                      EXT.W     D0                  
0133BA  1218                      MOVE.B    (A0)+,D1            
0133BC  4881                      EXT.W     D1                  
0133BE  1418                      MOVE.B    (A0)+,D2            
0133C0  4882                      EXT.W     D2                  
0133C2  1618                      MOVE.B    (A0)+,D3            
0133C4  4883                      EXT.W     D3                  
0133C6  E243                      ASR.W     #1,D3               
0133C8  D043                      ADD.W     D3,D0               
0133CA  1618                      MOVE.B    (A0)+,D3            
0133CC  4883                      EXT.W     D3                  
0133CE  E243                      ASR.W     #1,D3               
0133D0  D243                      ADD.W     D3,D1               
0133D2  1618                      MOVE.B    (A0)+,D3            
0133D4  4883                      EXT.W     D3                  
0133D6  E243                      ASR.W     #1,D3               
0133D8  D443                      ADD.W     D3,D2               
0133DA  261F                      MOVE.L    (A7)+,D3            
0133DC  6000FF9E                  BRA       -98(PC)              L0464
0133E0  203C00000014        __L0468:
                                  MOVE.L    #$14,D0             
0133E6  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0xa3 -  offset 0x26ca -  address $0133ec
0133EC  6100FF6E                  BSR       -146(PC)             L0463
0133F0  D06E0000                  ADD.W     0(A6),D0            
0133F4  D26E0002                  ADD.W     2(A6),D1            
0133F8  D46E0004                  ADD.W     4(A6),D2            
0133FC  33C00001715E              MOVE.W    D0,$0001715E         L06CA
013402  33C100017160              MOVE.W    D1,$00017160         L06CB
013408  33C200017162              MOVE.W    D2,$00017162         L06CC
01340E  61001CE0                  BSR       7392(PC)             FN_READ_OPNAME
013412  33C700017166              MOVE.W    D7,$00017166         L06CE
013418  33EEFFE600017164          MOVE.W    -26(A6),$00017164    L06CD
013420  610002A0                  BSR       672(PC)              L046D
013424  60000280                  BRA       640(PC)              L046C
013428  4E75                      RTS                           


; =============================================================================
; OPCODE 0xa4 -  offset 0x2708 -  address $01342a
01342A  6100FF30                  BSR       -208(PC)             L0463
01342E  D06E0000                  ADD.W     0(A6),D0            
013432  D26E0002                  ADD.W     2(A6),D1            
013436  D46E0004                  ADD.W     4(A6),D2            
01343A  33C00001715E              MOVE.W    D0,$0001715E         L06CA
013440  33C100017160              MOVE.W    D1,$00017160         L06CB
013446  33C200017162              MOVE.W    D2,$00017162         L06CC
01344C  61001CA2                  BSR       7330(PC)             FN_READ_OPNAME
013450  33C700017166              MOVE.W    D7,$00017166         L06CE
013456  61001C98                  BSR       7320(PC)             FN_READ_OPNAME
01345A  33C700017164              MOVE.W    D7,$00017164         L06CD
013460  61000260                  BSR       608(PC)              L046D
013464  60000240                  BRA       576(PC)              L046C
013468  203C00000012              MOVE.L    #$12,D0             
01346E  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0xa5 -  offset 0x2752 -  address $013474
013474  33EE00000001715E          MOVE.W    0(A6),$0001715E      L06CA
01347C  33EE000200017160          MOVE.W    2(A6),$00017160      L06CB
013484  33EE000400017162          MOVE.W    4(A6),$00017162      L06CC
01348C  61001C62                  BSR       7266(PC)             FN_READ_OPNAME
013490  33C700017166              MOVE.W    D7,$00017166         L06CE
013496  61001C58                  BSR       7256(PC)             FN_READ_OPNAME
01349A  33C700017164              MOVE.W    D7,$00017164         L06CD
0134A0  61000220                  BSR       544(PC)              L046D
0134A4  60000200                  BRA       512(PC)              L046C
0134A8  203C00000012              MOVE.L    #$12,D0             
0134AE  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
0134B4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x53 -  offset 0x2794 -  address $0134b6
0134B6  61001C38                  BSR       7224(PC)             FN_READ_OPNAME
0134BA  DE6E0000                  ADD.W     0(A6),D7            
0134BE  33C70001715E              MOVE.W    D7,$0001715E         L06CA
0134C4  61001C2A                  BSR       7210(PC)             FN_READ_OPNAME
0134C8  DE6E0002                  ADD.W     2(A6),D7            
0134CC  33C700017160              MOVE.W    D7,$00017160         L06CB
0134D2  61001C1C                  BSR       7196(PC)             FN_READ_OPNAME
0134D6  DE6E0004                  ADD.W     4(A6),D7            
0134DA  33C700017162              MOVE.W    D7,$00017162         L06CC
0134E0  61001C0E                  BSR       7182(PC)             FN_READ_OPNAME
0134E4  33C700017166              MOVE.W    D7,$00017166         L06CE
0134EA  33EEFFE600017164          MOVE.W    -26(A6),$00017164    L06CD
0134F2  610001CE                  BSR       462(PC)              L046D
0134F6  600001AE                  BRA       430(PC)              L046C


; =============================================================================
; OPCODE 0x8e -  offset 0x27d8 -  address $0134fa
0134FA  102E0009                  MOVE.B    9(A6),D0            
0134FE  4880                      EXT.W     D0                  
013500  D06E0000                  ADD.W     0(A6),D0            
013504  33C00001715E              MOVE.W    D0,$0001715E         L06CA
01350A  102E000A                  MOVE.B    10(A6),D0           
01350E  4880                      EXT.W     D0                  
013510  D06E0002                  ADD.W     2(A6),D0            
013514  33C000017160              MOVE.W    D0,$00017160         L06CB
01351A  102E000B                  MOVE.B    11(A6),D0           
01351E  4880                      EXT.W     D0                  
013520  D06E0004                  ADD.W     4(A6),D0            
013524  33C000017162              MOVE.W    D0,$00017162         L06CC
01352A  61001BC4                  BSR       7108(PC)             FN_READ_OPNAME
01352E  33C700017166              MOVE.W    D7,$00017166         L06CE
013534  33EEFFE600017164          MOVE.W    -26(A6),$00017164    L06CD
01353C  61000184                  BSR       388(PC)              L046D
013540  60000164                  BRA       356(PC)              L046C


; =============================================================================
; OPCODE 0x8f -  offset 0x2822 -  address $013544
013544  102E0009                  MOVE.B    9(A6),D0            
013548  4880                      EXT.W     D0                  
01354A  D06E0000                  ADD.W     0(A6),D0            
01354E  33C00001715E              MOVE.W    D0,$0001715E         L06CA
013554  102E000A                  MOVE.B    10(A6),D0           
013558  4880                      EXT.W     D0                  
01355A  D06E0002                  ADD.W     2(A6),D0            
01355E  33C000017160              MOVE.W    D0,$00017160         L06CB
013564  102E000B                  MOVE.B    11(A6),D0           
013568  4880                      EXT.W     D0                  
01356A  D06E0004                  ADD.W     4(A6),D0            
01356E  33C000017162              MOVE.W    D0,$00017162         L06CC
013574  61001B7A                  BSR       7034(PC)             FN_READ_OPNAME
013578  33C700017166              MOVE.W    D7,$00017166         L06CE
01357E  61001B70                  BSR       7024(PC)             FN_READ_OPNAME
013582  33C700017164              MOVE.W    D7,$00017164         L06CD
013588  61000138                  BSR       312(PC)              L046D
01358C  60000118                  BRA       280(PC)              L046C
013590  203C00000012              MOVE.L    #$12,D0             
013596  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x54 -  offset 0x287a -  address $01359c
01359C  61001B52                  BSR       6994(PC)             FN_READ_OPNAME
0135A0  33C70001715E              MOVE.W    D7,$0001715E         L06CA
0135A6  61001B48                  BSR       6984(PC)             FN_READ_OPNAME
0135AA  33C700017160              MOVE.W    D7,$00017160         L06CB
0135B0  61001B3E                  BSR       6974(PC)             FN_READ_OPNAME
0135B4  33C700017162              MOVE.W    D7,$00017162         L06CC
0135BA  61001B34                  BSR       6964(PC)             FN_READ_OPNAME
0135BE  33C700017166              MOVE.W    D7,$00017166         L06CE
0135C4  33EEFFE600017164          MOVE.W    -26(A6),$00017164    L06CD
0135CC  610000F4                  BSR       244(PC)              L046D
0135D0  600000D4                  BRA       212(PC)              L046C


; =============================================================================
; OPCODE 0x55 -  offset 0x28b2 -  address $0135d4
0135D4  61001B1A                  BSR       6938(PC)             FN_READ_OPNAME
0135D8  DE6E0000                  ADD.W     0(A6),D7            
0135DC  33C70001715E              MOVE.W    D7,$0001715E         L06CA
0135E2  61001B0C                  BSR       6924(PC)             FN_READ_OPNAME
0135E6  DE6E0002                  ADD.W     2(A6),D7            
0135EA  33C700017160              MOVE.W    D7,$00017160         L06CB
0135F0  61001AFE                  BSR       6910(PC)             FN_READ_OPNAME
0135F4  DE6E0004                  ADD.W     4(A6),D7            
0135F8  33C700017162              MOVE.W    D7,$00017162         L06CC
0135FE  61001AF0                  BSR       6896(PC)             FN_READ_OPNAME
013602  33C700017166              MOVE.W    D7,$00017166         L06CE
013608  61001AE6                  BSR       6886(PC)             FN_READ_OPNAME
01360C  33C700017164              MOVE.W    D7,$00017164         L06CD
013612  610000AE                  BSR       174(PC)              L046D
013616  6000008E                  BRA       142(PC)              L046C
01361A  203C00000012              MOVE.L    #$12,D0             
013620  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x56 -  offset 0x2904 -  address $013626
013626  61001AC8                  BSR       6856(PC)             FN_READ_OPNAME
01362A  33C70001715E              MOVE.W    D7,$0001715E         L06CA
013630  61001ABE                  BSR       6846(PC)             FN_READ_OPNAME
013634  33C700017160              MOVE.W    D7,$00017160         L06CB
01363A  61001AB4                  BSR       6836(PC)             FN_READ_OPNAME
01363E  33C700017162              MOVE.W    D7,$00017162         L06CC
013644  61001AAA                  BSR       6826(PC)             FN_READ_OPNAME
013648  33C700017166              MOVE.W    D7,$00017166         L06CE
01364E  61001AA0                  BSR       6816(PC)             FN_READ_OPNAME
013652  33C700017164              MOVE.W    D7,$00017164         L06CD
013658  61000068                  BSR       104(PC)              L046D
01365C  60000048                  BRA       72(PC)               L046C
013660  203C00000012              MOVE.L    #$12,D0             
013666  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x57 -  offset 0x294a -  address $01366c
01366C  207900017172        __L0469:
                                  MOVEA.L   $00017172,A0         L06D0
013672  3E10                      MOVE.W    (A0),D7             
013674  6B000008                  BMI       8(PC)                __L046A
013678  54B900017172              ADDQ.L    #2,$00017172         L06D0
01367E  6000DDCA            __L046A:
                                  BRA       -8758(PC)            L035D


; =============================================================================
; OPCODE 0x58 -  offset 0x2960 -  address $013682
013682  207900017172              MOVEA.L   $00017172,A0         L06D0
013688  B1FC00017884              CMPA.L    #$00017884,A0        L0722
01368E  6700000E                  BEQ       14(PC)               __L046B
013692  3E20                      MOVE.W    -(A0),D7            
013694  55B900017172              SUBQ.L    #2,$00017172         L06D0
01369A  6000DDAE                  BRA       -8786(PC)            L035D
01369E  3E3CFFFF            __L046B:
                                  MOVE.W    #$FFFF,D7           
0136A2  6000DDA6                  BRA       -8794(PC)            L035D


; =============================================================================
; OPCODE 0x60 -  offset 0x2984 -  address $0136a6
0136A6  23FC0001788400017172L046C:MOVE.L    #$00017884,$00017172 L0722 L06D0
0136B0  6000FFBA                  BRA       -70(PC)              __L0469


; =============================================================================
; OPCODE 0xe1 -  offset 0x2992 -  address $0136b4
0136B4  207900017172              MOVEA.L   $00017172,A0         L06D0
0136BA  3E2800FE                  MOVE.W    254(A0),D7          
0136BE  6000DD8A                  BRA       -8822(PC)            L035D
0136C2  48E7FFFE            L046D:MOVEM.L   A0-A6/D0-D7,-(A7)   
0136C6  23FC0001788400017172      MOVE.L    #$00017884,$00017172 L0722 L06D0
0136D0  303900017164              MOVE.W    $00017164,D0         L06CD
0136D6  6B0000A2                  BMI       162(PC)              L0472
0136DA  48C0                      EXT.L     D0                  
0136DC  2A6EFFEC                  MOVEA.L   -20(A6),A5          
0136E0  DBED000E                  ADDA.L    14(A5),A5           
0136E4  DBED0006                  ADDA.L    6(A5),A5            
0136E8  23CD00017178              MOVE.L    A5,$00017178         L06D2
0136EE  D080                      ADD.L     D0,D0               
0136F0  DAF50800                  ADDA.W    0(A5,D0.L),A5       
0136F4  4241                      CLR.W     D1                  
0136F6  2079000170A8              MOVEA.L   $000170A8,A0         L067A
0136FC  6008                      BRA.S     8(PC)                __L046F
0136FE  32301004            L046E:MOVE.W    4(A0,D1.W),D1       
013702  67000076                  BEQ       118(PC)              L0472
013706  22701000            __L046F:
                                  MOVEA.L   0(A0,D1.W),A1       
01370A  30290006                  MOVE.W    6(A1),D0            
01370E  B06E0006                  CMP.W     6(A6),D0            
013712  6600FFEA                  BNE       -22(PC)              L046E
013716  3029FFE6                  MOVE.W    -26(A1),D0          
01371A  6B00FFE2                  BMI       -30(PC)              L046E
01371E  48C0                      EXT.L     D0                  
013720  BDC9                      CMPA.L    A1,A6               
013722  6700FFDA                  BEQ       -38(PC)              L046E
013726  48E74000                  MOVEM.L   D1,-(A7)            
01372A  2869FFEC                  MOVEA.L   -20(A1),A4          
01372E  D9EC000E                  ADDA.L    14(A4),A4           
013732  D9EC0006                  ADDA.L    6(A4),A4            
013736  244C                      MOVEA.L   A4,A2               
013738  D080                      ADD.L     D0,D0               
01373A  D4F20800                  ADDA.W    0(A2,D0.L),A2       
01373E  264D                      MOVEA.L   A5,A3               
013740  4A5B                      TST.W     (A3)+               
013742  6A00000A                  BPL       10(PC)               __L0470
013746  61000058                  BSR       88(PC)               L0473
01374A  60000006                  BRA       6(PC)                __L0471
01374E  6100008E            __L0470:
                                  BSR       142(PC)              L0478
013752  4CDF0002            __L0471:
                                  MOVEM.L   (A7)+,D1            
013756  6700FFA6                  BEQ       -90(PC)              L046E
01375A  247900017172              MOVEA.L   $00017172,A2         L06D0
013760  3579000171700100          MOVE.W    $00017170,256(A2)    L06CF
013768  34C1                      MOVE.W    D1,(A2)+            
01376A  23CA00017172              MOVE.L    A2,$00017172         L06D0
013770  4A3900017188              TST.B     $00017188            L06D9
013776  6600FF86                  BNE       -122(PC)             L046E
01377A  247900017172        L0472:MOVEA.L   $00017172,A2         L06D0
013780  426A0100                  CLR.W     256(A2)             
013784  34FCFFFF                  MOVE.W    #$FFFF,(A2)+        
013788  23CA00017172              MOVE.L    A2,$00017172         L06D0
01378E  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
013792  4E75                      RTS                           
013794  203C00000010              MOVE.L    #$10,D0             
01379A  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
0137A0  4241                L0473:CLR.W     D1                  
0137A2  122BFFFF                  MOVE.B    -1(A3),D1           
0137A6  67000022                  BEQ       34(PC)               L0476
0137AA  5341                      SUBQ.W    #1,D1               
0137AC  301B                L0474:MOVE.W    (A3)+,D0            
0137AE  6B000014                  BMI       20(PC)               __L0475
0137B2  48C0                      EXT.L     D0                  
0137B4  48E74030                  MOVEM.L   A2-A3/D1,-(A7)      
0137B8  61000012                  BSR       18(PC)               __L0477
0137BC  4CDF0C02                  MOVEM.L   (A7)+,A2-A3/D1      
0137C0  66000008                  BNE       8(PC)                L0476
0137C4  51C9FFE6            __L0475:
                                  DBF       D1,-26(PC)           L0474
0137C8  4241                      CLR.W     D1                  
0137CA  4E75                L0476:RTS                           
0137CC  D080                __L0477:
                                  ADD.L     D0,D0               
0137CE  267900017178              MOVEA.L   $00017178,A3         L06D2
0137D4  D6F30800                  ADDA.W    0(A3,D0.L),A3       
0137D8  4A5B                      TST.W     (A3)+               
0137DA  6B00FFC4                  BMI       -60(PC)              L0473
0137DE  548B                L0478:ADDQ.L    #2,A3               
0137E0  67000010                  BEQ       16(PC)               __L0479
0137E4  0C2B0001FFFC              CMPI.B    #1,-4(A3)           
0137EA  6700004E                  BEQ       78(PC)               __L047C
0137EE  B040                      CMP.W     D0,D0               
0137F0  4E75                      RTS                           
0137F2  121B                __L0479:
                                  MOVE.B    (A3)+,D1            
0137F4  4881                      EXT.W     D1                  
0137F6  141B                      MOVE.B    (A3)+,D2            
0137F8  4882                      EXT.W     D2                  
0137FA  161B                      MOVE.B    (A3)+,D3            
0137FC  4883                      EXT.W     D3                  
0137FE  4A2EFFFD                  TST.B     -3(A6)              
013802  67000004                  BEQ       4(PC)                __L047A
013806  4441                      NEG.W     D1                  
013808  D2790001715E        __L047A:
                                  ADD.W     $0001715E,D1         L06CA
01380E  D47900017160              ADD.W     $00017160,D2         L06CB
013814  D67900017162              ADD.W     $00017162,D3         L06CC
01381A  181B                      MOVE.B    (A3)+,D4            
01381C  4884                      EXT.W     D4                  
01381E  1A1B                      MOVE.B    (A3)+,D5            
013820  4885                      EXT.W     D5                  
013822  1C1B                      MOVE.B    (A3)+,D6            
013824  4886                      EXT.W     D6                  
013826  4A2EFFFD                  TST.B     -3(A6)              
01382A  67000004                  BEQ       4(PC)                __L047B
01382E  4444                      NEG.W     D4                  
013830  D841                __L047B:
                                  ADD.W     D1,D4               
013832  DA42                      ADD.W     D2,D5               
013834  DC43                      ADD.W     D3,D6               
013836  60000036                  BRA       54(PC)               __L047F
01383A  4C9B000E            __L047C:
                                  MOVEM.W   (A3)+,D1-D3         
01383E  4A2EFFFD                  TST.B     -3(A6)              
013842  67000004                  BEQ       4(PC)                __L047D
013846  4441                      NEG.W     D1                  
013848  D2790001715E        __L047D:
                                  ADD.W     $0001715E,D1         L06CA
01384E  D47900017160              ADD.W     $00017160,D2         L06CB
013854  D67900017162              ADD.W     $00017162,D3         L06CC
01385A  4C9B0070                  MOVEM.W   (A3)+,D4-D6         
01385E  4A2EFFFD                  TST.B     -3(A6)              
013862  67000004                  BEQ       4(PC)                __L047E
013866  4444                      NEG.W     D4                  
013868  D841                __L047E:
                                  ADD.W     D1,D4               
01386A  DA42                      ADD.W     D2,D5               
01386C  DC43                      ADD.W     D3,D6               
01386E  BA42                __L047F:
                                  CMP.W     D2,D5               
013870  6E000004                  BGT       4(PC)                __L0480
013874  C545                      EXG       D5,D2               
013876  BC43                __L0480:
                                  CMP.W     D3,D6               
013878  6E000004                  BGT       4(PC)                __L0481
01387C  C746                      EXG       D6,D3               
01387E  B841                __L0481:
                                  CMP.W     D1,D4               
013880  6E000004                  BGT       4(PC)                __L0482
013884  C344                      EXG       D4,D1               
013886  48B9007E0001717C    __L0482:
                                  MOVEM.W   D1-D6,$0001717C      L06D3
01388E  4A5A                      TST.W     (A2)+               
013890  6A000042                  BPL       66(PC)               __L0489
013894  60000004                  BRA       4(PC)                L0484
013898  4E75                __L0483:
                                  RTS                           
01389A  4241                L0484:CLR.W     D1                  
01389C  122AFFFF                  MOVE.B    -1(A2),D1           
0138A0  67000022                  BEQ       34(PC)               L0487
0138A4  5341                      SUBQ.W    #1,D1               
0138A6  301A                L0485:MOVE.W    (A2)+,D0            
0138A8  6B000014                  BMI       20(PC)               __L0486
0138AC  48C0                      EXT.L     D0                  
0138AE  48E74020                  MOVEM.L   A2/D1,-(A7)         
0138B2  61000012                  BSR       18(PC)               __L0488
0138B6  4CDF0402                  MOVEM.L   (A7)+,A2/D1         
0138BA  66000008                  BNE       8(PC)                L0487
0138BE  51C9FFE6            __L0486:
                                  DBF       D1,-26(PC)           L0485
0138C2  4241                      CLR.W     D1                  
0138C4  4E75                L0487:RTS                           
0138C6  D080                __L0488:
                                  ADD.L     D0,D0               
0138C8  244C                      MOVEA.L   A4,A2               
0138CA  D4F40800                  ADDA.W    0(A4,D0.L),A2       
0138CE  4A5A                      TST.W     (A2)+               
0138D0  6B00FFC8                  BMI       -56(PC)              L0484
0138D4  67000010            __L0489:
                                  BEQ       16(PC)               L048A
0138D8  0C2A0001FFFE              CMPI.B    #1,-2(A2)           
0138DE  6700005C                  BEQ       92(PC)               __L048D
0138E2  600000F4                  BRA       244(PC)              L0494
0138E6  301A                L048A:MOVE.W    (A2)+,D0            
0138E8  C07900017166              AND.W     $00017166,D0         L06CE
0138EE  670000E8                  BEQ       232(PC)              L0494
0138F2  33EAFFFE00017170          MOVE.W    -2(A2),$00017170     L06CF
0138FA  121A                      MOVE.B    (A2)+,D1            
0138FC  4881                      EXT.W     D1                  
0138FE  141A                      MOVE.B    (A2)+,D2            
013900  4882                      EXT.W     D2                  
013902  161A                      MOVE.B    (A2)+,D3            
013904  4883                      EXT.W     D3                  
013906  4A29FFFD                  TST.B     -3(A1)              
01390A  67000004                  BEQ       4(PC)                __L048B
01390E  4441                      NEG.W     D1                  
013910  D2690000            __L048B:
                                  ADD.W     0(A1),D1            
013914  D4690002                  ADD.W     2(A1),D2            
013918  D6690004                  ADD.W     4(A1),D3            
01391C  181A                      MOVE.B    (A2)+,D4            
01391E  4884                      EXT.W     D4                  
013920  1A1A                      MOVE.B    (A2)+,D5            
013922  4885                      EXT.W     D5                  
013924  1C1A                      MOVE.B    (A2)+,D6            
013926  4886                      EXT.W     D6                  
013928  4A29FFFD                  TST.B     -3(A1)              
01392C  67000004                  BEQ       4(PC)                __L048C
013930  4444                      NEG.W     D4                  
013932  D841                __L048C:
                                  ADD.W     D1,D4               
013934  DA42                      ADD.W     D2,D5               
013936  DC43                      ADD.W     D3,D6               
013938  60000044                  BRA       68(PC)               __L0490
01393C  301A                __L048D:
                                  MOVE.W    (A2)+,D0            
01393E  C07900017166              AND.W     $00017166,D0         L06CE
013944  67000092                  BEQ       146(PC)              L0494
013948  33EAFFFE00017170          MOVE.W    -2(A2),$00017170     L06CF
013950  4C9A000E                  MOVEM.W   (A2)+,D1-D3         
013954  4A29FFFD                  TST.B     -3(A1)              
013958  67000004                  BEQ       4(PC)                __L048E
01395C  4441                      NEG.W     D1                  
01395E  D2690000            __L048E:
                                  ADD.W     0(A1),D1            
013962  D4690002                  ADD.W     2(A1),D2            
013966  D6690004                  ADD.W     4(A1),D3            
01396A  4C9A0070                  MOVEM.W   (A2)+,D4-D6         
01396E  4A29FFFD                  TST.B     -3(A1)              
013972  67000004                  BEQ       4(PC)                __L048F
013976  4444                      NEG.W     D4                  
013978  D841                __L048F:
                                  ADD.W     D1,D4               
01397A  DA42                      ADD.W     D2,D5               
01397C  DC43                      ADD.W     D3,D6               
01397E  BA42                __L0490:
                                  CMP.W     D2,D5               
013980  6E000004                  BGT       4(PC)                __L0491
013984  C545                      EXG       D5,D2               
013986  BA790001717E        __L0491:
                                  CMP.W     $0001717E,D5         L06D4
01398C  6D00004A                  BLT       74(PC)               L0494
013990  B47900017184              CMP.W     $00017184,D2         L06D7
013996  6E000040                  BGT       64(PC)               L0494
01399A  BC43                      CMP.W     D3,D6               
01399C  6E000004                  BGT       4(PC)                __L0492
0139A0  C746                      EXG       D6,D3               
0139A2  BC7900017180        __L0492:
                                  CMP.W     $00017180,D6         L06D5
0139A8  6D00002E                  BLT       46(PC)               L0494
0139AC  B67900017186              CMP.W     $00017186,D3         L06D8
0139B2  6E000024                  BGT       36(PC)               L0494
0139B6  B841                      CMP.W     D1,D4               
0139B8  6E000004                  BGT       4(PC)                __L0493
0139BC  C344                      EXG       D4,D1               
0139BE  B8790001717C        __L0493:
                                  CMP.W     $0001717C,D4         L06D3
0139C4  6D000012                  BLT       18(PC)               L0494
0139C8  B27900017182              CMP.W     $00017182,D1         L06D6
0139CE  6E000008                  BGT       8(PC)                L0494
0139D2  123C0001                  MOVE.B    #1,D1               
0139D6  4E75                      RTS                           
0139D8  4201                L0494:CLR.B     D1                  
0139DA  4E75                      RTS                           
0139DC  3029FFE6            __L0495:
                                  MOVE.W    -26(A1),D0          
0139E0  6B00003A                  BMI       58(PC)               L0498
0139E4  48C0                      EXT.L     D0                  
0139E6  BDC9                      CMPA.L    A1,A6               
0139E8  67000032                  BEQ       50(PC)               L0498
0139EC  48E740E0                  MOVEM.L   A0-A2/D1,-(A7)      
0139F0  2869FFEC                  MOVEA.L   -20(A1),A4          
0139F4  D9EC000E                  ADDA.L    14(A4),A4           
0139F8  D9EC0006                  ADDA.L    6(A4),A4            
0139FC  244C                      MOVEA.L   A4,A2               
0139FE  D080                      ADD.L     D0,D0               
013A00  D4F20800                  ADDA.W    0(A2,D0.L),A2       
013A04  301A                      MOVE.W    (A2)+,D0            
013A06  6A00000A                  BPL       10(PC)               __L0496
013A0A  61000020                  BSR       32(PC)               L0499
013A0E  60000006                  BRA       6(PC)                __L0497
013A12  61000052            __L0496:
                                  BSR       82(PC)               L049E
013A16  4CDF0702            __L0497:
                                  MOVEM.L   (A7)+,A0-A2/D1      
013A1A  4E75                      RTS                           
013A1C  B040                L0498:CMP.W     D0,D0               
013A1E  4E75                      RTS                           
013A20  203C00000010              MOVE.L    #$10,D0             
013A26  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
013A2C  4241                L0499:CLR.W     D1                  
013A2E  122AFFFF                  MOVE.B    -1(A2),D1           
013A32  67000022                  BEQ       34(PC)               L049C
013A36  5341                      SUBQ.W    #1,D1               
013A38  301A                L049A:MOVE.W    (A2)+,D0            
013A3A  6B000014                  BMI       20(PC)               __L049B
013A3E  48C0                      EXT.L     D0                  
013A40  48E74020                  MOVEM.L   A2/D1,-(A7)         
013A44  61000012                  BSR       18(PC)               __L049D
013A48  4CDF0402                  MOVEM.L   (A7)+,A2/D1         
013A4C  66000008                  BNE       8(PC)                L049C
013A50  51C9FFE6            __L049B:
                                  DBF       D1,-26(PC)           L049A
013A54  B040                      CMP.W     D0,D0               
013A56  4E75                L049C:RTS                           
013A58  D080                __L049D:
                                  ADD.L     D0,D0               
013A5A  244C                      MOVEA.L   A4,A2               
013A5C  D4F40800                  ADDA.W    0(A4,D0.L),A2       
013A60  4A5A                      TST.W     (A2)+               
013A62  6B00FFC8                  BMI       -56(PC)              L0499
013A66  301A                L049E:MOVE.W    (A2)+,D0            
013A68  33C000017170              MOVE.W    D0,$00017170         L06CF
013A6E  C07900017166              AND.W     $00017166,D0         L06CE
013A74  4E75                      RTS                           


; =============================================================================
; OPCODE 0x93 -  offset 0x2d54 -  address $013a76
013A76  61001678                  BSR       5752(PC)             FN_READ_OPNAME
013A7A  33C700017166              MOVE.W    D7,$00017166         L06CE
013A80  45F900017884              LEA       $00017884,A2         L0722
013A86  4241                      CLR.W     D1                  
013A88  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013A8E  6008                      BRA.S     8(PC)                __L04A0
013A90  32301004            L049F:MOVE.W    4(A0,D1.W),D1       
013A94  67000022                  BEQ       34(PC)               __L04A1
013A98  22701000            __L04A0:
                                  MOVEA.L   0(A0,D1.W),A1       
013A9C  6100FF3E                  BSR       -194(PC)             __L0495
013AA0  6700FFEE                  BEQ       -18(PC)              L049F
013AA4  3579000171700100          MOVE.W    $00017170,256(A2)    L06CF
013AAC  34C1                      MOVE.W    D1,(A2)+            
013AAE  4A3900017188              TST.B     $00017188            L06D9
013AB4  6600FFDA                  BNE       -38(PC)              L049F
013AB8  34FCFFFF            __L04A1:
                                  MOVE.W    #$FFFF,(A2)+        
013ABC  45F900010D22              LEA       $00010D22,A2         OPCODE_JUMP_TABLE
013AC2  423900017188              CLR.B     $00017188            L06D9
013AC8  6000FBDC                  BRA       -1060(PC)            L046C


; =============================================================================
; OPCODE 0x94 -  offset 0x2daa -  address $013acc
013ACC  61001622                  BSR       5666(PC)             FN_READ_OPNAME
013AD0  45F900017884              LEA       $00017884,A2         L0722
013AD6  4241                      CLR.W     D1                  
013AD8  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013ADE  6008                      BRA.S     8(PC)                __L04A3
013AE0  32301004            L04A2:MOVE.W    4(A0,D1.W),D1       
013AE4  67000024                  BEQ       36(PC)               __L04A4
013AE8  22701000            __L04A3:
                                  MOVEA.L   0(A0,D1.W),A1       
013AEC  BE69FFF0                  CMP.W     -16(A1),D7          
013AF0  6600FFEE                  BNE       -18(PC)              L04A2
013AF4  BDC9                      CMPA.L    A1,A6               
013AF6  6700FFE8                  BEQ       -24(PC)              L04A2
013AFA  426A0100                  CLR.W     256(A2)             
013AFE  34C1                      MOVE.W    D1,(A2)+            
013B00  4A3900017188              TST.B     $00017188            L06D9
013B06  6600FFD8                  BNE       -40(PC)              L04A2
013B0A  34FCFFFF            __L04A4:
                                  MOVE.W    #$FFFF,(A2)+        
013B0E  45F900010D22              LEA       $00010D22,A2         OPCODE_JUMP_TABLE
013B14  423900017188              CLR.B     $00017188            L06D9
013B1A  6000FB8A                  BRA       -1142(PC)            L046C


; =============================================================================
; OPCODE 0xca -  offset 0x2dfc -  address $013b1e
013B1E  610015D0                  BSR       5584(PC)             FN_READ_OPNAME
013B22  45F900017884              LEA       $00017884,A2         L0722
013B28  4241                      CLR.W     D1                  
013B2A  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013B30  6008                      BRA.S     8(PC)                __L04A6
013B32  32301004            L04A5:MOVE.W    4(A0,D1.W),D1       
013B36  67000024                  BEQ       36(PC)               __L04A7
013B3A  22701000            __L04A6:
                                  MOVEA.L   0(A0,D1.W),A1       
013B3E  BE29000C                  CMP.B     12(A1),D7           
013B42  6600FFEE                  BNE       -18(PC)              L04A5
013B46  BDC9                      CMPA.L    A1,A6               
013B48  6700FFE8                  BEQ       -24(PC)              L04A5
013B4C  426A0100                  CLR.W     256(A2)             
013B50  34C1                      MOVE.W    D1,(A2)+            
013B52  4A3900017188              TST.B     $00017188            L06D9
013B58  6600FFD8                  BNE       -40(PC)              L04A5
013B5C  34FCFFFF            __L04A7:
                                  MOVE.W    #$FFFF,(A2)+        
013B60  45F900010D22              LEA       $00010D22,A2         OPCODE_JUMP_TABLE
013B66  423900017188              CLR.B     $00017188            L06D9
013B6C  6000FB38                  BRA       -1224(PC)            L046C


; =============================================================================
; OPCODE 0x9c -  offset 0x2e4e -  address $013b70
013B70  45F900017884              LEA       $00017884,A2         L0722
013B76  4241                      CLR.W     D1                  
013B78  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013B7E  32301004            __L04A8:
                                  MOVE.W    4(A0,D1.W),D1       
013B82  6700000C                  BEQ       12(PC)               __L04A9
013B86  426A0100                  CLR.W     256(A2)             
013B8A  34C1                      MOVE.W    D1,(A2)+            
013B8C  6000FFF0                  BRA       -16(PC)              __L04A8
013B90  34FCFFFF            __L04A9:
                                  MOVE.W    #$FFFF,(A2)+        
013B94  45F900010D22              LEA       $00010D22,A2         OPCODE_JUMP_TABLE
013B9A  423900017188              CLR.B     $00017188            L06D9
013BA0  6000FB04                  BRA       -1276(PC)            L046C


; =============================================================================
; OPCODE 0x59 -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0x5a -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0x5b -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0x5c -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0x5d -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0x5e -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0xcb -  offset 0x2e82 -  address $013ba4


; =============================================================================
; OPCODE 0xcc -  offset 0x2e82 -  address $013ba4
013BA4  203C00000019              MOVE.L    #$19,D0             
013BAA  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR


; =============================================================================
; OPCODE 0x67 -  offset 0x2e8e -  address $013bb0
013BB0  13FC000100017188          MOVE.B    #1,$00017188         L06D9
013BB8  4E75                      RTS                           
013BBA  4E75                      RTS                           


; =============================================================================
; OPCODE 0x29 -  offset 0x2e9a -  address $013bbc
; cdim
013BBC  101B                      MOVE.B    (A3)+,D0            
013BBE  E140                      ASL.W     #8,D0               
013BC0  101B                      MOVE.B    (A3)+,D0            
013BC2  5340                      SUBQ.W    #1,D0               
013BC4  4241                      CLR.W     D1                  
013BC6  121B                      MOVE.B    (A3)+,D1            
013BC8  1D810000                  MOVE.B    D1,0(A6,D0.W)       
013BCC  5340                      SUBQ.W    #1,D0               
013BCE  1D9B0000                  MOVE.B    (A3)+,0(A6,D0.W)    
013BD2  6000000C                  BRA       12(PC)               __L04AB
013BD6  141B                __L04AA:
                                  MOVE.B    (A3)+,D2            
013BD8  E142                      ASL.W     #8,D2               
013BDA  141B                      MOVE.B    (A3)+,D2            
013BDC  3D820000                  MOVE.W    D2,0(A6,D0.W)       
013BE0  5540                __L04AB:
                                  SUBQ.W    #2,D0               
013BE2  51C9FFF2                  DBF       D1,-14(PC)           __L04AA
013BE6  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2e -  offset 0x2ec6 -  address $013be8
013BE8  61001506                  BSR       5382(PC)             FN_READ_OPNAME
013BEC  4241                      CLR.W     D1                  
013BEE  121B                      MOVE.B    (A3)+,D1            
013BF0  200B                      MOVE.L    A3,D0               
013BF2  08000000                  BTST      #0,D0               
013BF6  67000004                  BEQ       4(PC)                L04AC
013BFA  528B                      ADDQ.L    #1,A3               
013BFC  BE5B                L04AC:CMP.W     (A3)+,D7            
013BFE  67000018                  BEQ       24(PC)               __L04AE
013C02  548B                      ADDQ.L    #2,A3               
013C04  5DC9FFF6                  DBLT      D1,-10(PC)           L04AC
013C08  4A41                      TST.W     D1                  
013C0A  6A000004                  BPL       4(PC)                __L04AD
013C0E  4E75                      RTS                           
013C10  D241                __L04AD:
                                  ADD.W     D1,D1               
013C12  D241                      ADD.W     D1,D1               
013C14  D6C1                      ADDA.W    D1,A3               
013C16  4E75                      RTS                           
013C18  D6DB                __L04AE:
                                  ADDA.W    (A3)+,A3            
013C1A  4E75                      RTS                           


; =============================================================================
; OPCODE 0x2f -  offset 0x2efa -  address $013c1c
013C1C  610014D2                  BSR       5330(PC)             FN_READ_OPNAME
013C20  4241                      CLR.W     D1                  
013C22  121B                      MOVE.B    (A3)+,D1            
013C24  200B                      MOVE.L    A3,D0               
013C26  08000000                  BTST      #0,D0               
013C2A  67000004                  BEQ       4(PC)                __L04AF
013C2E  528B                      ADDQ.L    #1,A3               
013C30  DE5B                __L04AF:
                                  ADD.W     (A3)+,D7            
013C32  6B000010                  BMI       16(PC)               L04B0
013C36  BE41                      CMP.W     D1,D7               
013C38  6200000A                  BHI       10(PC)               L04B0
013C3C  DE47                      ADD.W     D7,D7               
013C3E  D6C7                      ADDA.W    D7,A3               
013C40  D6DB                      ADDA.W    (A3)+,A3            
013C42  4E75                      RTS                           
013C44  D241                L04B0:ADD.W     D1,D1               
013C46  5441                      ADDQ.W    #2,D1               
013C48  D6C1                      ADDA.W    D1,A3               
013C4A  4E75                      RTS                           
013C4C  4E75                __L04B1:
                                  RTS                           


; =============================================================================
; OPCODE 0x35 -  offset 0x2f2c -  address $013c4e
013C4E  610014A0                  BSR       5280(PC)             FN_READ_OPNAME
013C52  4A47                      TST.W     D7                  
013C54  6B000010                  BMI       16(PC)               __L04B2
013C58  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013C5E  20707000                  MOVEA.L   0(A0,D7.W),A0       
013C62  4228FFFC                  CLR.B     -4(A0)              
013C66  4E75                __L04B2:
                                  RTS                           


; =============================================================================
; OPCODE 0x3f -  offset 0x2f46 -  address $013c68
013C68  422EFFFC                  CLR.B     -4(A6)              
013C6C  4A3900017107              TST.B     $00017107            B_06A7
013C72  6600DD62                  BNE       -8862(PC)            OP_CSTOP
013C76  4E75                      RTS                           


; =============================================================================
; OPCODE 0x3e -  offset 0x2f56 -  address $013c78
013C78  61001476                  BSR       5238(PC)             FN_READ_OPNAME
013C7C  4A47                      TST.W     D7                  
013C7E  6B000012                  BMI       18(PC)               __L04B3
013C82  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013C88  20707000                  MOVEA.L   0(A0,D7.W),A0       
013C8C  117C0001FFFC              MOVE.B    #1,-4(A0)           
013C92  4E75                __L04B3:
                                  RTS                           


; =============================================================================
; OPCODE 0x61 -  offset 0x2f72 -  address $013c94
013C94  4241                      CLR.W     D1                  
013C96  121B                      MOVE.B    (A3)+,D1            
013C98  48E74000                  MOVEM.L   D1,-(A7)            
013C9C  61001452                  BSR       5202(PC)             FN_READ_OPNAME
013CA0  4CDF0002                  MOVEM.L   (A7)+,D1            
013CA4  BE7CFFFF                  CMP.W     #$FFFF,D7           
013CA8  67000066                  BEQ       102(PC)              L04B7
013CAC  2079000170A8              MOVEA.L   $000170A8,A0         L067A
013CB2  20307000                  MOVE.L    0(A0,D7.W),D0       
013CB6  67000058                  BEQ       88(PC)               L04B7
013CBA  2040                      MOVEA.L   D0,A0               
013CBC  08280000FFDC              BTST      #0,-36(A0)          
013CC2  6600004C                  BNE       76(PC)               L04B7
013CC6  3428FFE4                  MOVE.W    -28(A0),D2          
013CCA  2268FFEC                  MOVEA.L   -20(A0),A1          
013CCE  30290016                  MOVE.W    22(A1),D0           
013CD2  48E7E080            __L04B4:
                                  MOVEM.L   A0/D0-D2,-(A7)      
013CD6  61001418                  BSR       5144(PC)             FN_READ_OPNAME
013CDA  4CDF0107                  MOVEM.L   (A7)+,A0/D0-D2      
013CDE  3242                      MOVEA.W   D2,A1               
013CE0  5442                      ADDQ.W    #2,D2               
013CE2  B47CFFCC                  CMP.W     #$FFCC,D2           
013CE6  6D000004                  BLT       4(PC)                __L04B5
013CEA  9440                      SUB.W     D0,D2               
013CEC  B468FFE2            __L04B5:
                                  CMP.W     -30(A0),D2          
013CF0  6600000A                  BNE       10(PC)               __L04B6
013CF4  3142FFE4                  MOVE.W    D2,-28(A0)          
013CF8  60000022                  BRA       34(PC)               __L04B8
013CFC  31879000            __L04B6:
                                  MOVE.W    D7,0(A0,A1.W)       
013D00  51C9FFD0                  DBF       D1,-48(PC)           __L04B4
013D04  3142FFE4                  MOVE.W    D2,-28(A0)          
013D08  08E80007FFDC              BSET      #7,-36(A0)          
013D0E  4E75                      RTS                           
013D10  48E74000            L04B7:MOVEM.L   D1,-(A7)            
013D14  610013DA                  BSR       5082(PC)             FN_READ_OPNAME
013D18  4CDF0002                  MOVEM.L   (A7)+,D1            
013D1C  51C9FFF2            __L04B8:
                                  DBF       D1,-14(PC)           L04B7
013D20  4E75                      RTS                           
013D22  322EFFE2                  MOVE.W    -30(A6),D1          
013D26  B26EFFE4                  CMP.W     -28(A6),D1          
013D2A  66000006                  BNE       6(PC)                __L04B9
013D2E  7EFF                      MOVEQ     #$FF,D7             
013D30  4E75                      RTS                           
013D32  226EFFEC            __L04B9:
                                  MOVEA.L   -20(A6),A1          
013D36  30290016                  MOVE.W    22(A1),D0           
013D3A  3E361000                  MOVE.W    0(A6,D1.W),D7       
013D3E  48C7                      EXT.L     D7                  
013D40  5441                      ADDQ.W    #2,D1               
013D42  B27CFFCC                  CMP.W     #$FFCC,D1           
013D46  6D000004                  BLT       4(PC)                __L04BA
013D4A  9240                      SUB.W     D0,D1               
013D4C  3D41FFE2            __L04BA:
                                  MOVE.W    D1,-30(A6)          
013D50  B26EFFE4                  CMP.W     -28(A6),D1          
013D54  66000008                  BNE       8(PC)                __L04BB
013D58  08AE0007FFDC              BCLR      #7,-36(A6)          
013D5E  4E75                __L04BB:
                                  RTS                           


; =============================================================================
; OPCODE 0x66 -  offset 0x303e -  address $013d60
013D60  3D6EFFE4FFE2        __L04BC:
                                  MOVE.W    -28(A6),-30(A6)     
013D66  08AE0007FFDC              BCLR      #7,-36(A6)          
013D6C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x63 -  offset 0x304c -  address $013d6e
013D6E  08EE0000FFDC              BSET      #0,-36(A6)          
013D74  6100FFEA                  BSR       -22(PC)              __L04BC
013D78  4E75                      RTS                           


; =============================================================================
; OPCODE 0x62 -  offset 0x3058 -  address $013d7a
013D7A  08AE0000FFDC              BCLR      #0,-36(A6)          
013D80  4E75                      RTS                           


; =============================================================================
; OPCODE 0x65 -  offset 0x3060 -  address $013d82
013D82  08EE0001FFDC              BSET      #1,-36(A6)          
013D88  4E75                      RTS                           


; =============================================================================
; OPCODE 0x64 -  offset 0x3068 -  address $013d8a
013D8A  08AE0001FFDC              BCLR      #1,-36(A6)          
013D90  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6a -  offset 0x3070 -  address $013d92
013D92  6100135C                  BSR       4956(PC)             FN_READ_OPNAME
013D96  13C70001718A              MOVE.B    D7,$0001718A         L06DB
013D9C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6f -  offset 0x307c -  address $013d9e
013D9E  101B                      MOVE.B    (A3)+,D0            
013DA0  E140                      ASL.W     #8,D0               
013DA2  101B                      MOVE.B    (A3)+,D0            
013DA4  121B                      MOVE.B    (A3)+,D1            
013DA6  E141                      ASL.W     #8,D1               
013DA8  121B                      MOVE.B    (A3)+,D1            
013DAA  5341                      SUBQ.W    #1,D1               
013DAC  41F60000                  LEA       0(A6,D0.W),A0       
013DB0  10DB                __L04BD:
                                  MOVE.B    (A3)+,(A0)+         
013DB2  51C9FFFC                  DBF       D1,-4(PC)            __L04BD
013DB6  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd8 -  offset 0x3096 -  address $013db8
013DB8  61001336                  BSR       4918(PC)             FN_READ_OPNAME
013DBC  61001324                  BSR       4900(PC)             L0548
013DC0  4A47                      TST.W     D7                  
013DC2  6B000074                  BMI       116(PC)              __L04C3
013DC6  BE7C00C7                  CMP.W     #$C7,D7             
013DCA  6D000006                  BLT       6(PC)                __L04BE
013DCE  3E3C00C7                  MOVE.W    #$C7,D7             
013DD2  41F90000AE3A        __L04BE:
                                  LEA       $0000AE3A,A0         L002C
013DD8  303C00C8                  MOVE.W    #$C8,D0             
013DDC  BE50                __L04BF:
                                  CMP.W     (A0),D7             
013DDE  6700003C                  BEQ       60(PC)               __L04C2
013DE2  B050                      CMP.W     (A0),D0             
013DE4  5848                      ADDQ.W    #4,A0               
013DE6  6200FFF4                  BHI       -12(PC)              __L04BF
013DEA  B1FC0000AE4A              CMPA.L    #$0000AE4A,A0        __L002F
013DF0  6A000028                  BPL       40(PC)               L04C1
013DF4  31600004            __L04C0:
                                  MOVE.W    -(A0),4(A0)         
013DF8  3020                      MOVE.W    -(A0),D0            
013DFA  31400004                  MOVE.W    D0,4(A0)            
013DFE  BE40                      CMP.W     D0,D7               
013E00  6B00FFF2                  BMI       -14(PC)              __L04C0
013E04  31470004                  MOVE.W    D7,4(A0)            
013E08  31460006                  MOVE.W    D6,6(A0)            
013E0C  13FC0001000170DF          MOVE.B    #1,$000170DF         L0691
013E14  4EB90000B4DA              JSR       $0000B4DA            L004B
013E1A  4E75                L04C1:RTS                           
013E1C  BC680002            __L04C2:
                                  CMP.W     2(A0),D6            
013E20  6700FFF8                  BEQ       -8(PC)               L04C1
013E24  31460002                  MOVE.W    D6,2(A0)            
013E28  13FC0001000170DF          MOVE.B    #1,$000170DF         L0691
013E30  4EB90000B4DA              JSR       $0000B4DA            L004B
013E36  4E75                      RTS                           
013E38  4239000170DF        __L04C3:
                                  CLR.B     $000170DF            L0691
013E3E  42B90000AE3A              CLR.L     $0000AE3A            L002C
013E44  33FC00FF0000AE3E          MOVE.W    #$FF,$0000AE3E       __L002D
013E4C  42790000AE40              CLR.W     $0000AE40            __L002E
013E52  4EB90000B4DA              JSR       $0000B4DA            L004B
013E58  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd7 -  offset 0x3138 -  address $013e5a
013E5A  61001294                  BSR       4756(PC)             FN_READ_OPNAME
013E5E  02470003                  ANDI.W    #3,D7               
013E62  13C7000171EA              MOVE.B    D7,$000171EA         L06FB
013E68  13FC0001000171EB          MOVE.B    #1,$000171EB         L06FC
013E70  4E75                      RTS                           
013E72  48E78080            L04C4:MOVEM.L   A0/D0,-(A7)         
013E76  4240                      CLR.W     D0                  
013E78  1039000171EA              MOVE.B    $000171EA,D0         L06FB
013E7E  EB48                      LSL.W     #5,D0               
013E80  207C0000ACEE              MOVEA.L   #$0000ACEE,A0        L0023
013E86  D0C0                      ADDA.W    D0,A0               
013E88  23C8000171EC              MOVE.L    A0,$000171EC         L06FD
013E8E  207C0000AD6E              MOVEA.L   #$0000AD6E,A0        L0026
013E94  D0C0                      ADDA.W    D0,A0               
013E96  23C8000171F0              MOVE.L    A0,$000171F0         L06FE
013E9C  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
013EA0  4E75                      RTS                           


; =============================================================================
; OPCODE 0x68 -  offset 0x3180 -  address $013ea2
013EA2  423900017103              CLR.B     $00017103            L06A4
013EA8  61001246                  BSR       4678(PC)             FN_READ_OPNAME
013EAC  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
013EB4  67000066                  BEQ       102(PC)              L04C7
013EB8  0C390007000170DE          CMPI.B    #7,$000170DE         L0690
013EC0  6700005A                  BEQ       90(PC)               L04C7
013EC4  4A47                      TST.W     D7                  
013EC6  6A000030                  BPL       48(PC)               __L04C6
013ECA  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
013ED2  6700001C                  BEQ       28(PC)               L04C5
013ED6  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
013EDE  67000010                  BEQ       16(PC)               L04C5
013EE2  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
013EEA  67000004                  BEQ       4(PC)                L04C5
013EEE  4E75                      RTS                           
013EF0  4EB90000BF6C        L04C5:JSR       $0000BF6C            L00A8
013EF6  4E75                      RTS                           
013EF8  0C390001000170DE    __L04C6:
                                  CMPI.B    #1,$000170DE         L0690
013F00  6700001A                  BEQ       26(PC)               L04C7
013F04  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
013F0C  6700000E                  BEQ       14(PC)               L04C7
013F10  6100E5B4                  BSR       -6732(PC)            L03D8
013F14  0C1000FE                  CMPI.B    #$FE,(A0)           
013F18  67000004                  BEQ       4(PC)                L04C8
013F1C  4E75                L04C7:RTS                           
013F1E  4240                L04C8:CLR.W     D0                  
013F20  10280001                  MOVE.B    1(A0),D0            
013F24  5340                      SUBQ.W    #1,D0               
013F26  5488                      ADDQ.L    #2,A0               
013F28  48E720E0            __L04C9:
                                  MOVEM.L   A0-A2/D2,-(A7)      
013F2C  4EB900013E72              JSR       $00013E72            L04C4
013F32  2279000171EC              MOVEA.L   $000171EC,A1         L06FD
013F38  2479000171F0              MOVEA.L   $000171F0,A2         L06FE
013F3E  4A00                      TST.B     D0                  
013F40  6A000006                  BPL       6(PC)                __L04CA
013F44  303C000F                  MOVE.W    #$F,D0              
013F48  4201                __L04CA:
                                  CLR.B     D1                  
013F4A  4A39000171EB              TST.B     $000171EB            L06FC
013F50  6600000E                  BNE       14(PC)               L04CB
013F54  B03C000F                  CMP.B     #$F,D0              
013F58  67000006                  BEQ       6(PC)                L04CB
013F5C  123C0001                  MOVE.B    #1,D1               
013F60  3418                L04CB:MOVE.W    (A0)+,D2            
013F62  32C2                      MOVE.W    D2,(A1)+            
013F64  34C2                      MOVE.W    D2,(A2)+            
013F66  51C8FFF8                  DBF       D0,-8(PC)            L04CB
013F6A  4A01                      TST.B     D1                  
013F6C  67000012                  BEQ       18(PC)               __L04CD
013F70  303C000F                  MOVE.W    #$F,D0              
013F74  41F90000ADEE              LEA       $0000ADEE,A0         L0027
013F7A  32D8                __L04CC:
                                  MOVE.W    (A0)+,(A1)+         
013F7C  51C8FFFC                  DBF       D0,-4(PC)            __L04CC
013F80  4CDF0704            __L04CD:
                                  MOVEM.L   (A7)+,A0-A2/D2      
013F84  13FC00FF0001718C    L04CE:MOVE.B    #$FF,$0001718C       L06DD
013F8C  4239000171EA              CLR.B     $000171EA            L06FB
013F92  4239000171EB              CLR.B     $000171EB            L06FC
013F98  4E75                      RTS                           


; =============================================================================
; OPCODE 0x69 -  offset 0x3278 -  address $013f9a
013F9A  61001154                  BSR       4436(PC)             FN_READ_OPNAME
013F9E  61001142                  BSR       4418(PC)             L0548
013FA2  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
013FAA  67000036                  BEQ       54(PC)               L04CF
013FAE  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
013FB6  6700002A                  BEQ       42(PC)               L04CF
013FBA  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
013FC2  6700001E                  BEQ       30(PC)               L04CF
013FC6  0C390007000170DE          CMPI.B    #7,$000170DE         L0690
013FCE  67000012                  BEQ       18(PC)               L04CF
013FD2  2079000171EC              MOVEA.L   $000171EC,A0         L06FD
013FD8  DE47                      ADD.W     D7,D7               
013FDA  31867000                  MOVE.W    D6,0(A0,D7.W)       
013FDE  6100FFA4                  BSR       -92(PC)              L04CE
013FE2  4E75                L04CF:RTS                           


; =============================================================================
; OPCODE 0xbd -  offset 0x32c2 -  address $013fe4
013FE4  6100110A                  BSR       4362(PC)             FN_READ_OPNAME
013FE8  610010F8                  BSR       4344(PC)             L0548
013FEC  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
013FF4  6700006C                  BEQ       108(PC)              L04D1
013FF8  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
014000  67000060                  BEQ       96(PC)               L04D1
014004  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
01400C  67000054                  BEQ       84(PC)               L04D1
014010  0C390007000170DE          CMPI.B    #7,$000170DE         L0690
014018  67000048                  BEQ       72(PC)               L04D1
01401C  2079000171EC              MOVEA.L   $000171EC,A0         L06FD
014022  DE47                      ADD.W     D7,D7               
014024  34307000                  MOVE.W    0(A0,D7.W),D2       
014028  4243                      CLR.W     D3                  
01402A  4A46                      TST.W     D6                  
01402C  6B000036                  BMI       54(PC)               __L04D2
014030  3006                      MOVE.W    D6,D0               
014032  3202                      MOVE.W    D2,D1               
014034  6100005E                  BSR       94(PC)               L04D3
014038  3601                      MOVE.W    D1,D3               
01403A  3202                      MOVE.W    D2,D1               
01403C  3006                      MOVE.W    D6,D0               
01403E  E848                      LSR.W     #4,D0               
014040  E849                      LSR.W     #4,D1               
014042  61000050                  BSR       80(PC)               L04D3
014046  E949                      LSL.W     #4,D1               
014048  8641                      OR.W      D1,D3               
01404A  3006                      MOVE.W    D6,D0               
01404C  3202                      MOVE.W    D2,D1               
01404E  E048                      LSR.W     #8,D0               
014050  E049                      LSR.W     #8,D1               
014052  61000040                  BSR       64(PC)               L04D3
014056  E149                      LSL.W     #8,D1               
014058  8641                      OR.W      D1,D3               
01405A  31837000            __L04D0:
                                  MOVE.W    D3,0(A0,D7.W)       
01405E  6100FF24                  BSR       -220(PC)             L04CE
014062  4E75                L04D1:RTS                           
014064  4446                __L04D2:
                                  NEG.W     D6                  
014066  3006                      MOVE.W    D6,D0               
014068  3202                      MOVE.W    D2,D1               
01406A  6100004A                  BSR       74(PC)               L04D6
01406E  3601                      MOVE.W    D1,D3               
014070  3202                      MOVE.W    D2,D1               
014072  3006                      MOVE.W    D6,D0               
014074  E848                      LSR.W     #4,D0               
014076  E849                      LSR.W     #4,D1               
014078  6100003C                  BSR       60(PC)               L04D6
01407C  E949                      LSL.W     #4,D1               
01407E  8641                      OR.W      D1,D3               
014080  3006                      MOVE.W    D6,D0               
014082  3202                      MOVE.W    D2,D1               
014084  E048                      LSR.W     #8,D0               
014086  E049                      LSR.W     #8,D1               
014088  6100002C                  BSR       44(PC)               L04D6
01408C  E149                      LSL.W     #8,D1               
01408E  8641                      OR.W      D1,D3               
014090  6000FFC8                  BRA       -56(PC)              __L04D0
014094  0241000F            L04D3:ANDI.W    #$F,D1              
014098  0240000F                  ANDI.W    #$F,D0              
01409C  D240                      ADD.W     D0,D1               
01409E  6B000012                  BMI       18(PC)               __L04D5
0140A2  B27C0007                  CMP.W     #7,D1               
0140A6  62000004                  BHI       4(PC)                __L04D4
0140AA  4E75                      RTS                           
0140AC  323C0007            __L04D4:
                                  MOVE.W    #7,D1               
0140B0  4E75                      RTS                           
0140B2  4241                __L04D5:
                                  CLR.W     D1                  
0140B4  4E75                      RTS                           
0140B6  0241000F            L04D6:ANDI.W    #$F,D1              
0140BA  0240000F                  ANDI.W    #$F,D0              
0140BE  9240                      SUB.W     D0,D1               
0140C0  6B000012                  BMI       18(PC)               __L04D8
0140C4  B27C0007                  CMP.W     #7,D1               
0140C8  62000004                  BHI       4(PC)                __L04D7
0140CC  4E75                      RTS                           
0140CE  323C0007            __L04D7:
                                  MOVE.W    #7,D1               
0140D2  4E75                      RTS                           
0140D4  4241                __L04D8:
                                  CLR.W     D1                  
0140D6  4E75                      RTS                           


; =============================================================================
; OPCODE 0xbe -  offset 0x33b6 -  address $0140d8
0140D8  423900017103              CLR.B     $00017103            L06A4
0140DE  61001010                  BSR       4112(PC)             FN_READ_OPNAME
0140E2  61000FFE                  BSR       4094(PC)             L0548
0140E6  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
0140EE  67000032                  BEQ       50(PC)               L04D9
0140F2  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
0140FA  67000026                  BEQ       38(PC)               L04D9
0140FE  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
014106  6700001A                  BEQ       26(PC)               L04D9
01410A  0C390007000170DE          CMPI.B    #7,$000170DE         L0690
014112  6700000E                  BEQ       14(PC)               L04D9
014116  6100E3AE                  BSR       -7250(PC)            L03D8
01411A  0C1000FE                  CMPI.B    #$FE,(A0)           
01411E  67000004                  BEQ       4(PC)                __L04DA
014122  4E75                L04D9:RTS                           
014124  5488                __L04DA:
                                  ADDQ.L    #2,A0               
014126  4240                      CLR.W     D0                  
014128  1028FFFF                  MOVE.B    -1(A0),D0           
01412C  5300                      SUBQ.B    #1,D0               
01412E  6A000006                  BPL       6(PC)                L04DB
014132  103C000F                  MOVE.B    #$F,D0              
014136  4201                L04DB:CLR.B     D1                  
014138  4A39000171EB              TST.B     $000171EB            L06FC
01413E  6600000E                  BNE       14(PC)               L04DC
014142  B03C000F                  CMP.B     #$F,D0              
014146  67000006                  BEQ       6(PC)                L04DC
01414A  123C0001                  MOVE.B    #1,D1               
01414E  42390001718C        L04DC:CLR.B     $0001718C            L06DD
014154  4EB900013E72              JSR       $00013E72            L04C4
01415A  2279000171F0              MOVEA.L   $000171F0,A1         L06FE
014160  32D8                __L04DD:
                                  MOVE.W    (A0)+,(A1)+         
014162  51C8FFFC                  DBF       D0,-4(PC)            __L04DD
014166  4A01                      TST.B     D1                  
014168  67000012                  BEQ       18(PC)               __L04DF
01416C  303C000F                  MOVE.W    #$F,D0              
014170  41F90000ADEE              LEA       $0000ADEE,A0         L0027
014176  32D8                __L04DE:
                                  MOVE.W    (A0)+,(A1)+         
014178  51C8FFFC                  DBF       D0,-4(PC)            __L04DE
01417C  4A46                __L04DF:
                                  TST.W     D6                  
01417E  66000030                  BNE       48(PC)               __L04E0
014182  13FC00010001718F          MOVE.B    #1,$0001718F         L06E0
01418A  13FC00010001718E          MOVE.B    #1,$0001718E         L06DF
014192  13FC00010001718D          MOVE.B    #1,$0001718D         L06DE
01419A  13FC00010001718C          MOVE.B    #1,$0001718C         L06DD
0141A2  4239000171EA              CLR.B     $000171EA            L06FB
0141A8  4239000171EB              CLR.B     $000171EB            L06FC
0141AE  4E75                      RTS                           
0141B0  13FC000F0001718D    __L04E0:
                                  MOVE.B    #$F,$0001718D        L06DE
0141B8  E84E                      LSR.W     #4,D6               
0141BA  66000006                  BNE       6(PC)                __L04E1
0141BE  3C3C0001                  MOVE.W    #1,D6               
0141C2  13C60001718E        __L04E1:
                                  MOVE.B    D6,$0001718E         L06DF
0141C8  13C60001718F              MOVE.B    D6,$0001718F         L06E0
0141CE  13FC00010001718C          MOVE.B    #1,$0001718C         L06DD
0141D6  4239000171EA              CLR.B     $000171EA            L06FB
0141DC  4239000171EB              CLR.B     $000171EB            L06FC
0141E2  4E75                      RTS                           
0141E4  41F90000AD6E        __L04E2:
                                  LEA       $0000AD6E,A0         L0026
0141EA  43F90000ACEE              LEA       $0000ACEE,A1         L0023
0141F0  3E3C003F                  MOVE.W    #$3F,D7             ;(#"?") 
0141F4  DE47                __L04E3:
                                  ADD.W     D7,D7               
0141F6  3C307000                  MOVE.W    0(A0,D7.W),D6       
0141FA  34317000                  MOVE.W    0(A1,D7.W),D2       
0141FE  BC42                      CMP.W     D2,D6               
014200  67000080                  BEQ       128(PC)              __L04EA
014204  4243                      CLR.W     D3                  
014206  3006                      MOVE.W    D6,D0               
014208  3202                      MOVE.W    D2,D1               
01420A  0240000F                  ANDI.W    #$F,D0              
01420E  0241000F                  ANDI.W    #$F,D1              
014212  9041                      SUB.W     D1,D0               
014214  67000012                  BEQ       18(PC)               L04E5
014218  6A00000A                  BPL       10(PC)               __L04E4
01421C  610000D4                  BSR       212(PC)              L04EE
014220  60000006                  BRA       6(PC)                L04E5
014224  610000C2            __L04E4:
                                  BSR       194(PC)              L04ED
014228  3601                L04E5:MOVE.W    D1,D3               
01422A  3006                      MOVE.W    D6,D0               
01422C  3202                      MOVE.W    D2,D1               
01422E  E848                      LSR.W     #4,D0               
014230  E849                      LSR.W     #4,D1               
014232  0240000F                  ANDI.W    #$F,D0              
014236  0241000F                  ANDI.W    #$F,D1              
01423A  9041                      SUB.W     D1,D0               
01423C  67000012                  BEQ       18(PC)               L04E7
014240  6A00000A                  BPL       10(PC)               __L04E6
014244  610000AC                  BSR       172(PC)              L04EE
014248  60000006                  BRA       6(PC)                L04E7
01424C  6100009A            __L04E6:
                                  BSR       154(PC)              L04ED
014250  E949                L04E7:LSL.W     #4,D1               
014252  8641                      OR.W      D1,D3               
014254  3006                      MOVE.W    D6,D0               
014256  3202                      MOVE.W    D2,D1               
014258  E048                      LSR.W     #8,D0               
01425A  E049                      LSR.W     #8,D1               
01425C  0240000F                  ANDI.W    #$F,D0              
014260  0241000F                  ANDI.W    #$F,D1              
014264  9041                      SUB.W     D1,D0               
014266  67000012                  BEQ       18(PC)               L04E9
01426A  6A00000A                  BPL       10(PC)               __L04E8
01426E  61000082                  BSR       130(PC)              L04EE
014272  60000006                  BRA       6(PC)                L04E9
014276  61000070            __L04E8:
                                  BSR       112(PC)              L04ED
01427A  E149                L04E9:LSL.W     #8,D1               
01427C  8641                      OR.W      D1,D3               
01427E  33837000                  MOVE.W    D3,0(A1,D7.W)       
014282  E24F                __L04EA:
                                  LSR.W     #1,D7               
014284  51CFFF6E                  DBF       D7,-146(PC)          __L04E3
014288  207C0000ACEE              MOVEA.L   #$0000ACEE,A0        L0023
01428E  4EB90000BF2C              JSR       $0000BF2C            L00A4
014294  4E75                      RTS                           


; =============================================================================
; OPCODE 0xbc -  offset 0x3574 -  address $014296
014296  61000E4A                  BSR       3658(PC)             L0548
01429A  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
0142A2  67000042                  BEQ       66(PC)               L04EC
0142A6  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
0142AE  67000036                  BEQ       54(PC)               L04EC
0142B2  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
0142BA  6700002A                  BEQ       42(PC)               L04EC
0142BE  0C390007000170DE          CMPI.B    #7,$000170DE         L0690
0142C6  6700001E                  BEQ       30(PC)               L04EC
0142CA  303C000F                  MOVE.W    #$F,D0              
0142CE  4A39000171EB              TST.B     $000171EB            L06FC
0142D4  66000006                  BNE       6(PC)                __L04EB
0142D8  303C000F                  MOVE.W    #$F,D0              
0142DC  41F90000ADEE        __L04EB:
                                  LEA       $0000ADEE,A0         L0027
0142E2  6000FE52                  BRA       -430(PC)             L04DB
0142E6  4E75                L04EC:RTS                           
0142E8  303C0001            L04ED:MOVE.W    #1,D0               
0142EC  6100FDA6                  BSR       -602(PC)             L04D3
0142F0  4E75                      RTS                           
0142F2  303C0001            L04EE:MOVE.W    #1,D0               
0142F6  6100FDBE                  BSR       -578(PC)             L04D6
0142FA  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd1 -  offset 0x35da -  address $0142fc
0142FC  61000DF2                  BSR       3570(PC)             FN_READ_OPNAME
014300  3F07                      MOVE.W    D7,-(A7)            
014302  61000DEC                  BSR       3564(PC)             FN_READ_OPNAME
014306  3F07                      MOVE.W    D7,-(A7)            
014308  61000DE6                  BSR       3558(PC)             FN_READ_OPNAME
01430C  3F07                      MOVE.W    D7,-(A7)            
01430E  61000DE0                  BSR       3552(PC)             FN_READ_OPNAME
014312  3F07                      MOVE.W    D7,-(A7)            
014314  61000DDA                  BSR       3546(PC)             FN_READ_OPNAME
014318  3F07                      MOVE.W    D7,-(A7)            
01431A  61000DD4                  BSR       3540(PC)             FN_READ_OPNAME
01431E  3C1F                      MOVE.W    (A7)+,D6            
014320  361F                      MOVE.W    (A7)+,D3            
014322  341F                      MOVE.W    (A7)+,D2            
014324  321F                      MOVE.W    (A7)+,D1            
014326  301F                      MOVE.W    (A7)+,D0            
014328  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
01432E  4A6EFFEA                  TST.W     -22(A6)             
014332  6B00002A                  BMI       42(PC)               L04EF
014336  D0EEFFEA                  ADDA.W    -22(A6),A0          
01433A  08A800030001              BCLR      #3,1(A0)            
014340  08A800040001              BCLR      #4,1(A0)            
014346  4A40                      TST.W     D0                  
014348  67000014                  BEQ       20(PC)               L04EF
01434C  08E800030001              BSET      #3,1(A0)            
014352  08E800040001              BSET      #4,1(A0)            
014358  4EB900011AD4              JSR       $00011AD4            __L0392
01435E  4E75                L04EF:RTS                           


; =============================================================================
; OPCODE 0xd2 -  offset 0x363e -  address $014360
014360  61000D8E                  BSR       3470(PC)             FN_READ_OPNAME
014364  33C7000143B2              MOVE.W    D7,$000143B2         __L04F0
01436A  61000D84                  BSR       3460(PC)             FN_READ_OPNAME
01436E  33C7000143B4              MOVE.W    D7,$000143B4         __L04F1
014374  61000D7A                  BSR       3450(PC)             FN_READ_OPNAME
014378  33C7000143B6              MOVE.W    D7,$000143B6         __L04F2
01437E  61000D70                  BSR       3440(PC)             FN_READ_OPNAME
014382  33C7000143B8              MOVE.W    D7,$000143B8         __L04F3
014388  61000D66                  BSR       3430(PC)             FN_READ_OPNAME
01438C  33C7000143BA              MOVE.W    D7,$000143BA         __L04F4
014392  61000D5C                  BSR       3420(PC)             FN_READ_OPNAME
014396  33C7000143BC              MOVE.W    D7,$000143BC         __L04F5
01439C  61000D52                  BSR       3410(PC)             FN_READ_OPNAME
0143A0  33C7000143BE              MOVE.W    D7,$000143BE         __L04F6
0143A6  61000D48                  BSR       3400(PC)             FN_READ_OPNAME
0143AA  33C7000143C0              MOVE.W    D7,$000143C0         __L04F7
0143B0  4E75                      RTS                           
0143B2  0000                __L04F0:
                                  DC.B      $00,$00 
0143B4  0000                __L04F1:
                                  DC.B      $00,$00 
0143B6  0000                __L04F2:
                                  DC.B      $00,$00 
0143B8  0000                __L04F3:
                                  DC.B      $00,$00 
0143BA  0000                __L04F4:
                                  DC.B      $00,$00 
0143BC  0000                __L04F5:
                                  DC.B      $00,$00 
0143BE  0000                __L04F6:
                                  DC.B      $00,$00 
0143C0  0000                __L04F7:
                                  DC.B      $00,$00 


; =============================================================================
; OPCODE 0x6b -  offset 0x36a0 -  address $0143c2
0143C2  33EEFFF2000171A8          MOVE.W    -14(A6),$000171A8    L06E9
0143CA  61000D24                  BSR       3364(PC)             FN_READ_OPNAME
0143CE  13C70001719C              MOVE.B    D7,$0001719C         L06E2
0143D4  61000D1A                  BSR       3354(PC)             FN_READ_OPNAME
0143D8  13C7000171A4              MOVE.B    D7,$000171A4         L06E7
0143DE  61000D10                  BSR       3344(PC)             FN_READ_OPNAME
0143E2  33C7000171A0              MOVE.W    D7,$000171A0         L06E5
0143E8  61000D06                  BSR       3334(PC)             FN_READ_OPNAME
0143EC  33C70001719E              MOVE.W    D7,$0001719E         L06E4
0143F2  66000004                  BNE       4(PC)                __L04F8
0143F6  4E75                      RTS                           
0143F8  61000CF6            __L04F8:
                                  BSR       3318(PC)             FN_READ_OPNAME
0143FC  33C7000171A2              MOVE.W    D7,$000171A2         L06E6
014402  4279000171A6              CLR.W     $000171A6            L06E8
014408  13FC00010001719D          MOVE.B    #1,$0001719D         L06E3
014410  4EB90001456E              JSR       $0001456E            L04FF
014416  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6c -  offset 0x36f6 -  address $014418
014418  33EEFFF2000171A8          MOVE.W    -14(A6),$000171A8    L06E9
014420  61000CCE                  BSR       3278(PC)             FN_READ_OPNAME
014424  13C70001719C              MOVE.B    D7,$0001719C         L06E2
01442A  61000CC4                  BSR       3268(PC)             FN_READ_OPNAME
01442E  13C7000171A4              MOVE.B    D7,$000171A4         L06E7
014434  61000CBA                  BSR       3258(PC)             FN_READ_OPNAME
014438  33C7000171A0              MOVE.W    D7,$000171A0         L06E5
01443E  61000CB0                  BSR       3248(PC)             FN_READ_OPNAME
014442  33C70001719E              MOVE.W    D7,$0001719E         L06E4
014448  66000004                  BNE       4(PC)                __L04F9
01444C  4E75                      RTS                           
01444E  4279000171A2        __L04F9:
                                  CLR.W     $000171A2            L06E6
014454  4280                      CLR.L     D0                  
014456  1039000171A4              MOVE.B    $000171A4,D0         L06E7
01445C  E148                      LSL.W     #8,D0               
01445E  80F90001719E              DIVU      $0001719E,D0         L06E4
014464  66000006                  BNE       6(PC)                __L04FA
014468  303C0001                  MOVE.W    #1,D0               
01446C  4440                __L04FA:
                                  NEG.W     D0                  
01446E  33C0000171A6              MOVE.W    D0,$000171A6         L06E8
014474  13FC00030001719D          MOVE.B    #3,$0001719D         L06E3
01447C  4EB90001456E              JSR       $0001456E            L04FF
014482  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6e -  offset 0x3762 -  address $014484
014484  33EEFFF2000171A8          MOVE.W    -14(A6),$000171A8    L06E9
01448C  61000C62                  BSR       3170(PC)             FN_READ_OPNAME
014490  13C70001719C              MOVE.B    D7,$0001719C         L06E2
014496  61000C58                  BSR       3160(PC)             FN_READ_OPNAME
01449A  13C7000171A4              MOVE.B    D7,$000171A4         L06E7
0144A0  61000C4E                  BSR       3150(PC)             FN_READ_OPNAME
0144A4  33C7000171A0              MOVE.W    D7,$000171A0         L06E5
0144AA  61000C44                  BSR       3140(PC)             FN_READ_OPNAME
0144AE  33C70001719E              MOVE.W    D7,$0001719E         L06E4
0144B4  66000004                  BNE       4(PC)                __L04FB
0144B8  4E75                      RTS                           
0144BA  4279000171A2        __L04FB:
                                  CLR.W     $000171A2            L06E6
0144C0  4280                      CLR.L     D0                  
0144C2  1039000171A4              MOVE.B    $000171A4,D0         L06E7
0144C8  E148                      LSL.W     #8,D0               
0144CA  80F90001719E              DIVU      $0001719E,D0         L06E4
0144D0  66000006                  BNE       6(PC)                __L04FC
0144D4  303C0001                  MOVE.W    #1,D0               
0144D8  4440                __L04FC:
                                  NEG.W     D0                  
0144DA  33C0000171A6              MOVE.W    D0,$000171A6         L06E8
0144E0  13FC00020001719D          MOVE.B    #2,$0001719D         L06E3
0144E8  4EB90001456E              JSR       $0001456E            L04FF
0144EE  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd3 -  offset 0x37ce -  address $0144f0
; cengine
0144F0  61000BFE                  BSR       3070(PC)             FN_READ_OPNAME
0144F4  61000BFA                  BSR       3066(PC)             FN_READ_OPNAME
0144F8  61000BF6                  BSR       3062(PC)             FN_READ_OPNAME
0144FC  61000BF2                  BSR       3058(PC)             FN_READ_OPNAME
014500  4E75                      RTS                           


; =============================================================================
; OPCODE 0x6d -  offset 0x37e0 -  address $014502
014502  33EEFFF2000171A8          MOVE.W    -14(A6),$000171A8    L06E9
01450A  61000BE4                  BSR       3044(PC)             FN_READ_OPNAME
01450E  13C70001719C              MOVE.B    D7,$0001719C         L06E2
014514  61000BDA                  BSR       3034(PC)             FN_READ_OPNAME
014518  13C7000171A4              MOVE.B    D7,$000171A4         L06E7
01451E  61000BD0                  BSR       3024(PC)             FN_READ_OPNAME
014522  33C7000171A0              MOVE.W    D7,$000171A0         L06E5
014528  61000BC6                  BSR       3014(PC)             FN_READ_OPNAME
01452C  33C70001719E              MOVE.W    D7,$0001719E         L06E4
014532  66000004                  BNE       4(PC)                __L04FD
014536  4E75                      RTS                           
014538  4279000171A2        __L04FD:
                                  CLR.W     $000171A2            L06E6
01453E  4280                      CLR.L     D0                  
014540  1039000171A4              MOVE.B    $000171A4,D0         L06E7
014546  E148                      LSL.W     #8,D0               
014548  80F90001719E              DIVU      $0001719E,D0         L06E4
01454E  66000006                  BNE       6(PC)                __L04FE
014552  303C0001                  MOVE.W    #1,D0               
014556  4440                __L04FE:
                                  NEG.W     D0                  
014558  33C0000171A6              MOVE.W    D0,$000171A6         L06E8
01455E  13FC00010001719D          MOVE.B    #1,$0001719D         L06E3
014566  4EB90001456E              JSR       $0001456E            L04FF
01456C  4E75                      RTS                           
01456E  48E7FFFE            L04FF:MOVEM.L   A0-A6/D0-D7,-(A7)   
014572  3239000171A8              MOVE.W    $000171A8,D1         L06E9
014578  10390001719C              MOVE.B    $0001719C,D0         L06E2
01457E  47F900017D04              LEA       $00017D04,A3         L0724
014584  B22B000E                  CMP.B     14(A3),D1           
014588  6600000A                  BNE       10(PC)               __L0500
01458C  B02B0001                  CMP.B     1(A3),D0            
014590  670000B0                  BEQ       176(PC)              L0507
014594  47F900017D14        __L0500:
                                  LEA       $00017D14,A3         L0726
01459A  B26B000E                  CMP.W     14(A3),D1           
01459E  6600000A                  BNE       10(PC)               __L0501
0145A2  B02B0001                  CMP.B     1(A3),D0            
0145A6  6700009A                  BEQ       154(PC)              L0507
0145AA  47F900017D24        __L0501:
                                  LEA       $00017D24,A3         L0728
0145B0  B26B000E                  CMP.W     14(A3),D1           
0145B4  6600000A                  BNE       10(PC)               __L0502
0145B8  B02B0001                  CMP.B     1(A3),D0            
0145BC  67000084                  BEQ       132(PC)              L0507
0145C0  4A390001707C        __L0502:
                                  TST.B     $0001707C            B_IS_AMIGA
0145C6  67000018                  BEQ       24(PC)               L0503
0145CA  47F900017D34              LEA       $00017D34,A3         L072A
0145D0  B26B000E                  CMP.W     14(A3),D1           
0145D4  6600000A                  BNE       10(PC)               L0503
0145D8  B02B0001                  CMP.B     1(A3),D0            
0145DC  67000064                  BEQ       100(PC)              L0507
0145E0  47F900017D04        L0503:LEA       $00017D04,A3         L0724
0145E6  103900017D05              MOVE.B    $00017D05,D0         L0725
0145EC  B03900017D15              CMP.B     $00017D15,D0         L0727
0145F2  6F00000E                  BLE       14(PC)               __L0504
0145F6  47F900017D14              LEA       $00017D14,A3         L0726
0145FC  103900017D15              MOVE.B    $00017D15,D0         L0727
014602  B03900017D25        __L0504:
                                  CMP.B     $00017D25,D0         L0729
014608  6F00000E                  BLE       14(PC)               __L0505
01460C  47F900017D24              LEA       $00017D24,A3         L0728
014612  103900017D25              MOVE.B    $00017D25,D0         L0729
014618  4A390001707C        __L0505:
                                  TST.B     $0001707C            B_IS_AMIGA
01461E  67000018                  BEQ       24(PC)               L0506
014622  B03900017D35              CMP.B     $00017D35,D0         L072B
014628  6F00000E                  BLE       14(PC)               L0506
01462C  47F900017D34              LEA       $00017D34,A3         L072A
014632  103900017D35              MOVE.B    $00017D35,D0         L072B
014638  B0390001719C        L0506:CMP.B     $0001719C,D0         L06E2
01463E  6E00008C                  BGT       140(PC)              L050A
014642  10390001719C        L0507:MOVE.B    $0001719C,D0         L06E2
014648  4A390001707C              TST.B     $0001707C            B_IS_AMIGA
01464E  6600001C                  BNE       28(PC)               L0508
014652  4A39000171DA              TST.B     $000171DA            L06F5
014658  67000012                  BEQ       18(PC)               L0508
01465C  B039000171DB              CMP.B     $000171DB,D0         L06F6
014662  6D000008                  BLT       8(PC)                L0508
014666  4EB90000F0C4              JSR       $0000F0C4            L0271
01466C  4A390001719D        L0508:TST.B     $0001719D            L06E3
014672  6A00000C                  BPL       12(PC)               __L0509
014676  4EB90000F032              JSR       $0000F032            L026C
01467C  6000004E                  BRA       78(PC)               L050A
014680  177C00800000        __L0509:
                                  MOVE.B    #$80,0(A3)          
014686  17400001                  MOVE.B    D0,1(A3)            
01468A  17790001719D0002          MOVE.B    $0001719D,2(A3)      L06E3
014692  3779000171A0000A          MOVE.W    $000171A0,10(A3)     L06E5
01469A  3779000171A2000C          MOVE.W    $000171A2,12(A3)     L06E6
0146A2  1779000171A40006          MOVE.B    $000171A4,6(A3)      L06E7
0146AA  422B0007                  CLR.B     7(A3)               
0146AE  3779000171A60008          MOVE.W    $000171A6,8(A3)      L06E8
0146B6  37790001719E0004          MOVE.W    $0001719E,4(A3)      L06E4
0146BE  3779000171A8000E          MOVE.W    $000171A8,14(A3)     L06E9
0146C6  177C00020000              MOVE.B    #2,0(A3)            
0146CC  4CDF7FFF            L050A:MOVEM.L   (A7)+,A0-A6/D0-D7   
0146D0  4E75                      RTS                           


; =============================================================================
; OPCODE 0x98 -  offset 0x39b0 -  address $0146d2
0146D2  61000A1C                  BSR       2588(PC)             FN_READ_OPNAME
0146D6  13C7000171AE              MOVE.B    D7,$000171AE         L06EC
0146DC  13C7000171B0              MOVE.B    D7,$000171B0         L06ED
0146E2  3039000171B0              MOVE.W    $000171B0,D0         L06ED
0146E8  48C0                      EXT.L     D0                  
0146EA  3239000171BC              MOVE.W    $000171BC,D1         L06EF
0146F0  6700000A                  BEQ       10(PC)               __L050B
0146F4  80C1                      DIVU      D1,D0               
0146F6  33C0000171C2              MOVE.W    D0,$000171C2         L06F2
0146FC  3039000171B0        __L050B:
                                  MOVE.W    $000171B0,D0         L06ED
014702  48C0                      EXT.L     D0                  
014704  3239000171BE              MOVE.W    $000171BE,D1         L06F0
01470A  6700000A                  BEQ       10(PC)               __L050C
01470E  80C1                      DIVU      D1,D0               
014710  33C0000171C4              MOVE.W    D0,$000171C4         L06F3
014716  4E75                __L050C:
                                  RTS                           


; =============================================================================
; OPCODE 0x97 -  offset 0x39f6 -  address $014718
014718  610009D6                  BSR       2518(PC)             FN_READ_OPNAME
01471C  4A47                      TST.W     D7                  
01471E  66000006                  BNE       6(PC)                __L050D
014722  3E3C0001                  MOVE.W    #1,D7               
014726  13C7000171AB        __L050D:
                                  MOVE.B    D7,$000171AB         L06EB
01472C  4E75                      RTS                           
01472E  4A3900017103        L050E:TST.B     $00017103            L06A4
014734  66000028                  BNE       40(PC)               __L0510
014738  206EFFEC            __L050F:
                                  MOVEA.L   -20(A6),A0          
01473C  D1E8000E                  ADDA.L    14(A0),A0           
014740  D1E8000C                  ADDA.L    12(A0),A0           
014744  DE47                      ADD.W     D7,D7               
014746  DE47                      ADD.W     D7,D7               
014748  41F07000                  LEA       0(A0,D7.W),A0       
01474C  D1D0                      ADDA.L    (A0),A0             
01474E  2008                      MOVE.L    A0,D0               
014750  4E75                      RTS                           
014752  203C00000011              MOVE.L    #$11,D0             
014758  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
01475E  2F0E                __L0510:
                                  MOVE.L    A6,-(A7)            
014760  2C79000170E0              MOVEA.L   $000170E0,A6         L_L0692
014766  6100FFD0                  BSR       -48(PC)              __L050F
01476A  2C5F                      MOVEA.L   (A7)+,A6            
01476C  4E75                      RTS                           


; =============================================================================
; OPCODE 0xce -  offset 0x3a4c -  address $01476e
01476E  13FC000100017103          MOVE.B    #1,$00017103         L06A4
014776  60000008                  BRA       8(PC)                __L0511


; =============================================================================
; OPCODE 0xcd -  offset 0x3a58 -  address $01477a
01477A  423900017103              CLR.B     $00017103            L06A4
014780  6100096E            __L0511:
                                  BSR       2414(PC)             FN_READ_OPNAME
014784  48E70100                  MOVEM.L   D7,-(A7)            
014788  61000966                  BSR       2406(PC)             FN_READ_OPNAME
01478C  48E70100                  MOVEM.L   D7,-(A7)            
014790  6100095E                  BSR       2398(PC)             FN_READ_OPNAME
014794  3407                      MOVE.W    D7,D2               
014796  4CDF0080                  MOVEM.L   (A7)+,D7            
01479A  4CDF0040                  MOVEM.L   (A7)+,D6            
01479E  4A47                      TST.W     D7                  
0147A0  6A00000A                  BPL       10(PC)               __L0512
0147A4  4282                      CLR.L     D2                  
0147A6  4280                      CLR.L     D0                  
0147A8  60000022                  BRA       34(PC)               __L0514
0147AC  6100FF80            __L0512:
                                  BSR       -128(PC)             L050E
0147B0  1210                      MOVE.B    (A0),D1             
0147B2  B23C0001                  CMP.B     #1,D1               
0147B6  6700000A                  BEQ       10(PC)               __L0513
0147BA  B23C0002                  CMP.B     #2,D1               
0147BE  66000020                  BNE       32(PC)               __L0515
0147C2  32280002            __L0513:
                                  MOVE.W    2(A0),D1            
0147C6  068000000010              ADDI.L    #$10,D0             
0147CC  0246001F            __L0514:
                                  ANDI.W    #$1F,D6             
0147D0  E74E                      LSL.W     #3,D6               
0147D2  41F9000147E2              LEA       $000147E2,A0         __L0516
0147D8  21806000                  MOVE.L    D0,0(A0,D6.W)       
0147DC  31826004                  MOVE.W    D2,4(A0,D6.W)       
0147E0  4E75                __L0515:
                                  RTS                           
0147E2                      __L0516:
                                  DCB.W     128,0 


; =============================================================================
; OPCODE 0xac -  offset 0x3bc0 -  address $0148e2
0148E2  13FC0001                  DC.B      $13,$FC,$00,$01 
0148E6  00017103                  DC.L      $00017103            L06A4
0148EA  60000008                  BRA       8(PC)                __L0517


; =============================================================================
; OPCODE 0x95 -  offset 0x3bcc -  address $0148ee
0148EE  423900017103              CLR.B     $00017103            L06A4
0148F4  610007FA            __L0517:
                                  BSR       2042(PC)             FN_READ_OPNAME
0148F8  6100FE34                  BSR       -460(PC)             L050E
0148FC  1210                      MOVE.B    (A0),D1             
0148FE  6700000E                  BEQ       14(PC)               L0518
014902  B23C0003                  CMP.B     #3,D1               
014906  67000006                  BEQ       6(PC)                L0518
01490A  600000C2                  BRA       194(PC)              __L051D
01490E  068000000006        L0518:ADDI.L    #6,D0               
014914  23C0000171B8              MOVE.L    D0,$000171B8         L06EE
01491A  610007D4                  BSR       2004(PC)             FN_READ_OPNAME
01491E  4279000171B0              CLR.W     $000171B0            L06ED
014924  4279000171AE              CLR.W     $000171AE            L06EC
01492A  13C7000171B0              MOVE.B    D7,$000171B0         L06ED
014930  610007BE                  BSR       1982(PC)             FN_READ_OPNAME
014934  13C7000171AB              MOVE.B    D7,$000171AB         L06EB
01493A  610007B4                  BSR       1972(PC)             FN_READ_OPNAME
01493E  06470001                  ADDI.W    #1,D7               
014942  33C7000171BC              MOVE.W    D7,$000171BC         L06EF
014948  610007A6                  BSR       1958(PC)             FN_READ_OPNAME
01494C  33C7000171C0              MOVE.W    D7,$000171C0         L06F1
014952  6100079C                  BSR       1948(PC)             FN_READ_OPNAME
014956  06470001                  ADDI.W    #1,D7               
01495A  33C7000171BE              MOVE.W    D7,$000171BE         L06F0
014960  3039000171B0              MOVE.W    $000171B0,D0         L06ED
014966  48C0                      EXT.L     D0                  
014968  3239000171BC              MOVE.W    $000171BC,D1         L06EF
01496E  6700000A                  BEQ       10(PC)               __L0519
014972  80C1                      DIVU      D1,D0               
014974  33C0000171C2              MOVE.W    D0,$000171C2         L06F2
01497A  3039000171B0        __L0519:
                                  MOVE.W    $000171B0,D0         L06ED
014980  48C0                      EXT.L     D0                  
014982  3239000171BE              MOVE.W    $000171BE,D1         L06F0
014988  6700000A                  BEQ       10(PC)               __L051A
01498C  80C1                      DIVU      D1,D0               
01498E  33C0000171C4              MOVE.W    D0,$000171C4         L06F3
014994  41F9000147E2        __L051A:
                                  LEA       $000147E2,A0         L0516
01499A  2279000171B8              MOVEA.L   $000171B8,A1         L06EE
0149A0  1039000171AB              MOVE.B    $000171AB,D0         L06EB
0149A6  1239000171AE              MOVE.B    $000171AE,D1         L06EC
0149AC  4A29FFFA                  TST.B     -6(A1)              
0149B0  6600000C                  BNE       12(PC)               __L051B
0149B4  4EB90000F0DC              JSR       $0000F0DC            __L0272
0149BA  60000008                  BRA       8(PC)                __L051C
0149BE  4EB90000F154        __L051B:
                                  JSR       $0000F154            L0277
0149C4  13FC0001000171AA    __L051C:
                                  MOVE.B    #1,$000171AA         L06EA
0149CC  4E75                      RTS                           
0149CE  61000720            __L051D:
                                  BSR       1824(PC)             FN_READ_OPNAME
0149D2  6100071C                  BSR       1820(PC)             FN_READ_OPNAME
0149D6  61000718                  BSR       1816(PC)             FN_READ_OPNAME
0149DA  61000714                  BSR       1812(PC)             FN_READ_OPNAME
0149DE  61000710                  BSR       1808(PC)             FN_READ_OPNAME
0149E2  4E75                      RTS                           


; =============================================================================
; OPCODE 0x96 -  offset 0x3cc2 -  address $0149e4
0149E4  6100070A                  BSR       1802(PC)             FN_READ_OPNAME
0149E8  06470010                  ADDI.W    #$10,D7             
0149EC  33C7000171BE              MOVE.W    D7,$000171BE         L06F0
0149F2  4279000171BC              CLR.W     $000171BC            L06EF
0149F8  33FC0001000171C0          MOVE.W    #1,$000171C0         L06F1
014A00  3039000171AE              MOVE.W    $000171AE,D0         L06EC
014A06  48C0                      EXT.L     D0                  
014A08  3239000171BC              MOVE.W    $000171BC,D1         L06EF
014A0E  6700000A                  BEQ       10(PC)               __L051E
014A12  80C1                      DIVU      D1,D0               
014A14  33C0000171C2              MOVE.W    D0,$000171C2         L06F2
014A1A  3039000171AE        __L051E:
                                  MOVE.W    $000171AE,D0         L06EC
014A20  48C0                      EXT.L     D0                  
014A22  3239000171BE              MOVE.W    $000171BE,D1         L06F0
014A28  6700000A                  BEQ       10(PC)               __L051F
014A2C  80C1                      DIVU      D1,D0               
014A2E  33C0000171C4              MOVE.W    D0,$000171C4         L06F3
014A34  4E75                __L051F:
                                  RTS                           


; =============================================================================
; OPCODE 0xd5 -  offset 0x3d14 -  address $014a36
014A36  610006B8                  BSR       1720(PC)             FN_READ_OPNAME
014A3A  33C7000171D8              MOVE.W    D7,$000171D8         L06F4
014A40  4E75                      RTS                           


; =============================================================================
; OPCODE 0x9d -  offset 0x3d20 -  address $014a42
014A42  423900017103              CLR.B     $00017103            L06A4
014A48  610006A6            __L0520:
                                  BSR       1702(PC)             FN_READ_OPNAME
014A4C  48E70100                  MOVEM.L   D7,-(A7)            
014A50  6100069E                  BSR       1694(PC)             FN_READ_OPNAME
014A54  13C70001719C              MOVE.B    D7,$0001719C         L06E2
014A5A  61000694                  BSR       1684(PC)             FN_READ_OPNAME
014A5E  13C7000171A4              MOVE.B    D7,$000171A4         L06E7
014A64  6100068A                  BSR       1674(PC)             FN_READ_OPNAME
014A68  33C7000171DE              MOVE.W    D7,$000171DE         L06F8
014A6E  61000680                  BSR       1664(PC)             FN_READ_OPNAME
014A72  13C7000171DD              MOVE.B    D7,$000171DD         L06F7
014A78  4CDF0080                  MOVEM.L   (A7)+,D7            
014A7C  6100FCB0                  BSR       -848(PC)             L050E
014A80  1210                      MOVE.B    (A0),D1             
014A82  B23C0001                  CMP.B     #1,D1               
014A86  6700000A                  BEQ       10(PC)               __L0521
014A8A  B23C0002                  CMP.B     #2,D1               
014A8E  66000050                  BNE       80(PC)               L0524
014A92  4A39000171DD        __L0521:
                                  TST.B     $000171DD            L06F7
014A98  6600000A                  BNE       10(PC)               __L0522
014A9C  13E80001000171DD          MOVE.B    1(A0),$000171DD      L06F7
014AA4  22280002            __L0522:
                                  MOVE.L    2(A0),D1            
014AA8  048100000010              SUBI.L    #$10,D1             
014AAE  23C1000171E4              MOVE.L    D1,$000171E4         L06FA
014AB4  43E80010                  LEA       16(A0),A1           
014AB8  23C9000171E0              MOVE.L    A1,$000171E0         L06F9
014ABE  4A390001707C              TST.B     $0001707C            B_IS_AMIGA
014AC4  6600000C                  BNE       12(PC)               __L0523
014AC8  4EB90000F032              JSR       $0000F032            L026C
014ACE  60000010                  BRA       16(PC)               L0524
014AD2  13FC00800001719D    __L0523:
                                  MOVE.B    #$80,$0001719D       L06E3
014ADA  4EB90001456E              JSR       $0001456E            L04FF
014AE0  4E75                L0524:RTS                           


; =============================================================================
; OPCODE 0x9e -  offset 0x3dc0 -  address $014ae2
014AE2  13FC000100017103          MOVE.B    #1,$00017103         L06A4
014AEA  6000FF5C                  BRA       -164(PC)             __L0520


; =============================================================================
; OPCODE 0xa1 -  offset 0x3dcc -  address $014aee
014AEE  4EB90000F0C4              JSR       $0000F0C4            L0271
014AF4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x70 -  offset 0x3dd4 -  address $014af6
014AF6  204B                      MOVEA.L   A3,A0               
014AF8  0C1300FF                  CMPI.B    #$FF,(A3)           
014AFC  67000012                  BEQ       18(PC)               __L0526
014B00  4A1B                __L0525:
                                  TST.B     (A3)+               
014B02  6600FFFC                  BNE       -4(PC)               __L0525
014B06  101B                      MOVE.B    (A3)+,D0            
014B08  E140                      ASL.W     #8,D0               
014B0A  101B                      MOVE.B    (A3)+,D0            
014B0C  60002044                  BRA       8260(PC)             L0629
014B10  528B                __L0526:
                                  ADDQ.L    #1,A3               
014B12  610005E8                  BSR       1512(PC)             L054B
014B16  610005D8                  BSR       1496(PC)             FN_READ_OPNAME
014B1A  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
014B20  3007                      MOVE.W    D7,D0               
014B22  6000202E                  BRA       8238(PC)             L0629


; =============================================================================
; OPCODE 0x71 -  offset 0x3e04 -  address $014b26
014B26  600020D2                  BRA       8402(PC)             FN_DISK_CLOSE_FILE


; =============================================================================
; OPCODE 0x72 -  offset 0x3e08 -  address $014b2a
014B2A  204B                      MOVEA.L   A3,A0               
014B2C  4A1B                __L0527:
                                  TST.B     (A3)+               
014B2E  6600FFFC                  BNE       -4(PC)               __L0527
014B32  101B                      MOVE.B    (A3)+,D0            
014B34  E140                      ASL.W     #8,D0               
014B36  101B                      MOVE.B    (A3)+,D0            
014B38  610020EC                  BSR       8428(PC)             FN_DISK_CREATE_FILE
014B3C  610020BC                  BSR       8380(PC)             FN_DISK_CLOSE_FILE
014B40  4E75                      RTS                           


; =============================================================================
; OPCODE 0x73 -  offset 0x3e20 -  address $014b42
014B42  204B                      MOVEA.L   A3,A0               
014B44  4A1B                __L0528:
                                  TST.B     (A3)+               
014B46  6600FFFC                  BNE       -4(PC)               __L0528
014B4A  60002132                  BRA       8498(PC)             L0636


; =============================================================================
; OPCODE 0x74 -  offset 0x3e2c -  address $014b4e
014B4E  43F900017484              LEA       $00017484,A1         L071C
014B54  7202                      MOVEQ     #2,D1               
014B56  61002144                  BSR       8516(PC)             FN_DISK_READ_BYTES
014B5A  3E3900017484              MOVE.W    $00017484,D7         L071C
014B60  6000C8E8                  BRA       -14104(PC)           L035D


; =============================================================================
; OPCODE 0x75 -  offset 0x3e42 -  address $014b64
014B64  6100058A                  BSR       1418(PC)             FN_READ_OPNAME
014B68  33C700017484              MOVE.W    D7,$00017484         L071C
014B6E  43F900017484              LEA       $00017484,A1         L071C
014B74  7202                      MOVEQ     #2,D1               
014B76  61002150                  BSR       8528(PC)             FN_DISK_WRITE_BYTES
014B7A  4E75                      RTS                           


; =============================================================================
; OPCODE 0x76 -  offset 0x3e5a -  address $014b7c
014B7C  7200                      MOVEQ     #0,D1               
014B7E  224B                      MOVEA.L   A3,A1               
014B80  5281                __L0529:
                                  ADDQ.L    #1,D1               
014B82  4A1B                      TST.B     (A3)+               
014B84  6600FFFA                  BNE       -6(PC)               __L0529
014B88  6100213E                  BSR       8510(PC)             FN_DISK_WRITE_BYTES
014B8C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x77 -  offset 0x3e6c -  address $014b8e
014B8E  101B                      MOVE.B    (A3)+,D0            
014B90  E140                      ASL.W     #8,D0               
014B92  101B                      MOVE.B    (A3)+,D0            
014B94  7200                      MOVEQ     #0,D1               
014B96  121B                      MOVE.B    (A3)+,D1            
014B98  E141                      ASL.W     #8,D1               
014B9A  121B                      MOVE.B    (A3)+,D1            
014B9C  43F60000                  LEA       0(A6,D0.W),A1       
014BA0  610020FA                  BSR       8442(PC)             FN_DISK_READ_BYTES
014BA4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x78 -  offset 0x3e84 -  address $014ba6
014BA6  101B                      MOVE.B    (A3)+,D0            
014BA8  E140                      ASL.W     #8,D0               
014BAA  101B                      MOVE.B    (A3)+,D0            
014BAC  7200                      MOVEQ     #0,D1               
014BAE  121B                      MOVE.B    (A3)+,D1            
014BB0  E141                      ASL.W     #8,D1               
014BB2  121B                      MOVE.B    (A3)+,D1            
014BB4  43F60000                  LEA       0(A6,D0.W),A1       
014BB8  6100210E                  BSR       8462(PC)             FN_DISK_WRITE_BYTES
014BBC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x79 -  offset 0x3e9c -  address $014bbe
014BBE  61000530                  BSR       1328(PC)             FN_READ_OPNAME
014BC2  6100051E                  BSR       1310(PC)             L0548
014BC6  3007                      MOVE.W    D7,D0               
014BC8  3206                      MOVE.W    D6,D1               
014BCA  33C70001706A              MOVE.W    D7,$0001706A         L065E
014BD0  33C60001706C              MOVE.W    D6,$0001706C         L065F
014BD6  4EB90000BCA0              JSR       $0000BCA0            __L0099
014BDC  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7a -  offset 0x3ebc -  address $014bde
014BDE  61000510                  BSR       1296(PC)             FN_READ_OPNAME
014BE2  610004FE                  BSR       1278(PC)             L0548
014BE6  30390001706A              MOVE.W    $0001706A,D0         L065E
014BEC  32390001706C              MOVE.W    $0001706C,D1         L065F
014BF2  3400                      MOVE.W    D0,D2               
014BF4  3601                      MOVE.W    D1,D3               
014BF6  D447                      ADD.W     D7,D2               
014BF8  D646                      ADD.W     D6,D3               
014BFA  33C20001706A              MOVE.W    D2,$0001706A         L065E
014C00  33C30001706C              MOVE.W    D3,$0001706C         L065F
014C06  4EB90000BD1A              JSR       $0000BD1A            L009A
014C0C  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7b -  offset 0x3eec -  address $014c0e
014C0E  610004E0                  BSR       1248(PC)             FN_READ_OPNAME
014C12  610004CE                  BSR       1230(PC)             L0548
014C16  30390001706A              MOVE.W    $0001706A,D0         L065E
014C1C  32390001706C              MOVE.W    $0001706C,D1         L065F
014C22  3400                      MOVE.W    D0,D2               
014C24  3601                      MOVE.W    D1,D3               
014C26  D447                      ADD.W     D7,D2               
014C28  4EB90000BD1A              JSR       $0000BD1A            L009A
014C2E  3002                      MOVE.W    D2,D0               
014C30  D646                      ADD.W     D6,D3               
014C32  4EB90000BD1A              JSR       $0000BD1A            L009A
014C38  3203                      MOVE.W    D3,D1               
014C3A  34390001706A              MOVE.W    $0001706A,D2         L065E
014C40  4EB90000BD1A              JSR       $0000BD1A            L009A
014C46  3002                      MOVE.W    D2,D0               
014C48  32390001706C              MOVE.W    $0001706C,D1         L065F
014C4E  4EB90000BD1A              JSR       $0000BD1A            L009A
014C54  DF790001706A              ADD.W     D7,$0001706A         L065E
014C5A  DD790001706C              ADD.W     D6,$0001706C         L065F
014C60  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7c -  offset 0x3f40 -  address $014c62
014C62  6100048C                  BSR       1164(PC)             FN_READ_OPNAME
014C66  6100047A                  BSR       1146(PC)             L0548
014C6A  30390001706A              MOVE.W    $0001706A,D0         L065E
014C70  32390001706C              MOVE.W    $0001706C,D1         L065F
014C76  3400                      MOVE.W    D0,D2               
014C78  3601                      MOVE.W    D1,D3               
014C7A  D447                      ADD.W     D7,D2               
014C7C  D646                      ADD.W     D6,D3               
014C7E  33C20001706A              MOVE.W    D2,$0001706A         L065E
014C84  33C30001706C              MOVE.W    D3,$0001706C         L065F
014C8A  4EB90000BDC2              JSR       $0000BDC2            __L009B
014C90  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7d -  offset 0x3f70 -  address $014c92
014C92  6100045C                  BSR       1116(PC)             FN_READ_OPNAME
014C96  13C70001706E              MOVE.B    D7,$0001706E         L0660
014C9C  3007                      MOVE.W    D7,D0               
014C9E  4EB90000BE62              JSR       $0000BE62            L009E
014CA4  4E75                      RTS                           


; =============================================================================
; OPCODE 0xbb -  offset 0x3f84 -  address $014ca6
014CA6  61000448                  BSR       1096(PC)             FN_READ_OPNAME
014CAA  13C70001706F              MOVE.B    D7,$0001706F         L0661
014CB0  3007                      MOVE.W    D7,D0               
014CB2  4EB90000BEF2              JSR       $0000BEF2            __L00A2
014CB8  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7e -  offset 0x3f98 -  address $014cba
014CBA  61000434                  BSR       1076(PC)             FN_READ_OPNAME
014CBE  33C70001706A              MOVE.W    D7,$0001706A         L065E
014CC4  6100042A                  BSR       1066(PC)             FN_READ_OPNAME
014CC8  33C70001706C              MOVE.W    D7,$0001706C         L065F
014CCE  4E75                      RTS                           


; =============================================================================
; OPCODE 0x7f -  offset 0x3fae -  address $014cd0
014CD0  6100041E                  BSR       1054(PC)             FN_READ_OPNAME
014CD4  DF790001706A              ADD.W     D7,$0001706A         L065E
014CDA  61000414                  BSR       1044(PC)             FN_READ_OPNAME
014CDE  DF790001706C              ADD.W     D7,$0001706C         L065F
014CE4  4E75                      RTS                           


; =============================================================================
; OPCODE 0x80 -  offset 0x3fc4 -  address $014ce6
014CE6  61000408                  BSR       1032(PC)             FN_READ_OPNAME
014CEA  13C700017070              MOVE.B    D7,$00017070         L0662
014CF0  3007                      MOVE.W    D7,D0               
014CF2  4EB90000BF12              JSR       $0000BF12            __L00A3
014CF8  4E75                      RTS                           


; =============================================================================
; OPCODE 0x81 -  offset 0x3fd8 -  address $014cfa
014CFA  423900017103              CLR.B     $00017103            L06A4
014D00  610003EE                  BSR       1006(PC)             FN_READ_OPNAME
014D04  13EEFFFD00017100          MOVE.B    -3(A6),$00017100     L06A1
014D0C  42B9000170F2        __L052A:
                                  CLR.L     $000170F2            L0699
014D12  23F90001708200017108      MOVE.L    $00017082,$00017108  L066E L06A8
014D1C  23F9000170860001710C      MOVE.L    $00017086,$0001710C  L066F L06AA
014D26  23F90001708200017124      MOVE.L    $00017082,$00017124  L066E L06B5
014D30  23F90001708600017128      MOVE.L    $00017086,$00017128  L066F L06B7
014D3A  30390001710C              MOVE.W    $0001710C,D0         L06AA
014D40  907900017108              SUB.W     $00017108,D0         L06A8
014D46  5240                      ADDQ.W    #1,D0               
014D48  E448                      LSR.W     #2,D0               
014D4A  33C000017110              MOVE.W    D0,$00017110         L06AC
014D50  6100D774            __L052B:
                                  BSR       -10380(PC)           L03D8
014D54  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
014D5A  3239000170CA              MOVE.W    $000170CA,D1         L0685
014D60  1010                      MOVE.B    (A0),D0             
014D62  6A000098                  BPL       152(PC)              __L0530
014D66  B03C00FE                  CMP.B     #$FE,D0             
014D6A  6700F1B2                  BEQ       -3662(PC)            L04C8
014D6E  4243                      CLR.W     D3                  
014D70  16280001                  MOVE.B    1(A0),D3            
014D74  5488                      ADDQ.L    #2,A0               
014D76  6000007E                  BRA       126(PC)              __L052F
014D7A  3F3900017100        L052C:MOVE.W    $00017100,-(A7)      L06A1
014D80  3F39000170F2              MOVE.W    $000170F2,-(A7)      L0699
014D86  3F39000170F4              MOVE.W    $000170F4,-(A7)      L069A
014D8C  3F39000170F6              MOVE.W    $000170F6,-(A7)      L069B
014D92  13FC000100017101          MOVE.B    #1,$00017101         L06A2
014D9A  3E18                      MOVE.W    (A0)+,D7            
014D9C  3018                      MOVE.W    (A0)+,D0            
014D9E  4A3900017100              TST.B     $00017100            L06A1
014DA4  67000004                  BEQ       4(PC)                __L052D
014DA8  4440                      NEG.W     D0                  
014DAA  D179000170F2        __L052D:
                                  ADD.W     D0,$000170F2         L0699
014DB0  3018                      MOVE.W    (A0)+,D0            
014DB2  D179000170F4              ADD.W     D0,$000170F4         L069A
014DB8  3018                      MOVE.W    (A0)+,D0            
014DBA  D179000170F6              ADD.W     D0,$000170F6         L069B
014DC0  4A47                      TST.W     D7                  
014DC2  6A00000E                  BPL       14(PC)               __L052E
014DC6  02477FFF                  ANDI.W    #$7FFF,D7           
014DCA  0879000000017100          BCHG      #0,$00017100         L06A1
014DD2  3F03                __L052E:
                                  MOVE.W    D3,-(A7)            
014DD4  2F08                      MOVE.L    A0,-(A7)            
014DD6  6100FF78                  BSR       -136(PC)             __L052B
014DDA  205F                      MOVEA.L   (A7)+,A0            
014DDC  361F                      MOVE.W    (A7)+,D3            
014DDE  33DF000170F6              MOVE.W    (A7)+,$000170F6      L069B
014DE4  33DF000170F4              MOVE.W    (A7)+,$000170F4      L069A
014DEA  33DF000170F2              MOVE.W    (A7)+,$000170F2      L0699
014DF0  33DF00017100              MOVE.W    (A7)+,$00017100      L06A1
014DF6  51CBFF82            __L052F:
                                  DBF       D3,-126(PC)          L052C
014DFA  4E75                      RTS                           
014DFC  23821008            __L0530:
                                  MOVE.L    D2,8(A1,D1.W)       
014E00  23B90001706A100C          MOVE.L    $0001706A,12(A1,D1.W)  L065E
014E08  3039000170F2              MOVE.W    $000170F2,D0         L0699
014E0E  D171100C                  ADD.W     D0,12(A1,D1.W)      
014E12  3039000170F4              MOVE.W    $000170F4,D0         L069A
014E18  D171100E                  ADD.W     D0,14(A1,D1.W)      
014E1C  42711010                  CLR.W     16(A1,D1.W)         
014E20  0C100003                  CMPI.B    #3,(A0)             
014E24  66000022                  BNE       34(PC)               __L0531
014E28  30280004                  MOVE.W    4(A0),D0            
014E2C  D171100C                  ADD.W     D0,12(A1,D1.W)      
014E30  30280006                  MOVE.W    6(A0),D0            
014E34  D171100E                  ADD.W     D0,14(A1,D1.W)      
014E38  30280002                  MOVE.W    2(A0),D0            
014E3C  E540                      ASL.W     #2,D0               
014E3E  2042                      MOVEA.L   D2,A0               
014E40  D0C0                      ADDA.W    D0,A0               
014E42  23881008                  MOVE.L    A0,8(A1,D1.W)       
014E46  D1D0                      ADDA.L    (A0),A0             
014E48  13B9000171001008    __L0531:
                                  MOVE.B    $00017100,8(A1,D1.W) L06A1
014E50  30280002                  MOVE.W    2(A0),D0            
014E54  E248                      LSR.W     #1,D0               
014E56  9171100C                  SUB.W     D0,12(A1,D1.W)      
014E5A  30280004                  MOVE.W    4(A0),D0            
014E5E  E248                      LSR.W     #1,D0               
014E60  9171100E                  SUB.W     D0,14(A1,D1.W)      
014E64  41F11000                  LEA       0(A1,D1.W),A0       
014E68  4A390001712D              TST.B     $0001712D            L06BA
014E6E  67000010                  BEQ       16(PC)               __L0532
014E72  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
014E76  4EB90000D3C0              JSR       $0000D3C0            L0192
014E7C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
014E80  48E7FFFE            __L0532:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
014E84  4EB90000D3C0              JSR       $0000D3C0            L0192
014E8A  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
014E8E  6100EA08                  BSR       -5624(PC)            __L0483
014E92  4E75                      RTS                           


; =============================================================================
; OPCODE 0x84 -  offset 0x4172 -  address $014e94
014E94  4AB9000170DA              TST.L     $000170DA            L068F
014E9A  6700001C                  BEQ       28(PC)               __op84
014E9E  0C390001000170CF          CMPI.B    #1,$000170CF         L0688
014EA6  6C000010                  BGE       16(PC)               __op84
014EAA  13FC0002000170CF          MOVE.B    #2,$000170CF         L0688
014EB2  4239000170D1              CLR.B     $000170D1            L068A
014EB8  4E75                __op84:
                                  RTS                           


; =============================================================================
; OPCODE 0x85 -  offset 0x4198 -  address $014eba
014EBA  4AB9000170DA              TST.L     $000170DA            L068F
014EC0  67000028                  BEQ       40(PC)               __op85
014EC4  4A39000170CF              TST.B     $000170CF            L0688
014ECA  6B00001E                  BMI       30(PC)               __op85
014ECE  0C390002000170CF          CMPI.B    #2,$000170CF         L0688
014ED6  6700000A                  BEQ       10(PC)               __op85_2
014EDA  4239000170CF              CLR.B     $000170CF            L0688
014EE0  4E75                      RTS                           
014EE2  13FC00FF000170CF    __op85_2:
                                  MOVE.B    #$FF,$000170CF       L0688
014EEA  4E75                __op85:
                                  RTS                           


; =============================================================================
; OPCODE 0x86 -  offset 0x41ca -  address $014eec
; cmouse
014EEC  4EB90000B6F2              JSR       $0000B6F2            L005B
014EF2  48A76000                  MOVEM.W   D1-D2,-(A7)         
014EF6  3E00                      MOVE.W    D0,D7               
014EF8  6100C550                  BSR       -15024(PC)           L035D
014EFC  4C9F0080                  MOVEM.W   (A7)+,D7            
014F00  6100C548                  BSR       -15032(PC)           L035D
014F04  3E1F                      MOVE.W    (A7)+,D7            
014F06  66000008                  BNE       8(PC)                __L0536
014F0A  1E39000170D2              MOVE.B    $000170D2,D7         L068B
014F10  6100C538            __L0536:
                                  BSR       -15048(PC)           L035D
014F14  4239000170D2              CLR.B     $000170D2            L068B
014F1A  4239000170D3              CLR.B     $000170D3            L068C
014F20  4E75                      RTS                           


; =============================================================================
; OPCODE 0x87 -  offset 0x4200 -  address $014f22
014F22  423900017103              CLR.B     $00017103            L06A4
014F28  610001C6                  BSR       454(PC)              FN_READ_OPNAME
014F2C  6100D598                  BSR       -10856(PC)           L03D8
014F30  4A10                      TST.B     (A0)                
014F32  67000004                  BEQ       4(PC)                __L0537
014F36  4E75                      RTS                           
014F38  23C8000170DA        __L0537:
                                  MOVE.L    A0,$000170DA         L068F
014F3E  4E75                      RTS                           


; =============================================================================
; OPCODE 0x88 -  offset 0x421e -  address $014f40
014F40  610001AE                  BSR       430(PC)              FN_READ_OPNAME
014F44  6100019C                  BSR       412(PC)              L0548
014F48  3007                      MOVE.W    D7,D0               
014F4A  3206                      MOVE.W    D6,D1               
014F4C  4EB90000B708              JSR       $0000B708            __L005C
014F52  4E75                      RTS                           


; =============================================================================
; OPCODE 0x89 -  offset 0x4232 -  address $014f54
014F54  4240                      CLR.W     D0                  
014F56  101B                      MOVE.B    (A3)+,D0            
014F58  D000                      ADD.B     D0,D0               
014F5A  D02BFFFF                  ADD.B     -1(A3),D0           
014F5E  06400003                  ADDI.W    #3,D0               
014F62  D6C0                      ADDA.W    D0,A3               
014F64  4E75                      RTS                           


; =============================================================================
; OPCODE 0x8a -  offset 0x4244 -  address $014f66
014F66  101B                      MOVE.B    (A3)+,D0            
014F68  E140                      ASL.W     #8,D0               
014F6A  101B                      MOVE.B    (A3)+,D0            
014F6C  3D40FFE0                  MOVE.W    D0,-32(A6)          
014F70  4E75                      RTS                           


; =============================================================================
; OPCODE 0x91 -  offset 0x4250 -  address $014f72
014F72  101B                      MOVE.B    (A3)+,D0            
014F74  E140                      ASL.W     #8,D0               
014F76  101B                      MOVE.B    (A3)+,D0            
014F78  323C0005                  MOVE.W    #5,D1               
014F7C  1D9B0000            __L0538:
                                  MOVE.B    (A3)+,0(A6,D0.W)    
014F80  51C9FFFA                  DBF       D1,-6(PC)            __L0538
014F84  4E75                      RTS                           


; =============================================================================
; OPCODE 0x92 -  offset 0x4264 -  address $014f86
014F86  1D5BFFDE                  MOVE.B    (A3)+,-34(A6)       
014F8A  1D5BFFDF                  MOVE.B    (A3)+,-33(A6)       
014F8E  4E75                      RTS                           


; =============================================================================
; OPCODE 0x82 -  offset 0x426e -  address $014f90
014F90  6100015E                  BSR       350(PC)              FN_READ_OPNAME
014F94  6100015A                  BSR       346(PC)              FN_READ_OPNAME
014F98  4E75                      RTS                           


; =============================================================================
; OPCODE 0x34 -  offset 0x4278 -  address $014f9a
014F9A  61000154                  BSR       340(PC)              FN_READ_OPNAME
014F9E  41F9000170F8              LEA       $000170F8,A0         L069C
014FA4  31470002                  MOVE.W    D7,2(A0)            
014FA8  4E75                      RTS                           
014FAA  2079000170A8        L0539:MOVEA.L   $000170A8,A0         L067A
014FB0  20707000                  MOVEA.L   0(A0,D7.W),A0       
014FB4  4CA8000E0000              MOVEM.W   0(A0),D1-D3         
014FBA  926E0000                  SUB.W     0(A6),D1            
014FBE  946E0002                  SUB.W     2(A6),D2            
014FC2  966E0004                  SUB.W     4(A6),D3            
014FC6  4A6EFFE0                  TST.W     -32(A6)             
014FCA  67000014                  BEQ       20(PC)               __L053B
014FCE  206EFFEC                  MOVEA.L   -20(A6),A0          
014FD2  D0EEFFE0                  ADDA.W    -32(A6),A0          
014FD6  4247                __L053A:
                                  CLR.W     D7                  
014FD8  1E18                      MOVE.B    (A0)+,D7            
014FDA  1D470008                  MOVE.B    D7,8(A6)            
014FDE  4E75                      RTS                           
014FE0  207C00015014        __L053B:
                                  MOVEA.L   #$00015014,A0        L0540
014FE6  6000FFEE                  BRA       -18(PC)              __L053A
014FEA  1810                L053C:MOVE.B    (A0),D4             
014FEC  4884                      EXT.W     D4                  
014FEE  48C4                      EXT.L     D4                  
014FF0  67000004                  BEQ       4(PC)                __L053D
014FF4  C9C1                      MULS      D1,D4               
014FF6  10280001            __L053D:
                                  MOVE.B    1(A0),D0            
014FFA  67000008                  BEQ       8(PC)                __L053E
014FFE  4880                      EXT.W     D0                  
015000  C1C2                      MULS      D2,D0               
015002  D880                      ADD.L     D0,D4               
015004  10280002            __L053E:
                                  MOVE.B    2(A0),D0            
015008  67000008                  BEQ       8(PC)                __L053F
01500C  4880                      EXT.W     D0                  
01500E  C1C3                      MULS      D3,D0               
015010  D880                      ADD.L     D0,D4               
015012  4E75                __L053F:
                                  RTS                           
015014  19                  L0540:DC.B      $19 
015015  010000010100000100  L0541:DC.B      $01,$00,$00,$01,$01,$00,$00,$01,$00 
01501E  FF0100FF0000FFFF          DC.B      $FF,$01,$00,$FF,$00,$00,$FF,$FF 
015026  0000FF0001FF0001          DC.B      $00,$00,$FF,$00,$01,$FF,$00,$01 
01502E  0001010101000101          DC.B      $00,$01,$01,$01,$01,$00,$01,$01 
015036  FF0101FF0001FFFF          DC.B      $FF,$01,$01,$FF,$00,$01,$FF,$FF 
01503E  0100FF0101FF0100          DC.B      $01,$00,$FF,$01,$01,$FF,$01,$00 
015046  00010100FF0101FF          DC.B      $00,$01,$01,$00,$FF,$01,$01,$FF 
01504E  0001FFFF01FFFF00          DC.B      $00,$01,$FF,$FF,$01,$FF,$FF,$00 
015056  FFFFFFFF00FFFF01          DC.B      $FF,$FF,$FF,$FF,$00,$FF,$FF,$01 
01505E  FFFF0000FF00              DC.B      $FF,$FF,$00,$00,$FF,$00 


; =============================================================================
; OPCODE 0x8c -  offset 0x4342 -  address $015064
015064  6100008A                  BSR       138(PC)              FN_READ_OPNAME
015068  BE7CFFFF                  CMP.W     #$FFFF,D7           
01506C  6700003E                  BEQ       62(PC)               L0545
015070  6100FF38                  BSR       -200(PC)             L0539
015074  48E70E00                  MOVEM.L   D4-D6,-(A7)         
015078  2A3C80000000              MOVE.L    #$80000000,D5       
01507E  6100FF6A            __L0542:
                                  BSR       -150(PC)             L053C
015082  BA84                      CMP.L     D4,D5               
015084  6C000008                  BGE       8(PC)                __L0543
015088  2A04                      MOVE.L    D4,D5               
01508A  1C07                      MOVE.B    D7,D6               
01508C  2248                      MOVEA.L   A0,A1               
01508E  5688                __L0543:
                                  ADDQ.L    #3,A0               
015090  51CFFFEC                  DBF       D7,-20(PC)           __L0542
015094  9D2E0008            __L0544:
                                  SUB.B     D6,8(A6)            
015098  4CDF0070                  MOVEM.L   (A7)+,D4-D6         
01509C  1D510009                  MOVE.B    (A1),9(A6)          
0150A0  1D690001000A              MOVE.B    1(A1),10(A6)        
0150A6  1D690002000B              MOVE.B    2(A1),11(A6)        
0150AC  4E75                L0545:RTS                           


; =============================================================================
; OPCODE 0x8d -  offset 0x438c -  address $0150ae
0150AE  61000040                  BSR       64(PC)               FN_READ_OPNAME
0150B2  BE7CFFFF                  CMP.W     #$FFFF,D7           
0150B6  6700FFF4                  BEQ       -12(PC)              L0545
0150BA  6100FEEE                  BSR       -274(PC)             L0539
0150BE  48E70E00                  MOVEM.L   D4-D6,-(A7)         
0150C2  2A3C7FFFFFFF              MOVE.L    #$7FFFFFFF,D5       
0150C8  6100FF20            __L0546:
                                  BSR       -224(PC)             L053C
0150CC  BA84                      CMP.L     D4,D5               
0150CE  6F000008                  BLE       8(PC)                __L0547
0150D2  2A04                      MOVE.L    D4,D5               
0150D4  1C07                      MOVE.B    D7,D6               
0150D6  2248                      MOVEA.L   A0,A1               
0150D8  5688                __L0547:
                                  ADDQ.L    #3,A0               
0150DA  51CFFFEC                  DBF       D7,-20(PC)           __L0546
0150DE  6000FFB4                  BRA       -76(PC)              __L0544
0150E2  3F07                L0548:MOVE.W    D7,-(A7)            
0150E4  61000008                  BSR       8(PC)                FN_READ_OPNAME_SAVE_D7
0150E8  3C07                      MOVE.W    D7,D6               
0150EA  3E1F                      MOVE.W    (A7)+,D7            
0150EC  4E75                      RTS                           



0150EE  3C07                FN_READ_OPNAME_SAVE_D7:
                                  MOVE.W    D7,D6               
0150F0  4240                FN_READ_OPNAME:
                                  CLR.W     D0                  
0150F2  101B                      MOVE.B    (A3)+,D0            ; read next byte from script
0150F4  30350000                  MOVE.W    0(A5,D0.W),D0       ; OPNAME_JUMP_TABLE + D0 -> D0
0150F8  4EF50000                  JMP       0(A5,D0.W)          ; jump to OPNAME


0150FC  207900017146        L054B:MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015102  23F90001714A00017146      MOVE.L    $0001714A,$00017146  ADDR_CURRENT_PATH ADDR_SCRIPT_NAME
01510C  23C80001714A              MOVE.L    A0,$0001714A         ADDR_CURRENT_PATH
015112  6000FFDC                  BRA       -36(PC)              FN_READ_OPNAME


; =============================================================================
; OPNAME 0x00
; oimmb
015116  1E1B                      MOVE.B    (A3)+,D7            
015118  4887                      EXT.W     D7                  
01511A  4E75                      RTS                           

; =============================================================================
; OPNAME 0x01
; oimmw
01511C  1E1B                      MOVE.B    (A3)+,D7            
01511E  E147                      ASL.W     #8,D7               
015120  1E1B                      MOVE.B    (A3)+,D7            
015122  4E75                      RTS                           

; =============================================================================
; OPNAME 0x02
; oimmp
015124  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01512A  10DB                __L054C:
                                  MOVE.B    (A3)+,(A0)+         
01512C  6600FFFC                  BNE       -4(PC)               __L054C
015130  4E75                      RTS                           


015132  101B                      MOVE.B    (A3)+,D0            
015134  E140                      ASL.W     #8,D0               
015136  101B                      MOVE.B    (A3)+,D0            
015138  1E360000                  MOVE.B    0(A6,D0.W),D7       
01513C  4887                      EXT.W     D7                  
01513E  4E75                      RTS                           

015140  101B                      MOVE.B    (A3)+,D0            
015142  E140                      ASL.W     #8,D0               
015144  101B                      MOVE.B    (A3)+,D0            
015146  3E360000                  MOVE.W    0(A6,D0.W),D7       
01514A  4E75                      RTS                           

01514C  101B                      MOVE.B    (A3)+,D0            
01514E  E140                      ASL.W     #8,D0               
015150  101B                      MOVE.B    (A3)+,D0            
015152  43F60000                  LEA       0(A6,D0.W),A1       
015156  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01515C  10D9                __L054D:
                                  MOVE.B    (A1)+,(A0)+         
01515E  6600FFFC                  BNE       -4(PC)               __L054D
015162  4E75                      RTS                           

015164  101B                      MOVE.B    (A3)+,D0            
015166  E140                      ASL.W     #8,D0               
015168  101B                      MOVE.B    (A3)+,D0            
01516A  6100022E                  BSR       558(PC)              L0555
01516E  1E360000                  MOVE.B    0(A6,D0.W),D7       
015172  4887                      EXT.W     D7                  
015174  4E75                      RTS                           
015176  101B                      MOVE.B    (A3)+,D0            
015178  E140                      ASL.W     #8,D0               
01517A  101B                      MOVE.B    (A3)+,D0            
01517C  61000258                  BSR       600(PC)              L0557
015180  3E360000                  MOVE.W    0(A6,D0.W),D7       
015184  4E75                      RTS                           
015186  101B                      MOVE.B    (A3)+,D0            
015188  E140                      ASL.W     #8,D0               
01518A  101B                      MOVE.B    (A3)+,D0            
01518C  61000268                  BSR       616(PC)              L0559
015190  43F60000                  LEA       0(A6,D0.W),A1       
015194  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01519A  10D9                __L054E:
                                  MOVE.B    (A1)+,(A0)+         
01519C  6600FFFC                  BNE       -4(PC)               __L054E
0151A0  4E75                      RTS                           
0151A2  4240                      CLR.W     D0                  
0151A4  101B                      MOVE.B    (A3)+,D0            
0151A6  1E360000                  MOVE.B    0(A6,D0.W),D7       
0151AA  4887                      EXT.W     D7                  
0151AC  4E75                      RTS                           
0151AE  4240                      CLR.W     D0                  
0151B0  101B                      MOVE.B    (A3)+,D0            
0151B2  3E360000                  MOVE.W    0(A6,D0.W),D7       
0151B6  4E75                      RTS                           
0151B8  4240                      CLR.W     D0                  
0151BA  101B                      MOVE.B    (A3)+,D0            
0151BC  43F60000                  LEA       0(A6,D0.W),A1       
0151C0  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0151C6  10D9                __L054F:
                                  MOVE.B    (A1)+,(A0)+         
0151C8  6600FFFC                  BNE       -4(PC)               __L054F
0151CC  4E75                      RTS                           
0151CE  4240                      CLR.W     D0                  
0151D0  101B                      MOVE.B    (A3)+,D0            
0151D2  610001C6                  BSR       454(PC)              L0555
0151D6  1E360000                  MOVE.B    0(A6,D0.W),D7       
0151DA  4887                      EXT.W     D7                  
0151DC  4E75                      RTS                           
0151DE  4240                      CLR.W     D0                  
0151E0  101B                      MOVE.B    (A3)+,D0            
0151E2  610001F2                  BSR       498(PC)              L0557
0151E6  3E360000                  MOVE.W    0(A6,D0.W),D7       
0151EA  4E75                      RTS                           
0151EC  4240                      CLR.W     D0                  
0151EE  101B                      MOVE.B    (A3)+,D0            
0151F0  61000204                  BSR       516(PC)              L0559
0151F4  43F60000                  LEA       0(A6,D0.W),A1       
0151F8  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0151FE  10D9                __L0550:
                                  MOVE.B    (A1)+,(A0)+         
015200  6600FFFC                  BNE       -4(PC)               __L0550
015204  4E75                      RTS                           
015206  101B                      MOVE.B    (A3)+,D0            
015208  E140                      ASL.W     #8,D0               
01520A  101B                      MOVE.B    (A3)+,D0            
01520C  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015212  1E310000                  MOVE.B    0(A1,D0.W),D7       
015216  4887                      EXT.W     D7                  
015218  4E75                      RTS                           
01521A  101B                      MOVE.B    (A3)+,D0            
01521C  E140                      ASL.W     #8,D0               
01521E  101B                      MOVE.B    (A3)+,D0            
015220  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015226  3E310000                  MOVE.W    0(A1,D0.W),D7       
01522A  4E75                      RTS                           
01522C  101B                      MOVE.B    (A3)+,D0            
01522E  E140                      ASL.W     #8,D0               
015230  101B                      MOVE.B    (A3)+,D0            
015232  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015238  43F10000                  LEA       0(A1,D0.W),A1       
01523C  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015242  10D9                __L0551:
                                  MOVE.B    (A1)+,(A0)+         
015244  6600FFFC                  BNE       -4(PC)               __L0551
015248  4E75                      RTS                           
01524A  101B                      MOVE.B    (A3)+,D0            
01524C  E140                      ASL.W     #8,D0               
01524E  101B                      MOVE.B    (A3)+,D0            
015250  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015256  C34E                      EXG       A6,A1               
015258  61000140                  BSR       320(PC)              L0555
01525C  C34E                      EXG       A6,A1               
01525E  1E310000                  MOVE.B    0(A1,D0.W),D7       
015262  4887                      EXT.W     D7                  
015264  4E75                      RTS                           
015266  101B                      MOVE.B    (A3)+,D0            
015268  E140                      ASL.W     #8,D0               
01526A  101B                      MOVE.B    (A3)+,D0            
01526C  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015272  C34E                      EXG       A6,A1               
015274  61000160                  BSR       352(PC)              L0557
015278  C34E                      EXG       A6,A1               
01527A  3E310000                  MOVE.W    0(A1,D0.W),D7       
01527E  4E75                      RTS                           
015280  101B                      MOVE.B    (A3)+,D0            
015282  E140                      ASL.W     #8,D0               
015284  101B                      MOVE.B    (A3)+,D0            
015286  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
01528C  C34E                      EXG       A6,A1               
01528E  61000166                  BSR       358(PC)              L0559
015292  C34E                      EXG       A6,A1               
015294  43F10000                  LEA       0(A1,D0.W),A1       
015298  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01529E  10D9                __L0552:
                                  MOVE.B    (A1)+,(A0)+         
0152A0  6600FFFC                  BNE       -4(PC)               __L0552
0152A4  4E75                      RTS                           
0152A6  101B                      MOVE.B    (A3)+,D0            
0152A8  E140                      ASL.W     #8,D0               
0152AA  101B                      MOVE.B    (A3)+,D0            
0152AC  30360000                  MOVE.W    0(A6,D0.W),D0       
0152B0  2079000170A8              MOVEA.L   $000170A8,A0         L067A
0152B6  22700000                  MOVEA.L   0(A0,D0.W),A1       
0152BA  101B                      MOVE.B    (A3)+,D0            
0152BC  E140                      ASL.W     #8,D0               
0152BE  101B                      MOVE.B    (A3)+,D0            
0152C0  1E310000                  MOVE.B    0(A1,D0.W),D7       
0152C4  4887                      EXT.W     D7                  
0152C6  4E75                      RTS                           
0152C8  101B                      MOVE.B    (A3)+,D0            
0152CA  E140                      ASL.W     #8,D0               
0152CC  101B                      MOVE.B    (A3)+,D0            
0152CE  30360000                  MOVE.W    0(A6,D0.W),D0       
0152D2  2079000170A8              MOVEA.L   $000170A8,A0         L067A
0152D8  22700000                  MOVEA.L   0(A0,D0.W),A1       
0152DC  101B                      MOVE.B    (A3)+,D0            
0152DE  E140                      ASL.W     #8,D0               
0152E0  101B                      MOVE.B    (A3)+,D0            
0152E2  3E310000                  MOVE.W    0(A1,D0.W),D7       
0152E6  4E75                      RTS                           
0152E8  101B                      MOVE.B    (A3)+,D0            
0152EA  E140                      ASL.W     #8,D0               
0152EC  101B                      MOVE.B    (A3)+,D0            
0152EE  30360000                  MOVE.W    0(A6,D0.W),D0       
0152F2  2079000170A8              MOVEA.L   $000170A8,A0         L067A
0152F8  22700000                  MOVEA.L   0(A0,D0.W),A1       
0152FC  101B                      MOVE.B    (A3)+,D0            
0152FE  E140                      ASL.W     #8,D0               
015300  101B                      MOVE.B    (A3)+,D0            
015302  43F10000                  LEA       0(A1,D0.W),A1       
015306  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01530C  10D9                __L0553:
                                  MOVE.B    (A1)+,(A0)+         
01530E  6600FFFC                  BNE       -4(PC)               __L0553
015312  4E75                      RTS                           
015314  101B                      MOVE.B    (A3)+,D0            
015316  E140                      ASL.W     #8,D0               
015318  101B                      MOVE.B    (A3)+,D0            
01531A  121B                      MOVE.B    (A3)+,D1            
01531C  E141                      ASL.W     #8,D1               
01531E  121B                      MOVE.B    (A3)+,D1            
015320  32361000                  MOVE.W    0(A6,D1.W),D1       
015324  2079000170A8              MOVEA.L   $000170A8,A0         L067A
01532A  22701000                  MOVEA.L   0(A0,D1.W),A1       
01532E  C34E                      EXG       A6,A1               
015330  61000068                  BSR       104(PC)              L0555
015334  C34E                      EXG       A6,A1               
015336  1E310000                  MOVE.B    0(A1,D0.W),D7       
01533A  4887                      EXT.W     D7                  
01533C  4E75                      RTS                           
01533E  101B                      MOVE.B    (A3)+,D0            
015340  E140                      ASL.W     #8,D0               
015342  101B                      MOVE.B    (A3)+,D0            
015344  121B                      MOVE.B    (A3)+,D1            
015346  E141                      ASL.W     #8,D1               
015348  121B                      MOVE.B    (A3)+,D1            
01534A  32361000                  MOVE.W    0(A6,D1.W),D1       
01534E  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015354  22701000                  MOVEA.L   0(A0,D1.W),A1       
015358  C34E                      EXG       A6,A1               
01535A  6100007A                  BSR       122(PC)              L0557
01535E  C34E                      EXG       A6,A1               
015360  3E310000                  MOVE.W    0(A1,D0.W),D7       
015364  4E75                      RTS                           
015366  101B                      MOVE.B    (A3)+,D0            
015368  E140                      ASL.W     #8,D0               
01536A  101B                      MOVE.B    (A3)+,D0            
01536C  121B                      MOVE.B    (A3)+,D1            
01536E  E141                      ASL.W     #8,D1               
015370  121B                      MOVE.B    (A3)+,D1            
015372  32361000                  MOVE.W    0(A6,D1.W),D1       
015376  2079000170A8              MOVEA.L   $000170A8,A0         L067A
01537C  22701000                  MOVEA.L   0(A0,D1.W),A1       
015380  C34E                      EXG       A6,A1               
015382  61000072                  BSR       114(PC)              L0559
015386  C34E                      EXG       A6,A1               
015388  43F10000                  LEA       0(A1,D0.W),A1       
01538C  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015392  10D9                __L0554:
                                  MOVE.B    (A1)+,(A0)+         
015394  6600FFFC                  BNE       -4(PC)               __L0554
015398  4E75                      RTS                           


01539A  123600FF            L0555:
                                  MOVE.B    -1(A6,D0.W),D1      
01539E  4881                      EXT.W     D1                  
0153A0  41F600FE                  LEA       -2(A6,D0.W),A0      
0153A4  D047                      ADD.W     D7,D0               
0153A6  51C90004                  DBF       D1,4(PC)             L0556
0153AA  4E75                      RTS                           

0153AC  341C                L0556:MOVE.W    (A4)+,D2            
0153AE  C5E0                      MULS      -(A0),D2            
0153B0  D042                      ADD.W     D2,D0               
0153B2  51C9FFF8                  DBF       D1,-8(PC)            L0556
0153B6  4E75                      RTS                           
0153B8  E248                      LSR.W     #1,D0               
0153BA  3200                      MOVE.W    D0,D1               
0153BC  203C00000017              MOVE.L    #$17,D0             
0153C2  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
0153C8  3200                      MOVE.W    D0,D1               
0153CA  203C00000018              MOVE.L    #$18,D0             
0153D0  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
0153D6  123600FF            L0557:MOVE.B    -1(A6,D0.W),D1      
0153DA  4881                      EXT.W     D1                  
0153DC  41F600FE                  LEA       -2(A6,D0.W),A0      
0153E0  DE47                      ADD.W     D7,D7               
0153E2  D047                      ADD.W     D7,D0               
0153E4  51C90004                  DBF       D1,4(PC)             L0558
0153E8  4E75                      RTS                           
0153EA  341C                L0558:MOVE.W    (A4)+,D2            
0153EC  C5E0                      MULS      -(A0),D2            
0153EE  D042                      ADD.W     D2,D0               
0153F0  51C9FFF8                  DBF       D1,-8(PC)            L0558
0153F4  4E75                      RTS                           
0153F6  123600FF            L0559:MOVE.B    -1(A6,D0.W),D1      
0153FA  4881                      EXT.W     D1                  
0153FC  41F600FE                  LEA       -2(A6,D0.W),A0      
015400  4242                      CLR.W     D2                  
015402  1410                      MOVE.B    (A0),D2             
015404  CEC2                      MULU      D2,D7               
015406  D047                      ADD.W     D7,D0               
015408  51C90004                  DBF       D1,4(PC)             L055A
01540C  4E75                      RTS                           
01540E  341C                L055A:MOVE.W    (A4)+,D2            
015410  C5E0                      MULS      -(A0),D2            
015412  D042                      ADD.W     D2,D0               
015414  51C9FFF8                  DBF       D1,-8(PC)            L055A
015418  4E75                      RTS                           
01541A  6100FCD2                  BSR       -814(PC)             FN_READ_OPNAME_SAVE_D7
01541E  CE46                      AND.W     D6,D7               
015420  4E75                      RTS                           
015422  6100FCCA                  BSR       -822(PC)             FN_READ_OPNAME_SAVE_D7
015426  8E46                      OR.W      D6,D7               
015428  4E75                      RTS                           
01542A  6100FCC2                  BSR       -830(PC)             FN_READ_OPNAME_SAVE_D7
01542E  BD47                      EOR.W     D6,D7               
015430  4E75                      RTS                           
015432  6100FCBA                  BSR       -838(PC)             FN_READ_OPNAME_SAVE_D7
015436  BD47                      EOR.W     D6,D7               
015438  4647                      NOT.W     D7                  
01543A  4E75                      RTS                           
01543C  6100FCB0                  BSR       -848(PC)             FN_READ_OPNAME_SAVE_D7
015440  BE46                      CMP.W     D6,D7               
015442  67000006                  BEQ       6(PC)                __L055B
015446  4247                      CLR.W     D7                  
015448  4E75                      RTS                           
01544A  3E3CFFFF            __L055B:
                                  MOVE.W    #$FFFF,D7           
01544E  4E75                      RTS                           
015450  6100FC9C                  BSR       -868(PC)             FN_READ_OPNAME_SAVE_D7
015454  BE46                      CMP.W     D6,D7               
015456  66000006                  BNE       6(PC)                __L055C
01545A  4247                      CLR.W     D7                  
01545C  4E75                      RTS                           
01545E  3E3CFFFF            __L055C:
                                  MOVE.W    #$FFFF,D7           
015462  4E75                      RTS                           
015464  6100FC88                  BSR       -888(PC)             FN_READ_OPNAME_SAVE_D7
015468  BC47                      CMP.W     D7,D6               
01546A  6F000006                  BLE       6(PC)                __L055D
01546E  4247                      CLR.W     D7                  
015470  4E75                      RTS                           
015472  3E3CFFFF            __L055D:
                                  MOVE.W    #$FFFF,D7           
015476  4E75                      RTS                           
015478  6100FC74                  BSR       -908(PC)             FN_READ_OPNAME_SAVE_D7
01547C  BC47                      CMP.W     D7,D6               
01547E  6C000006                  BGE       6(PC)                __L055E
015482  4247                      CLR.W     D7                  
015484  4E75                      RTS                           
015486  3E3CFFFF            __L055E:
                                  MOVE.W    #$FFFF,D7           
01548A  4E75                      RTS                           
01548C  6100FC60                  BSR       -928(PC)             FN_READ_OPNAME_SAVE_D7
015490  BC47                      CMP.W     D7,D6               
015492  6D000006                  BLT       6(PC)                __L055F
015496  4247                      CLR.W     D7                  
015498  4E75                      RTS                           
01549A  3E3CFFFF            __L055F:
                                  MOVE.W    #$FFFF,D7           
01549E  4E75                      RTS                           
0154A0  6100FC4C                  BSR       -948(PC)             FN_READ_OPNAME_SAVE_D7
0154A4  BC47                      CMP.W     D7,D6               
0154A6  6E000006                  BGT       6(PC)                __L0560
0154AA  4247                      CLR.W     D7                  
0154AC  4E75                      RTS                           
0154AE  3E3CFFFF            __L0560:
                                  MOVE.W    #$FFFF,D7           
0154B2  4E75                      RTS                           
0154B4  6100FC38                  BSR       -968(PC)             FN_READ_OPNAME_SAVE_D7
0154B8  DE46                      ADD.W     D6,D7               
0154BA  4E75                      RTS                           
0154BC  6100FC30                  BSR       -976(PC)             FN_READ_OPNAME_SAVE_D7
0154C0  9C47                      SUB.W     D7,D6               
0154C2  3E06                      MOVE.W    D6,D7               
0154C4  4E75                      RTS                           
0154C6  6100FC26                  BSR       -986(PC)             FN_READ_OPNAME_SAVE_D7
0154CA  48C6                      EXT.L     D6                  
0154CC  8DC7                      DIVS      D7,D6               
0154CE  2E06                      MOVE.L    D6,D7               
0154D0  4847                      SWAP      D7                  
0154D2  4E75                      RTS                           
0154D4  6100FC18                  BSR       -1000(PC)            FN_READ_OPNAME_SAVE_D7
0154D8  48C6                      EXT.L     D6                  
0154DA  8DC7                      DIVS      D7,D6               
0154DC  3E06                      MOVE.W    D6,D7               
0154DE  4E75                      RTS                           
0154E0  6100FC0C                  BSR       -1012(PC)            FN_READ_OPNAME_SAVE_D7
0154E4  CFC6                      MULS      D6,D7               
0154E6  4E75                      RTS                           
0154E8  4447                      NEG.W     D7                  
0154EA  4E75                      RTS                           
0154EC  4A47                      TST.W     D7                  
0154EE  6A000004                  BPL       4(PC)                __L0561
0154F2  4447                      NEG.W     D7                  
0154F4  4E75                __L0561:
                                  RTS                           
0154F6  303900017136              MOVE.W    $00017136,D0         L06BE
0154FC  6100000E                  BSR       14(PC)               L0562
015500  33C000017136              MOVE.W    D0,$00017136         L06BE
015506  CEC0                      MULU      D0,D7               
015508  4847                      SWAP      D7                  
01550A  4E75                      RTS                           
01550C  C0FC7AB7            L0562:MULU      #$7AB7,D0           
015510  068032A9F881              ADDI.L    #$32A9F881,D0       
015516  4E75                      RTS                           
015518  4A47                      TST.W     D7                  
01551A  6700000C                  BEQ       12(PC)               __L0563
01551E  6B00000C                  BMI       12(PC)               __L0564
015522  3E3C0001                  MOVE.W    #1,D7               
015526  4E75                      RTS                           
015528  4247                __L0563:
                                  CLR.W     D7                  
01552A  4E75                      RTS                           
01552C  3E3CFFFF            __L0564:
                                  MOVE.W    #$FFFF,D7           
015530  4E75                      RTS                           
015532  4647                      NOT.W     D7                  
015534  4E75                      RTS                           
015536  4A3900017209              TST.B     $00017209            L06FF
01553C  67000008                  BEQ       8(PC)                __L0565
015540  4247                      CLR.W     D7                  
015542  1E00                      MOVE.B    D0,D7               
015544  4E75                      RTS                           
015546  4EB90000B77E        __L0565:
                                  JSR       $0000B77E            L0065
01554C  4247                      CLR.W     D7                  
01554E  3E00                      MOVE.W    D0,D7               
015550  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd9 -  offset 0x4830 -  address $015552
015552  6100FB9C                  BSR       -1124(PC)            FN_READ_OPNAME
015556  4E75                      RTS                           


; =============================================================================
; OPCODE 0xda -  offset 0x4836 -  address $015558
015558  6100FBA2                  BSR       -1118(PC)            L054B
01555C  4E75                      RTS                           


; =============================================================================
; OPCODE 0xd4 -  offset 0x483c -  address $01555e
01555E  4E75                      RTS                           
015560  4E75                      RTS                           
015562  4A3900017209              TST.B     $00017209            L06FF
015568  67000008                  BEQ       8(PC)                __L0566
01556C  4247                      CLR.W     D7                  
01556E  1E01                      MOVE.B    D1,D7               
015570  4E75                      RTS                           
015572  4EB90000B788        __L0566:
                                  JSR       $0000B788            L0066
015578  4247                      CLR.W     D7                  
01557A  1E00                      MOVE.B    D0,D7               
01557C  4E75                      RTS                           
01557E  4EB90000B792              JSR       $0000B792            L0067
015584  3E00                      MOVE.W    D0,D7               
015586  4E75                      RTS                           
015588  4A3900017209              TST.B     $00017209            L06FF
01558E  67000006                  BEQ       6(PC)                __L0567
015592  4247                      CLR.W     D7                  
015594  4E75                      RTS                           
015596  3007                __L0567:
                                  MOVE.W    D7,D0               
015598  4EB90000B716              JSR       $0000B716            __L005D
01559E  4247                      CLR.W     D7                  
0155A0  1E00                      MOVE.B    D0,D7               
0155A2  4E75                      RTS                           
0155A4  4A3900017209              TST.B     $00017209            L06FF
0155AA  67000006                  BEQ       6(PC)                __L0568
0155AE  4247                      CLR.W     D7                  
0155B0  4E75                      RTS                           
0155B2  3007                __L0568:
                                  MOVE.W    D7,D0               
0155B4  4EB90000B730              JSR       $0000B730            __L005F
0155BA  4247                      CLR.W     D7                  
0155BC  1E00                      MOVE.B    D0,D7               
0155BE  4E75                      RTS                           
0155C0  301C                      MOVE.W    (A4)+,D0            
0155C2  EC58                      ROR.W     #6,D0               
0155C4  6100FF46                  BSR       -186(PC)             L0562
0155C8  CEC0                      MULU      D0,D7               
0155CA  4847                      SWAP      D7                  
0155CC  4E75                      RTS                           
0155CE  4A47                      TST.W     D7                  
0155D0  67000068                  BEQ       104(PC)              __L0569
0155D4  BE7C0001                  CMP.W     #1,D7               
0155D8  67000072                  BEQ       114(PC)              __L056A
0155DC  BE7C0002                  CMP.W     #2,D7               
0155E0  6700007C                  BEQ       124(PC)              __L056B
0155E4  BE7C0003                  CMP.W     #3,D7               
0155E8  67000082                  BEQ       130(PC)              __L056C
0155EC  BE7C0004                  CMP.W     #4,D7               
0155F0  67000088                  BEQ       136(PC)              __L056D
0155F4  BE7C0061                  CMP.W     #$61,D7             ;(#"a") 
0155F8  6700009E                  BEQ       158(PC)              L0570
0155FC  BE7C0041                  CMP.W     #$41,D7             ;(#"A") 
015600  670000A6                  BEQ       166(PC)              L0571
015604  BE7C0062                  CMP.W     #$62,D7             ;(#"b") 
015608  6700008E                  BEQ       142(PC)              L0570
01560C  BE7C0042                  CMP.W     #$42,D7             ;(#"B") 
015610  67000096                  BEQ       150(PC)              L0571
015614  BE7C0063                  CMP.W     #$63,D7             ;(#"c") 
015618  6700007E                  BEQ       126(PC)              L0570
01561C  BE7C0043                  CMP.W     #$43,D7             ;(#"C") 
015620  67000086                  BEQ       134(PC)              L0571
015624  BE7C0064                  CMP.W     #$64,D7             ;(#"d") 
015628  6700006E                  BEQ       110(PC)              L0570
01562C  BE7C0044                  CMP.W     #$44,D7             ;(#"D") 
015630  67000076                  BEQ       118(PC)              L0571
015634  3E3CFFFF                  MOVE.W    #$FFFF,D7           
015638  4E75                      RTS                           
01563A  2E3900017066        __L0569:
                                  MOVE.L    $00017066,D7         L065D
015640  9EB90001709C              SUB.L     $0001709C,D7         L0676
015646  8EFC03E8                  DIVU      #$3E8,D7            
01564A  4E75                      RTS                           
01564C  2E39000170BA        __L056A:
                                  MOVE.L    $000170BA,D7         L0680
015652  9EB9000170B0              SUB.L     $000170B0,D7         L067C
015658  8EFC03E8                  DIVU      #$3E8,D7            
01565C  4E75                      RTS                           
01565E  3E39000170A0        __L056B:
                                  MOVE.W    $000170A0,D7         L0677
015664  9E79000170A2              SUB.W     $000170A2,D7         L0678
01566A  4E75                      RTS                           
01566C  3E39000170B4        __L056C:
                                  MOVE.W    $000170B4,D7         L067D
015672  9E79000170B6              SUB.W     $000170B6,D7         L067E
015678  4E75                      RTS                           
01567A  4247                __L056D:
                                  CLR.W     D7                  
01567C  2079000170C2              MOVEA.L   $000170C2,A0         L_L0682
015682  3039000170C6              MOVE.W    $000170C6,D0         W_L0683
015688  6700000C                  BEQ       12(PC)               __L056F
01568C  5247                __L056E:
                                  ADDQ.W    #1,D7               
01568E  30300004                  MOVE.W    4(A0,D0.W),D0       
015692  6600FFF8                  BNE       -8(PC)               __L056E
015696  4E75                __L056F:
                                  RTS                           
015698  3007                L0570:MOVE.W    D7,D0               
01569A  04400061                  SUBI.W    #$61,D0             
01569E  4EB90000C100              JSR       $0000C100            SYS_DISK_GET_FREE_SPACE
0156A4  3E00                      MOVE.W    D0,D7               
0156A6  4E75                      RTS                           
0156A8  3007                L0571:MOVE.W    D7,D0               
0156AA  04400041                  SUBI.W    #$41,D0             
0156AE  4EB90000C100              JSR       $0000C100            SYS_DISK_GET_FREE_SPACE
0156B4  3E00                      MOVE.W    D0,D7               
0156B6  4E75                      RTS                           
0156B8  4EB90000B912              JSR       $0000B912            __L0072
0156BE  3E00                      MOVE.W    D0,D7               
0156C0  4E75                      RTS                           
0156C2  61000298                  BSR       664(PC)              L058F
0156C6  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0156CC  024700FF                  ANDI.W    #$FF,D7             
0156D0  42307000                  CLR.B     0(A0,D7.W)          
0156D4  4E75                      RTS                           
0156D6  61000284                  BSR       644(PC)              L058F
0156DA  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0156E0  4A18                __L0572:
                                  TST.B     (A0)+               
0156E2  6600FFFC                  BNE       -4(PC)               __L0572
0156E6  5388                      SUBQ.L    #1,A0               
0156E8  90C7                      SUBA.W    D7,A0               
0156EA  B1F900017146              CMPA.L    $00017146,A0         ADDR_SCRIPT_NAME
0156F0  62000008                  BHI       8(PC)                __L0573
0156F4  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0156FA  227900017146        __L0573:
                                  MOVEA.L   $00017146,A1         ADDR_SCRIPT_NAME
015700  12D8                __L0574:
                                  MOVE.B    (A0)+,(A1)+         
015702  6600FFFC                  BNE       -4(PC)               __L0574
015706  4E75                      RTS                           
015708  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01570E  D0DC                      ADDA.W    (A4)+,A0            
015710  6100024A                  BSR       586(PC)              L058F
015714  227900017146              MOVEA.L   $00017146,A1         ADDR_SCRIPT_NAME
01571A  5347                      SUBQ.W    #1,D7               
01571C  12D8                __L0575:
                                  MOVE.B    (A0)+,(A1)+         
01571E  57CFFFFC                  DBEQ      D7,-4(PC)            __L0575
015722  4A47                      TST.W     D7                  
015724  6A000004                  BPL       4(PC)                __L0576
015728  4211                      CLR.B     (A1)                
01572A  4E75                __L0576:
                                  RTS                           
01572C  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015732  4A18                __L0577:
                                  TST.B     (A0)+               
015734  6600FFFC                  BNE       -4(PC)               __L0577
015738  91F900017146              SUBA.L    $00017146,A0         ADDR_SCRIPT_NAME
01573E  3E08                      MOVE.W    A0,D7               
015740  5347                      SUBQ.W    #1,D7               
015742  4E75                      RTS                           
015744  4247                __L0578:
                                  CLR.W     D7                  
015746  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01574C  4241                      CLR.W     D1                  
01574E  4A10                      TST.B     (A0)                
015750  67000080                  BEQ       128(PC)              L057F
015754  0C10002D                  CMPI.B    #$2D,(A0)           ;(#"-") 
015758  66000008                  BNE       8(PC)                __L0579
01575C  123C0001                  MOVE.B    #1,D1               
015760  5288                      ADDQ.L    #1,A0               
015762  0C100024            __L0579:
                                  CMPI.B    #$24,(A0)           ;(#"$") 
015766  67000028                  BEQ       40(PC)               __L057B
01576A  1018                __L057A:
                                  MOVE.B    (A0)+,D0            
01576C  67000064                  BEQ       100(PC)              L057F
015770  B03C0030                  CMP.B     #$30,D0             ;(#"0") 
015774  6B00005C                  BMI       92(PC)               L057F
015778  B03C0039                  CMP.B     #$39,D0             ;(#"9") 
01577C  62000054                  BHI       84(PC)               L057F
015780  CEFC000A                  MULU      #$A,D7              
015784  04000030                  SUBI.B    #$30,D0             
015788  4880                      EXT.W     D0                  
01578A  DE40                      ADD.W     D0,D7               
01578C  6000FFDC                  BRA       -36(PC)              __L057A
015790  5288                __L057B:
                                  ADDQ.L    #1,A0               
015792  1018                __L057C:
                                  MOVE.B    (A0)+,D0            
015794  6700003C                  BEQ       60(PC)               L057F
015798  B03C0030                  CMP.B     #$30,D0             ;(#"0") 
01579C  6B000034                  BMI       52(PC)               L057F
0157A0  B03C0039                  CMP.B     #$39,D0             ;(#"9") 
0157A4  62000010                  BHI       16(PC)               __L057E
0157A8  04000030                  SUBI.B    #$30,D0             
0157AC  4880                __L057D:
                                  EXT.W     D0                  
0157AE  E947                      ASL.W     #4,D7               
0157B0  DE40                      ADD.W     D0,D7               
0157B2  6000FFDE                  BRA       -34(PC)              __L057C
0157B6  020000DF            __L057E:
                                  ANDI.B    #$DF,D0             
0157BA  B03C0041                  CMP.B     #$41,D0             ;(#"A") 
0157BE  6B000012                  BMI       18(PC)               L057F
0157C2  B03C0046                  CMP.B     #$46,D0             ;(#"F") 
0157C6  6200000A                  BHI       10(PC)               L057F
0157CA  04000037                  SUBI.B    #$37,D0             
0157CE  6000FFDC                  BRA       -36(PC)              __L057D
0157D2  4A01                L057F:TST.B     D1                  
0157D4  66000004                  BNE       4(PC)                __L0580
0157D8  4E75                      RTS                           
0157DA  4447                __L0580:
                                  NEG.W     D7                  
0157DC  4E75                      RTS                           
0157DE  4247                      CLR.W     D7                  
0157E0  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0157E6  1E10                      MOVE.B    (A0),D7             
0157E8  4E75                      RTS                           
0157EA  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0157F0  3007                      MOVE.W    D7,D0               
0157F2  6100C072                  BSR       -16270(PC)           L0380
0157F6  4E75                      RTS                           
0157F8  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0157FE  10C7                      MOVE.B    D7,(A0)+            
015800  4210                      CLR.B     (A0)                
015802  4E75                      RTS                           
015804  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
01580A  4EB90000C03C              JSR       $0000C03C            SYS_DISK_FOPEN_R
015810  67000006                  BEQ       6(PC)                __L0581
015814  4247                      CLR.W     D7                  
015816  4E75                      RTS                           
015818  3E3CFFFF            __L0581:
                                  MOVE.W    #$FFFF,D7           
01581C  4E75                      RTS                           
01581E  4A47                      TST.W     D7                  
015820  BE7C0061                  CMP.W     #$61,D7             ;(#"a") 
015824  67000048                  BEQ       72(PC)               L0583
015828  BE7C0041                  CMP.W     #$41,D7             ;(#"A") 
01582C  67000040                  BEQ       64(PC)               L0583
015830  BE7C0062                  CMP.W     #$62,D7             ;(#"b") 
015834  6700004A                  BEQ       74(PC)               L0584
015838  BE7C0042                  CMP.W     #$42,D7             ;(#"B") 
01583C  67000042                  BEQ       66(PC)               L0584
015840  BE7C0063                  CMP.W     #$63,D7             ;(#"c") 
015844  6700004C                  BEQ       76(PC)               L0585
015848  BE7C0043                  CMP.W     #$43,D7             ;(#"C") 
01584C  67000044                  BEQ       68(PC)               L0585
015850  BE7C0064                  CMP.W     #$64,D7             ;(#"d") 
015854  6700004E                  BEQ       78(PC)               L0586
015858  BE7C0044                  CMP.W     #$44,D7             ;(#"D") 
01585C  67000046                  BEQ       70(PC)               L0586
015860  BE7C003F                  CMP.W     #$3F,D7             ;(#"?") 
015864  67000050                  BEQ       80(PC)               __L0587
015868  3E3CFFFF            L0582:MOVE.W    #$FFFF,D7           
01586C  4E75                      RTS                           
01586E  303C0000            L0583:MOVE.W    #0,D0               
015872  4EB90000C140              JSR       $0000C140            L00B9
015878  4A40                      TST.W     D0                  
01587A  6B00FFEC                  BMI       -20(PC)              L0582
01587E  4E75                      RTS                           
015880  303C0001            L0584:MOVE.W    #1,D0               
015884  4EB90000C140              JSR       $0000C140            L00B9
01588A  4A40                      TST.W     D0                  
01588C  6B00FFDA                  BMI       -38(PC)              L0582
015890  4E75                      RTS                           
015892  303C0002            L0585:MOVE.W    #2,D0               
015896  4EB90000C140              JSR       $0000C140            L00B9
01589C  4A40                      TST.W     D0                  
01589E  6B00FFC8                  BMI       -56(PC)              L0582
0158A2  4E75                      RTS                           
0158A4  303C0003            L0586:MOVE.W    #3,D0               
0158A8  4EB90000C140              JSR       $0000C140            L00B9
0158AE  4A40                      TST.W     D0                  
0158B0  6B00FFB6                  BMI       -74(PC)              L0582
0158B4  4E75                      RTS                           
0158B6  303CFFFF            __L0587:
                                  MOVE.W    #$FFFF,D0           
0158BA  4EB90000C140              JSR       $0000C140            L00B9
0158C0  4A40                      TST.W     D0                  
0158C2  6B00FFA4                  BMI       -92(PC)              L0582
0158C6  B07C001A                  CMP.W     #$1A,D0             
0158CA  6C00FF9C                  BGE       -100(PC)             L0582
0158CE  06400041                  ADDI.W    #$41,D0             
0158D2  3E00                      MOVE.W    D0,D7               
0158D4  4E75                      RTS                           
0158D6  4247                      CLR.W     D7                  
0158D8  1E3900017071              MOVE.B    $00017071,D7         L0663
0158DE  4E75                      RTS                           
0158E0  4EB90000B9C8              JSR       $0000B9C8            __L007F
0158E6  3E00                      MOVE.W    D0,D7               
0158E8  4E75                      RTS                           
0158EA  4EB90000B9CE              JSR       $0000B9CE            __L0080
0158F0  3E00                      MOVE.W    D0,D7               
0158F2  4E75                      RTS                           
0158F4  227900017146        L0588:MOVEA.L   $00017146,A1         ADDR_SCRIPT_NAME
0158FA  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
015900  B109                __L0589:
                                  CMPM.B    (A1)+,(A0)+         
015902  6600000E                  BNE       14(PC)               __L058A
015906  4A28FFFF                  TST.B     -1(A0)              
01590A  6600FFF4                  BNE       -12(PC)              __L0589
01590E  B040                      CMP.W     D0,D0               
015910  4E75                      RTS                           
015912  4A28FFFF            __L058A:
                                  TST.B     -1(A0)              
015916  66000006                  BNE       6(PC)                __L058B
01591A  B040                      CMP.W     D0,D0               
01591C  4E75                      RTS                           
01591E  1028FFFF            __L058B:
                                  MOVE.B    -1(A0),D0           
015922  9029FFFF                  SUB.B     -1(A1),D0           
015926  4E75                      RTS                           
015928  48E78080            __L058C:
                                  MOVEM.L   A0/D0,-(A7)         
01592C  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015932  4A18                __L058D:
                                  TST.B     (A0)+               
015934  6600FFFC                  BNE       -4(PC)               __L058D
015938  2008                      MOVE.L    A0,D0               
01593A  90B900017146              SUB.L     $00017146,D0         ADDR_SCRIPT_NAME
015940  08000000                  BTST      #0,D0               
015944  67000004                  BEQ       4(PC)                L058E
015948  4224                      CLR.B     -(A4)               
01594A  1920                L058E:MOVE.B    -(A0),-(A4)         
01594C  B1F900017146              CMPA.L    $00017146,A0         ADDR_SCRIPT_NAME
015952  6600FFF6                  BNE       -10(PC)              L058E
015956  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
01595A  4E75                      RTS                           
01595C  48E78080            L058F:MOVEM.L   A0/D0,-(A7)         
015960  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
015966  10DC                __L0590:
                                  MOVE.B    (A4)+,(A0)+         
015968  6600FFFC                  BNE       -4(PC)               __L0590
01596C  200C                      MOVE.L    A4,D0               
01596E  08000000                  BTST      #0,D0               
015972  67000004                  BEQ       4(PC)                __L0591
015976  528C                      ADDQ.L    #1,A4               
015978  4CDF0101            __L0591:
                                  MOVEM.L   (A7)+,A0/D0         
01597C  4E75                      RTS                           
01597E  48E78080                  MOVEM.L   A0/D0,-(A7)         
015982  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
015988  4A18                __L0592:
                                  TST.B     (A0)+               
01598A  6600FFFC                  BNE       -4(PC)               __L0592
01598E  2008                      MOVE.L    A0,D0               
015990  90B90001714A              SUB.L     $0001714A,D0         ADDR_CURRENT_PATH
015996  08000000                  BTST      #0,D0               
01599A  67000004                  BEQ       4(PC)                L0593
01599E  4224                      CLR.B     -(A4)               
0159A0  1920                L0593:MOVE.B    -(A0),-(A4)         
0159A2  B1F90001714A              CMPA.L    $0001714A,A0         ADDR_CURRENT_PATH
0159A8  6600FFF6                  BNE       -10(PC)              L0593
0159AC  4CDF0101                  MOVEM.L   (A7)+,A0/D0         
0159B0  4E75                      RTS                           
0159B2  2F00                __L0594:
                                  MOVE.L    D0,-(A7)            
0159B4  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
0159BA  10DC                __L0595:
                                  MOVE.B    (A4)+,(A0)+         
0159BC  6600FFFC                  BNE       -4(PC)               __L0595
0159C0  200C                      MOVE.L    A4,D0               
0159C2  08000000                  BTST      #0,D0               
0159C6  67000004                  BEQ       4(PC)                __L0596
0159CA  538C                      SUBQ.L    #1,A4               
0159CC  201F                __L0596:
                                  MOVE.L    (A7)+,D0            
0159CE  4E75                      RTS                           
0159D0  6100FF56                  BSR       -170(PC)             __L058C
0159D4  4E75                      RTS                           
0159D6  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
0159DC  23F9000171460001714A      MOVE.L    $00017146,$0001714A  ADDR_SCRIPT_NAME ADDR_CURRENT_PATH
0159E6  23C800017146              MOVE.L    A0,$00017146         ADDR_SCRIPT_NAME
0159EC  6100FFC4                  BSR       -60(PC)              __L0594
0159F0  4E75                      RTS                           
0159F2  6100F708                  BSR       -2296(PC)            L054B
0159F6  6100FEFC                  BSR       -260(PC)             L0588
0159FA  67000006                  BEQ       6(PC)                __L0597
0159FE  4247                      CLR.W     D7                  
015A00  4E75                      RTS                           
015A02  3E3CFFFF            __L0597:
                                  MOVE.W    #$FFFF,D7           
015A06  4E75                      RTS                           
015A08  6100F6F2                  BSR       -2318(PC)            L054B
015A0C  6100FEE6                  BSR       -282(PC)             L0588
015A10  66000006                  BNE       6(PC)                __L0598
015A14  4247                      CLR.W     D7                  
015A16  4E75                      RTS                           
015A18  3E3CFFFF            __L0598:
                                  MOVE.W    #$FFFF,D7           
015A1C  4E75                      RTS                           
015A1E  6100F6DC                  BSR       -2340(PC)            L054B
015A22  6100FED0                  BSR       -304(PC)             L0588
015A26  6F000006                  BLE       6(PC)                __L0599
015A2A  4247                      CLR.W     D7                  
015A2C  4E75                      RTS                           
015A2E  3E3CFFFF            __L0599:
                                  MOVE.W    #$FFFF,D7           
015A32  4E75                      RTS                           
015A34  6100F6C6                  BSR       -2362(PC)            L054B
015A38  6100FEBA                  BSR       -326(PC)             L0588
015A3C  6C000006                  BGE       6(PC)                __L059A
015A40  4247                      CLR.W     D7                  
015A42  4E75                      RTS                           
015A44  3E3CFFFF            __L059A:
                                  MOVE.W    #$FFFF,D7           
015A48  4E75                      RTS                           
015A4A  6100F6B0                  BSR       -2384(PC)            L054B
015A4E  6100FEA4                  BSR       -348(PC)             L0588
015A52  6D000006                  BLT       6(PC)                __L059B
015A56  4247                      CLR.W     D7                  
015A58  4E75                      RTS                           
015A5A  3E3CFFFF            __L059B:
                                  MOVE.W    #$FFFF,D7           
015A5E  4E75                      RTS                           
015A60  6100F69A                  BSR       -2406(PC)            L054B
015A64  6100FE8E                  BSR       -370(PC)             L0588
015A68  6E000006                  BGT       6(PC)                __L059C
015A6C  4247                      CLR.W     D7                  
015A6E  4E75                      RTS                           
015A70  3E3CFFFF            __L059C:
                                  MOVE.W    #$FFFF,D7           
015A74  4E75                      RTS                           
015A76  6100F684                  BSR       -2428(PC)            L054B
015A7A  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
015A80  227900017146              MOVEA.L   $00017146,A1         ADDR_SCRIPT_NAME
015A86  4A18                __L059D:
                                  TST.B     (A0)+               
015A88  6600FFFC                  BNE       -4(PC)               __L059D
015A8C  5388                      SUBQ.L    #1,A0               
015A8E  10D9                __L059E:
                                  MOVE.B    (A1)+,(A0)+         
015A90  6600FFFC                  BNE       -4(PC)               __L059E
015A94  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
015A9A  23F9000171460001714A      MOVE.L    $00017146,$0001714A  ADDR_SCRIPT_NAME ADDR_CURRENT_PATH
015AA4  23C800017146              MOVE.L    A0,$00017146         ADDR_SCRIPT_NAME
015AAA  4E75                      RTS                           
015AAC  3907                      MOVE.W    D7,-(A4)            
015AAE  4E75                      RTS                           
015AB0  3E06                      MOVE.W    D6,D7               
015AB2  3C1C                      MOVE.W    (A4)+,D6            
015AB4  4E75                      RTS                           


015AB6  6100F638            FN_READ_OPNAME_LOOP:
                                  BSR       -2504(PC)            FN_READ_OPNAME
015ABA  60FA                      BRA.S     -6(PC)               FN_READ_OPNAME_LOOP


; =============================================================================
; ADD_JUMP_TABLE 0x1e -  offset 0x4aee -  address $015abc
; 

; OPNAME 0x3A - oshiftkey
015ABC  588F                      ADDQ.L    #4,A7               
015ABE  4E75                      RTS                           


015AC0  101B                      MOVE.B    (A3)+,D0            
015AC2  E140                      ASL.W     #8,D0               
015AC4  101B                      MOVE.B    (A3)+,D0            
015AC6  1D870000                  MOVE.B    D7,0(A6,D0.W)       
015ACA  4E75                      RTS                           
015ACC  101B                      MOVE.B    (A3)+,D0            
015ACE  E140                      ASL.W     #8,D0               
015AD0  101B                      MOVE.B    (A3)+,D0            
015AD2  3D870000                  MOVE.W    D7,0(A6,D0.W)       
015AD6  4E75                      RTS                           
015AD8  101B                      MOVE.B    (A3)+,D0            
015ADA  E140                      ASL.W     #8,D0               
015ADC  101B                      MOVE.B    (A3)+,D0            
015ADE  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015AE4  43F60000                  LEA       0(A6,D0.W),A1       
015AE8  12D8                __L05A0:
                                  MOVE.B    (A0)+,(A1)+         
015AEA  6600FFFC                  BNE       -4(PC)               __L05A0
015AEE  4E75                      RTS          


; STORENAME 0x0E
; sdirti                 
015AF0  101B                      MOVE.B    (A3)+,D0            
015AF2  E140                      ASL.W     #8,D0               
015AF4  101B                      MOVE.B    (A3)+,D0            
015AF6  6100F8A2                  BSR       -1886(PC)            L0555
015AFA  3E1C                      MOVE.W    (A4)+,D7            
015AFC  1D870000                  MOVE.B    D7,0(A6,D0.W)       
015B00  4E75                      RTS                           


015B02  101B                      MOVE.B    (A3)+,D0            
015B04  E140                      ASL.W     #8,D0               
015B06  101B                      MOVE.B    (A3)+,D0            
015B08  6100F8CC                  BSR       -1844(PC)            L0557
015B0C  3E1C                      MOVE.W    (A4)+,D7            
015B0E  3D870000                  MOVE.W    D7,0(A6,D0.W)       
015B12  4E75                      RTS                           
015B14  101B                      MOVE.B    (A3)+,D0            
015B16  E140                      ASL.W     #8,D0               
015B18  101B                      MOVE.B    (A3)+,D0            
015B1A  6100F8DA                  BSR       -1830(PC)            L0559
015B1E  43F60000                  LEA       0(A6,D0.W),A1       
015B22  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015B28  12D8                __L05A1:
                                  MOVE.B    (A0)+,(A1)+         
015B2A  6600FFFC                  BNE       -4(PC)               __L05A1
015B2E  4E75                      RTS                           
015B30  4240                      CLR.W     D0                  
015B32  101B                      MOVE.B    (A3)+,D0            
015B34  1D870000                  MOVE.B    D7,0(A6,D0.W)       
015B38  4E75                      RTS                           
015B3A  4240                      CLR.W     D0                  
015B3C  101B                      MOVE.B    (A3)+,D0            
015B3E  3D870000                  MOVE.W    D7,0(A6,D0.W)       
015B42  4E75                      RTS                           
015B44  4240                      CLR.W     D0                  
015B46  101B                      MOVE.B    (A3)+,D0            
015B48  43F60000                  LEA       0(A6,D0.W),A1       
015B4C  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015B52  12D8                __L05A2:
                                  MOVE.B    (A0)+,(A1)+         
015B54  6600FFFC                  BNE       -4(PC)               __L05A2
015B58  4E75                      RTS                           
015B5A  4240                      CLR.W     D0                  
015B5C  101B                      MOVE.B    (A3)+,D0            
015B5E  6100F83A                  BSR       -1990(PC)            L0555
015B62  3E1C                      MOVE.W    (A4)+,D7            
015B64  1D870000                  MOVE.B    D7,0(A6,D0.W)       
015B68  4E75                      RTS                           
015B6A  4240                      CLR.W     D0                  
015B6C  101B                      MOVE.B    (A3)+,D0            
015B6E  6100F866                  BSR       -1946(PC)            L0557
015B72  3E1C                      MOVE.W    (A4)+,D7            
015B74  3D870000                  MOVE.W    D7,0(A6,D0.W)       
015B78  4E75                      RTS                           
015B7A  4240                      CLR.W     D0                  
015B7C  101B                      MOVE.B    (A3)+,D0            
015B7E  6100F876                  BSR       -1930(PC)            L0559
015B82  43F60000                  LEA       0(A6,D0.W),A1       
015B86  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015B8C  12D8                __L05A3:
                                  MOVE.B    (A0)+,(A1)+         
015B8E  6600FFFC                  BNE       -4(PC)               __L05A3
015B92  4E75                      RTS                           
015B94  101B                      MOVE.B    (A3)+,D0            
015B96  E140                      ASL.W     #8,D0               
015B98  101B                      MOVE.B    (A3)+,D0            
015B9A  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015BA0  13870000                  MOVE.B    D7,0(A1,D0.W)       
015BA4  4E75                      RTS                           
015BA6  101B                      MOVE.B    (A3)+,D0            
015BA8  E140                      ASL.W     #8,D0               
015BAA  101B                      MOVE.B    (A3)+,D0            
015BAC  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015BB2  33870000                  MOVE.W    D7,0(A1,D0.W)       
015BB6  4E75                      RTS                           
015BB8  101B                      MOVE.B    (A3)+,D0            
015BBA  E140                      ASL.W     #8,D0               
015BBC  101B                      MOVE.B    (A3)+,D0            
015BBE  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015BC4  43F10000                  LEA       0(A1,D0.W),A1       
015BC8  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015BCE  12D8                __L05A4:
                                  MOVE.B    (A0)+,(A1)+         
015BD0  6600FFFC                  BNE       -4(PC)               __L05A4
015BD4  4E75                      RTS                           
015BD6  101B                      MOVE.B    (A3)+,D0            
015BD8  E140                      ASL.W     #8,D0               
015BDA  101B                      MOVE.B    (A3)+,D0            
015BDC  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015BE2  C34E                      EXG       A6,A1               
015BE4  6100F7B4                  BSR       -2124(PC)            L0555
015BE8  C34E                      EXG       A6,A1               
015BEA  3E1C                      MOVE.W    (A4)+,D7            
015BEC  13870000                  MOVE.B    D7,0(A1,D0.W)       
015BF0  4E75                      RTS                           
015BF2  101B                      MOVE.B    (A3)+,D0            
015BF4  E140                      ASL.W     #8,D0               
015BF6  101B                      MOVE.B    (A3)+,D0            
015BF8  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015BFE  C34E                      EXG       A6,A1               
015C00  6100F7D4                  BSR       -2092(PC)            L0557
015C04  C34E                      EXG       A6,A1               
015C06  3E1C                      MOVE.W    (A4)+,D7            
015C08  33870000                  MOVE.W    D7,0(A1,D0.W)       
015C0C  4E75                      RTS                           
015C0E  101B                      MOVE.B    (A3)+,D0            
015C10  E140                      ASL.W     #8,D0               
015C12  101B                      MOVE.B    (A3)+,D0            
015C14  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015C1A  C34E                      EXG       A6,A1               
015C1C  6100F7D8                  BSR       -2088(PC)            L0559
015C20  C34E                      EXG       A6,A1               
015C22  43F10000                  LEA       0(A1,D0.W),A1       
015C26  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015C2C  12D8                __L05A5:
                                  MOVE.B    (A0)+,(A1)+         
015C2E  6600FFFC                  BNE       -4(PC)               __L05A5
015C32  4E75                      RTS                           
015C34  101B                      MOVE.B    (A3)+,D0            
015C36  E140                      ASL.W     #8,D0               
015C38  101B                      MOVE.B    (A3)+,D0            
015C3A  30360000                  MOVE.W    0(A6,D0.W),D0       
015C3E  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015C44  22700000                  MOVEA.L   0(A0,D0.W),A1       
015C48  101B                      MOVE.B    (A3)+,D0            
015C4A  E140                      ASL.W     #8,D0               
015C4C  101B                      MOVE.B    (A3)+,D0            
015C4E  13870000                  MOVE.B    D7,0(A1,D0.W)       
015C52  4E75                      RTS                           
015C54  101B                      MOVE.B    (A3)+,D0            
015C56  E140                      ASL.W     #8,D0               
015C58  101B                      MOVE.B    (A3)+,D0            
015C5A  30360000                  MOVE.W    0(A6,D0.W),D0       
015C5E  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015C64  22700000                  MOVEA.L   0(A0,D0.W),A1       
015C68  101B                      MOVE.B    (A3)+,D0            
015C6A  E140                      ASL.W     #8,D0               
015C6C  101B                      MOVE.B    (A3)+,D0            
015C6E  33870000                  MOVE.W    D7,0(A1,D0.W)       
015C72  4E75                      RTS                           
015C74  101B                      MOVE.B    (A3)+,D0            
015C76  E140                      ASL.W     #8,D0               
015C78  101B                      MOVE.B    (A3)+,D0            
015C7A  30360000                  MOVE.W    0(A6,D0.W),D0       
015C7E  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015C84  22700000                  MOVEA.L   0(A0,D0.W),A1       
015C88  101B                      MOVE.B    (A3)+,D0            
015C8A  E140                      ASL.W     #8,D0               
015C8C  101B                      MOVE.B    (A3)+,D0            
015C8E  43F10000                  LEA       0(A1,D0.W),A1       
015C92  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015C98  12D8                __L05A6:
                                  MOVE.B    (A0)+,(A1)+         
015C9A  6600FFFC                  BNE       -4(PC)               __L05A6
015C9E  4E75                      RTS                           
015CA0  101B                      MOVE.B    (A3)+,D0            
015CA2  E140                      ASL.W     #8,D0               
015CA4  101B                      MOVE.B    (A3)+,D0            
015CA6  121B                      MOVE.B    (A3)+,D1            
015CA8  E141                      ASL.W     #8,D1               
015CAA  121B                      MOVE.B    (A3)+,D1            
015CAC  32361000                  MOVE.W    0(A6,D1.W),D1       
015CB0  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015CB6  22701000                  MOVEA.L   0(A0,D1.W),A1       
015CBA  C34E                      EXG       A6,A1               
015CBC  6100F6DC                  BSR       -2340(PC)            L0555
015CC0  C34E                      EXG       A6,A1               
015CC2  3E1C                      MOVE.W    (A4)+,D7            
015CC4  13870000                  MOVE.B    D7,0(A1,D0.W)       
015CC8  4E75                      RTS                           
015CCA  101B                      MOVE.B    (A3)+,D0            
015CCC  E140                      ASL.W     #8,D0               
015CCE  101B                      MOVE.B    (A3)+,D0            
015CD0  121B                      MOVE.B    (A3)+,D1            
015CD2  E141                      ASL.W     #8,D1               
015CD4  121B                      MOVE.B    (A3)+,D1            
015CD6  32361000                  MOVE.W    0(A6,D1.W),D1       
015CDA  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015CE0  22701000                  MOVEA.L   0(A0,D1.W),A1       
015CE4  C34E                      EXG       A6,A1               
015CE6  6100F6EE                  BSR       -2322(PC)            L0557
015CEA  C34E                      EXG       A6,A1               
015CEC  3E1C                      MOVE.W    (A4)+,D7            
015CEE  33870000                  MOVE.W    D7,0(A1,D0.W)       
015CF2  4E75                      RTS                           
015CF4  101B                      MOVE.B    (A3)+,D0            
015CF6  E140                      ASL.W     #8,D0               
015CF8  101B                      MOVE.B    (A3)+,D0            
015CFA  121B                      MOVE.B    (A3)+,D1            
015CFC  E141                      ASL.W     #8,D1               
015CFE  121B                      MOVE.B    (A3)+,D1            
015D00  32361000                  MOVE.W    0(A6,D1.W),D1       
015D04  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015D0A  22701000                  MOVEA.L   0(A0,D1.W),A1       
015D0E  C34E                      EXG       A6,A1               
015D10  6100F6E4                  BSR       -2332(PC)            L0559
015D14  C34E                      EXG       A6,A1               
015D16  43F10000                  LEA       0(A1,D0.W),A1       
015D1A  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015D20  12D8                __L05A7:
                                  MOVE.B    (A0)+,(A1)+         
015D22  6600FFFC                  BNE       -4(PC)               __L05A7
015D26  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x1c -  offset 0x4d5a -  address $015d28
015D28  3907                      MOVE.W    D7,-(A4)            
015D2A  4E75                      RTS   


; =============================================================================
; STORENAME 0x38
; ahimw                        
015D2C  3907                      MOVE.W    D7,-(A4)            
015D2E  6100FD86                  BSR       -634(PC)             FN_READ_OPNAME_LOOP
015D32  41F900010F92              LEA       $00010F92,A0         STORE_JUMP_TABLE
015D38  101B                      MOVE.B    (A3)+,D0            
015D3A  4880                      EXT.W     D0                  
015D3C  30300000                  MOVE.W    0(A0,D0.W),D0       
015D40  4EF00000                  JMP       0(A0,D0.W)          


; =============================================================================
; ADD_JUMP_TABLE 0x3 -  offset 0x4d76 -  address $015d44
015D44  101B                      MOVE.B    (A3)+,D0            
015D46  E140                      ASL.W     #8,D0               
015D48  101B                      MOVE.B    (A3)+,D0            
015D4A  DF360000                  ADD.B     D7,0(A6,D0.W)       
015D4E  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x4 -  offset 0x4d82 -  address $015d50
015D50  101B                      MOVE.B    (A3)+,D0            
015D52  E140                      ASL.W     #8,D0               
015D54  101B                      MOVE.B    (A3)+,D0            
015D56  DF760000                  ADD.W     D7,0(A6,D0.W)       
015D5A  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x5 -  offset 0x4d8e -  address $015d5c
015D5C  101B                      MOVE.B    (A3)+,D0            
015D5E  E140                      ASL.W     #8,D0               
015D60  101B                      MOVE.B    (A3)+,D0            
015D62  43F60000                  LEA       0(A6,D0.W),A1       
015D66  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015D6C  4A19                __L05A8:
                                  TST.B     (A1)+               
015D6E  6600FFFC                  BNE       -4(PC)               __L05A8
015D72  5389                      SUBQ.L    #1,A1               
015D74  12D8                __L05A9:
                                  MOVE.B    (A0)+,(A1)+         
015D76  6600FFFC                  BNE       -4(PC)               __L05A9
015D7A  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x7 -  offset 0x4dae -  address $015d7c
015D7C  101B                      MOVE.B    (A3)+,D0            
015D7E  E140                      ASL.W     #8,D0               
015D80  101B                      MOVE.B    (A3)+,D0            
015D82  6100F616                  BSR       -2538(PC)            L0555
015D86  3E1C                      MOVE.W    (A4)+,D7            
015D88  DF360000                  ADD.B     D7,0(A6,D0.W)       
015D8C  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x8 -  offset 0x4dc0 -  address $015d8e
015D8E  101B                      MOVE.B    (A3)+,D0            
015D90  E140                      ASL.W     #8,D0               
015D92  101B                      MOVE.B    (A3)+,D0            
015D94  6100F640                  BSR       -2496(PC)            L0557
015D98  3E1C                      MOVE.W    (A4)+,D7            
015D9A  DF760000                  ADD.W     D7,0(A6,D0.W)       
015D9E  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x6 -  offset 0x4dd2 -  address $015da0
015DA0  101B                      MOVE.B    (A3)+,D0            
015DA2  E140                      ASL.W     #8,D0               
015DA4  101B                      MOVE.B    (A3)+,D0            
015DA6  6100F64E                  BSR       -2482(PC)            L0559
015DAA  43F60000                  LEA       0(A6,D0.W),A1       
015DAE  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015DB4  4A19                __L05AA:
                                  TST.B     (A1)+               
015DB6  6600FFFC                  BNE       -4(PC)               __L05AA
015DBA  5389                      SUBQ.L    #1,A1               
015DBC  12D8                __L05AB:
                                  MOVE.B    (A0)+,(A1)+         
015DBE  6600FFFC                  BNE       -4(PC)               __L05AB
015DC2  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x9 -  offset 0x4df6 -  address $015dc4
015DC4  4240                      CLR.W     D0                  
015DC6  101B                      MOVE.B    (A3)+,D0            
015DC8  DF360000                  ADD.B     D7,0(A6,D0.W)       
015DCC  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xa -  offset 0x4e00 -  address $015dce
015DCE  4240                      CLR.W     D0                  
015DD0  101B                      MOVE.B    (A3)+,D0            
015DD2  DF760000                  ADD.W     D7,0(A6,D0.W)       
015DD6  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xb -  offset 0x4e0a -  address $015dd8
015DD8  4240                      CLR.W     D0                  
015DDA  101B                      MOVE.B    (A3)+,D0            
015DDC  43F60000                  LEA       0(A6,D0.W),A1       
015DE0  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015DE6  4A19                __L05AC:
                                  TST.B     (A1)+               
015DE8  6600FFFC                  BNE       -4(PC)               __L05AC
015DEC  5389                      SUBQ.L    #1,A1               
015DEE  12D8                __L05AD:
                                  MOVE.B    (A0)+,(A1)+         
015DF0  6600FFFC                  BNE       -4(PC)               __L05AD
015DF4  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xd -  offset 0x4e28 -  address $015df6
015DF6  4240                      CLR.W     D0                  
015DF8  101B                      MOVE.B    (A3)+,D0            
015DFA  6100F59E                  BSR       -2658(PC)            L0555
015DFE  3E1C                      MOVE.W    (A4)+,D7            
015E00  DF360000                  ADD.B     D7,0(A6,D0.W)       
015E04  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xe -  offset 0x4e38 -  address $015e06
015E06  4240                      CLR.W     D0                  
015E08  101B                      MOVE.B    (A3)+,D0            
015E0A  6100F5CA                  BSR       -2614(PC)            L0557
015E0E  3E1C                      MOVE.W    (A4)+,D7            
015E10  DF760000                  ADD.W     D7,0(A6,D0.W)       
015E14  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xc -  offset 0x4e48 -  address $015e16
015E16  4240                      CLR.W     D0                  
015E18  101B                      MOVE.B    (A3)+,D0            
015E1A  6100F5DA                  BSR       -2598(PC)            L0559
015E1E  43F60000                  LEA       0(A6,D0.W),A1       
015E22  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015E28  4A19                __L05AE:
                                  TST.B     (A1)+               
015E2A  6600FFFC                  BNE       -4(PC)               __L05AE
015E2E  5389                      SUBQ.L    #1,A1               
015E30  12D8                __L05AF:
                                  MOVE.B    (A0)+,(A1)+         
015E32  6600FFFC                  BNE       -4(PC)               __L05AF
015E36  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0xf -  offset 0x4e6a -  address $015e38
015E38  101B                      MOVE.B    (A3)+,D0            
015E3A  E140                      ASL.W     #8,D0               
015E3C  101B                      MOVE.B    (A3)+,D0            
015E3E  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015E44  DF310000                  ADD.B     D7,0(A1,D0.W)       
015E48  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x10 -  offset 0x4e7c -  address $015e4a
015E4A  101B                      MOVE.B    (A3)+,D0            
015E4C  E140                      ASL.W     #8,D0               
015E4E  101B                      MOVE.B    (A3)+,D0            
015E50  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015E56  DF710000                  ADD.W     D7,0(A1,D0.W)       
015E5A  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x11 -  offset 0x4e8e -  address $015e5c
015E5C  101B                      MOVE.B    (A3)+,D0            
015E5E  E140                      ASL.W     #8,D0               
015E60  101B                      MOVE.B    (A3)+,D0            
015E62  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015E68  43F10000                  LEA       0(A1,D0.W),A1       
015E6C  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015E72  4A19                __L05B0:
                                  TST.B     (A1)+               
015E74  6600FFFC                  BNE       -4(PC)               __L05B0
015E78  5389                      SUBQ.L    #1,A1               
015E7A  12D8                __L05B1:
                                  MOVE.B    (A0)+,(A1)+         
015E7C  6600FFFC                  BNE       -4(PC)               __L05B1
015E80  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x13 -  offset 0x4eb4 -  address $015e82
015E82  101B                      MOVE.B    (A3)+,D0            
015E84  E140                      ASL.W     #8,D0               
015E86  101B                      MOVE.B    (A3)+,D0            
015E88  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015E8E  C34E                      EXG       A6,A1               
015E90  6100F508                  BSR       -2808(PC)            L0555
015E94  C34E                      EXG       A6,A1               
015E96  3E1C                      MOVE.W    (A4)+,D7            
015E98  DF310000                  ADD.B     D7,0(A1,D0.W)       
015E9C  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x14 -  offset 0x4ed0 -  address $015e9e


; =============================================================================
; ADD_JUMP_TABLE 0x15 -  offset 0x4ed0 -  address $015e9e
015E9E  101B                      MOVE.B    (A3)+,D0            
015EA0  E140                      ASL.W     #8,D0               
015EA2  101B                      MOVE.B    (A3)+,D0            
015EA4  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015EAA  C34E                      EXG       A6,A1               
015EAC  6100F528                  BSR       -2776(PC)            L0557
015EB0  C34E                      EXG       A6,A1               
015EB2  3E1C                      MOVE.W    (A4)+,D7            
015EB4  DF710000                  ADD.W     D7,0(A1,D0.W)       
015EB8  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x12 -  offset 0x4eec -  address $015eba
015EBA  101B                      MOVE.B    (A3)+,D0            
015EBC  E140                      ASL.W     #8,D0               
015EBE  101B                      MOVE.B    (A3)+,D0            
015EC0  2279000170E0              MOVEA.L   $000170E0,A1         L_L0692
015EC6  C34E                      EXG       A6,A1               
015EC8  6100F52C                  BSR       -2772(PC)            L0559
015ECC  C34E                      EXG       A6,A1               
015ECE  43F10000                  LEA       0(A1,D0.W),A1       
015ED2  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015ED8  4A19                __L05B2:
                                  TST.B     (A1)+               
015EDA  6600FFFC                  BNE       -4(PC)               __L05B2
015EDE  5389                      SUBQ.L    #1,A1               
015EE0  12D8                __L05B3:
                                  MOVE.B    (A0)+,(A1)+         
015EE2  6600FFFC                  BNE       -4(PC)               __L05B3
015EE6  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x16 -  offset 0x4f1a -  address $015ee8
015EE8  101B                      MOVE.B    (A3)+,D0            
015EEA  E140                      ASL.W     #8,D0               
015EEC  101B                      MOVE.B    (A3)+,D0            
015EEE  30360000                  MOVE.W    0(A6,D0.W),D0       
015EF2  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015EF8  22700000                  MOVEA.L   0(A0,D0.W),A1       
015EFC  101B                      MOVE.B    (A3)+,D0            
015EFE  E140                      ASL.W     #8,D0               
015F00  101B                      MOVE.B    (A3)+,D0            
015F02  DF310000                  ADD.B     D7,0(A1,D0.W)       
015F06  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x17 -  offset 0x4f3a -  address $015f08
015F08  101B                      MOVE.B    (A3)+,D0            
015F0A  E140                      ASL.W     #8,D0               
015F0C  101B                      MOVE.B    (A3)+,D0            
015F0E  30360000                  MOVE.W    0(A6,D0.W),D0       
015F12  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015F18  22700000                  MOVEA.L   0(A0,D0.W),A1       
015F1C  101B                      MOVE.B    (A3)+,D0            
015F1E  E140                      ASL.W     #8,D0               
015F20  101B                      MOVE.B    (A3)+,D0            
015F22  DF710000                  ADD.W     D7,0(A1,D0.W)       
015F26  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x18 -  offset 0x4f5a -  address $015f28
015F28  101B                      MOVE.B    (A3)+,D0            
015F2A  E140                      ASL.W     #8,D0               
015F2C  101B                      MOVE.B    (A3)+,D0            
015F2E  30360000                  MOVE.W    0(A6,D0.W),D0       
015F32  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015F38  22700000                  MOVEA.L   0(A0,D0.W),A1       
015F3C  101B                      MOVE.B    (A3)+,D0            
015F3E  E140                      ASL.W     #8,D0               
015F40  101B                      MOVE.B    (A3)+,D0            
015F42  43F10000                  LEA       0(A1,D0.W),A1       
015F46  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015F4C  4A19                __L05B4:
                                  TST.B     (A1)+               
015F4E  6600FFFC                  BNE       -4(PC)               __L05B4
015F52  5389                      SUBQ.L    #1,A1               
015F54  12D8                __L05B5:
                                  MOVE.B    (A0)+,(A1)+         
015F56  6600FFFC                  BNE       -4(PC)               __L05B5
015F5A  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x1a -  offset 0x4f8e -  address $015f5c
015F5C  101B                      MOVE.B    (A3)+,D0            
015F5E  E140                      ASL.W     #8,D0               
015F60  101B                      MOVE.B    (A3)+,D0            
015F62  121B                      MOVE.B    (A3)+,D1            
015F64  E141                      ASL.W     #8,D1               
015F66  121B                      MOVE.B    (A3)+,D1            
015F68  32361000                  MOVE.W    0(A6,D1.W),D1       
015F6C  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015F72  22701000                  MOVEA.L   0(A0,D1.W),A1       
015F76  C34E                      EXG       A6,A1               
015F78  6100F420                  BSR       -3040(PC)            L0555
015F7C  C34E                      EXG       A6,A1               
015F7E  3E1C                      MOVE.W    (A4)+,D7            
015F80  DF310000                  ADD.B     D7,0(A1,D0.W)       
015F84  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x1b -  offset 0x4fb8 -  address $015f86
015F86  101B                      MOVE.B    (A3)+,D0            
015F88  E140                      ASL.W     #8,D0               
015F8A  101B                      MOVE.B    (A3)+,D0            
015F8C  121B                      MOVE.B    (A3)+,D1            
015F8E  E141                      ASL.W     #8,D1               
015F90  121B                      MOVE.B    (A3)+,D1            
015F92  32361000                  MOVE.W    0(A6,D1.W),D1       
015F96  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015F9C  22701000                  MOVEA.L   0(A0,D1.W),A1       
015FA0  C34E                      EXG       A6,A1               
015FA2  6100F432                  BSR       -3022(PC)            L0557
015FA6  C34E                      EXG       A6,A1               
015FA8  3E1C                      MOVE.W    (A4)+,D7            
015FAA  DF710000                  ADD.W     D7,0(A1,D0.W)       
015FAE  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x19 -  offset 0x4fe2 -  address $015fb0
015FB0  101B                      MOVE.B    (A3)+,D0            
015FB2  E140                      ASL.W     #8,D0               
015FB4  101B                      MOVE.B    (A3)+,D0            
015FB6  121B                      MOVE.B    (A3)+,D1            
015FB8  E141                      ASL.W     #8,D1               
015FBA  121B                      MOVE.B    (A3)+,D1            
015FBC  32361000                  MOVE.W    0(A6,D1.W),D1       
015FC0  2079000170A8              MOVEA.L   $000170A8,A0         L067A
015FC6  22701000                  MOVEA.L   0(A0,D1.W),A1       
015FCA  C34E                      EXG       A6,A1               
015FCC  6100F428                  BSR       -3032(PC)            L0559
015FD0  C34E                      EXG       A6,A1               
015FD2  43F10000                  LEA       0(A1,D0.W),A1       
015FD6  20790001714E              MOVEA.L   $0001714E,A0         L06C4
015FDC  4A19                __L05B6:
                                  TST.B     (A1)+               
015FDE  6600FFFC                  BNE       -4(PC)               __L05B6
015FE2  5389                      SUBQ.L    #1,A1               
015FE4  12D8                __L05B7:
                                  MOVE.B    (A0)+,(A1)+         
015FE6  6600FFFC                  BNE       -4(PC)               __L05B7
015FEA  4E75                      RTS                           


; =============================================================================
; ADD_JUMP_TABLE 0x1d -  offset 0x501e -  address $015fec
015FEC  3907                      MOVE.W    D7,-(A4)            
015FEE  6100FAC6                  BSR       -1338(PC)            FN_READ_OPNAME_LOOP
015FF2  41F900010FCE              LEA       $00010FCE,A0         ADD_JUMP_TABLE
015FF8  101B                      MOVE.B    (A3)+,D0            
015FFA  4880                      EXT.W     D0                  
015FFC  30300000                  MOVE.W    0(A0,D0.W),D0       
016000  4EF00000                  JMP       0(A0,D0.W)          



016004  4EB90000B7C2        FN_INIT_3:
	                              JSR       $0000B7C2            __L006B
01600A  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
01600E  6100DF74                  BSR       -8332(PC)            L04CE
016012  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016016  4242                      CLR.W     D2                  
016018  61000284                  BSR       644(PC)              FN_READ_FILE_NAME
01601C  4E75                      RTS                           
01601E  48E7FFFE            __L05B9:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016022  4279000171BC              CLR.W     $000171BC            L06EF
016028  4279000171C0              CLR.W     $000171C0            L06F1
01602E  4279000171BE              CLR.W     $000171BE            L06F0
016034  4EB90000F19C              JSR       $0000F19C            L027A
01603A  4239000171AA              CLR.B     $000171AA            L06EA
016040  4EB90000F0C4              JSR       $0000F0C4            L0271
016046  4239000171DA              CLR.B     $000171DA            L06F5
01604C  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016050  207C0000AF52              MOVEA.L   #$0000AF52,A0        BUF_PATH
016056  22790001714A              MOVEA.L   $0001714A,A1         ADDR_CURRENT_PATH
01605C  12D8                __L05BA:
                                  MOVE.B    (A0)+,(A1)+         
01605E  6600FFFC                  BNE       -4(PC)               __L05BA
016062  207900017146              MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
016068  13790000AC7CFFFF          MOVE.B    $0000AC7C,-1(A1)     L0018
016070  4A390001707C              TST.B     $0001707C            B_IS_AMIGA
016076  67000004                  BEQ       4(PC)                __L05BB
01607A  5349                      SUBQ.W    #1,A1               
01607C  4A10                __L05BB:
                                  TST.B     (A0)                
01607E  6700001A                  BEQ       26(PC)               L05BD
016082  12D8                __L05BC:
                                  MOVE.B    (A0)+,(A1)+         
016084  6600FFFC                  BNE       -4(PC)               __L05BC
016088  4A390001707C              TST.B     $0001707C            B_IS_AMIGA
01608E  6600000A                  BNE       10(PC)               L05BD
016092  13790000AC7CFFFF          MOVE.B    $0000AC7C,-1(A1)     L0018
01609A  4211                L05BD:CLR.B     (A1)                
01609C  20790001714A              MOVEA.L   $0001714A,A0         ADDR_CURRENT_PATH
0160A2  4EB90000C12C              JSR       $0000C12C            SYS_SET_CURRENT_PATH
0160A8  207C000160B6              MOVEA.L   #$000160B6,A0        STR_MAINAO_LOWERCASE
0160AE  4242                      CLR.W     D2                  
0160B0  610001EC                  BSR       492(PC)              FN_READ_FILE_NAME
0160B4  4E75                      RTS                           

; =============================================================================      
0160B6  6D61696E2E616F00    STR_MAINAO_LOWERCASE:
                                  DC.B      $6D,$61,$69,$6E,$2E,$61,$6F,$00 
0160BE  0000                      DC.B      $00,$00 


; =============================================================================      
0160C0  7206                __L05BF:
                                  MOVEQ     #6,D1               
0160C2  43F900017484              LEA       $00017484,A1         L071C
0160C8  303C0002                  MOVE.W    #2,D0               
0160CC  61000A84                  BSR       2692(PC)             L0629
0160D0  61000BCA                  BSR       3018(PC)             FN_DISK_READ_BYTES
0160D4  7210                __L05C0:
                                  MOVEQ     #$10,D1             
0160D6  43F90001748A              LEA       $0001748A,A1         L071D
0160DC  61000BBE                  BSR       3006(PC)             FN_DISK_READ_BYTES
0160E0  30390001748A              MOVE.W    $0001748A,D0         L071D
0160E6  33C0000170A0              MOVE.W    D0,$000170A0         L0677
0160EC  227900017062              MOVEA.L   $00017062,A1         L065C
0160F2  23C900017094              MOVE.L    A1,$00017094         L0674
0160F8  23C9000170A4              MOVE.L    A1,$000170A4         L0679
0160FE  C0FC0004                  MULU      #4,D0               
016102  D089                      ADD.L     A1,D0               
016104  4279000170A2              CLR.W     $000170A2            L0678
01610A  42390001712D              CLR.B     $0001712D            L06BA
016110  42390001718A              CLR.B     $0001718A            L06DB
016116  42B900017104              CLR.L     $00017104            W_06A5
01611C  4279000170FA              CLR.W     $000170FA            L069D
016122  4239000170CE              CLR.B     $000170CE            L0687
016128  13FC00FF000170CF          MOVE.B    #$FF,$000170CF       L0688
016130  33FCFFFF00017072          MOVE.W    #$FFFF,$00017072     L0664
016138  42390001720A              CLR.B     $0001720A            L0700
01613E  23C0000170A8              MOVE.L    D0,$000170A8         L067A
016144  30390001748C              MOVE.W    $0001748C,D0         L071E
01614A  33C0000170B4              MOVE.W    D0,$000170B4         L067D
016150  C0FC0006                  MULU      #6,D0               
016154  D0B9000170A8              ADD.L     $000170A8,D0         L067A
01615A  23C0000170AC              MOVE.L    D0,$000170AC         L067B
016160  23C0000170B0              MOVE.L    D0,$000170B0         L067C
016166  D0B900017492              ADD.L     $00017492,D0         L0720
01616C  0000000F                  ORI.B     #$F,D0              
016170  5280                      ADDQ.L    #1,D0               
016172  23C0000170BA              MOVE.L    D0,$000170BA         L0680
016178  223900017496              MOVE.L    $00017496,D1         L0721
01617E  068100000003              ADDI.L    #3,D1               
016184  C2FC0028                  MULU      #$28,D1             
016188  D081                      ADD.L     D1,D0               
01618A  23C0000170BE              MOVE.L    D0,$000170BE         L0681
016190  23C000017098              MOVE.L    D0,$00017098         L0675
016196  23C00001709C              MOVE.L    D0,$0001709C         L0676
01619C  D0B90001748E              ADD.L     $0001748E,D0         L071F
0161A2  B0B900017066              CMP.L     $00017066,D0         L065D
0161A8  62000008                  BHI       8(PC)                __L05C1
0161AC  23C000017066              MOVE.L    D0,$00017066         L065D
0161B2  6100069A            __L05C1:
                                  BSR       1690(PC)             L0612
0161B6  61000054                  BSR       84(PC)               __L05C4
0161BA  22790001709C              MOVEA.L   $0001709C,A1         L0676
0161C0  223900017484              MOVE.L    $00017484,D1         L071C
0161C6  028100FFFFFF              ANDI.L    #$FFFFFF,D1         
0161CC  13F90001748400017080      MOVE.B    $00017484,$00017080  L071C L066C
0161D6  6A000010                  BPL       16(PC)               __L05C2
0161DA  048100000016              SUBI.L    #$16,D1             
0161E0  610001AE                  BSR       430(PC)              L05D0
0161E4  6000000C                  BRA       12(PC)               _close_file_after_depack_ok
0161E8  048100000016        __L05C2:
                                  SUBI.L    #$16,D1             
0161EE  61000AAC                  BSR       2732(PC)             FN_DISK_READ_BYTES
0161F2  61000A06            _close_file_after_depack_ok:
                                  BSR       2566(PC)             FN_DISK_CLOSE_FILE ; depack success, close pak file
0161F6  61000694                  BSR       1684(PC)             L0614
0161FA  4241                      CLR.W     D1                  
0161FC  6100079A                  BSR       1946(PC)             __L061E
016200  4268FFEA                  CLR.W     -22(A0)             
016204  23C8000170E0              MOVE.L    A0,$000170E0         L_L0692
01620A  4E75                      RTS                           


01620C  48E780C0            __L05C4:
                                  MOVEM.L   A0-A1/D0,-(A7)      
016210  2079000170BA              MOVEA.L   $000170BA,A0         L0680
016216  D1FC00008000              ADDA.L    #$8000,A0           
01621C  23C8000170C2              MOVE.L    A0,$000170C2         L_L0682
016222  303C8000                  MOVE.W    #$8000,D0           
016226  33C0000170C8              MOVE.W    D0,$000170C8         L0684
01622C  42B0000C                  CLR.L     12(A0,D0.W)         
016230  31BC013F0016              MOVE.W    #$13F,22(A0,D0.W)   
016236  31BC00C70018              MOVE.W    #$C7,24(A0,D0.W)    
01623C  06400028                  ADDI.W    #$28,D0             
016240  42300001                  CLR.B     1(A0,D0.W)          
016244  33C00001720E              MOVE.W    D0,$0001720E         L0702
01624A  06400028                  ADDI.W    #$28,D0             
01624E  33C0000170CC              MOVE.W    D0,$000170CC         L0686
016254  42B0000C                  CLR.L     12(A0,D0.W)         
016258  11BC00FE0000              MOVE.B    #$FE,0(A0,D0.W)     
01625E  4239000170CE              CLR.B     $000170CE            L0687
016264  06400028                  ADDI.W    #$28,D0             
016268  33C0000170C6              MOVE.W    D0,$000170C6         W_L0683
01626E  43F00000                  LEA       0(A0,D0.W),A1       
016272  06400028            L05C5:ADDI.W    #$28,D0             
016276  6700FFFA                  BEQ       -6(PC)               L05C5
01627A  B07C7FFF                  CMP.W     #$7FFF,D0           
01627E  6E000014                  BGT       20(PC)               __L05C6
016282  33400004                  MOVE.W    D0,4(A1)            
016286  43F00000                  LEA       0(A0,D0.W),A1       
01628A  B3F9000170BE              CMPA.L    $000170BE,A1         L0681
016290  6D00FFE0                  BLT       -32(PC)              L05C5
016294  4269FFDC            __L05C6:
                                  CLR.W     -36(A1)             
016298  4CDF0301                  MOVEM.L   (A7)+,A0-A1/D0      
01629C  4E75                      RTS                           


; =============================================================================
; A0 pointe sur nom fichier
01629E  123C0041            FN_READ_FILE_NAME:
                                  MOVE.B    #$41,D1             ;(#"A") ;(#"A") ; first char of file extension ???
0162A2  4A390001707C              TST.B     $0001707C            B_IS_AMIGA
0162A8  67000006                  BEQ       6(PC)                __not_amiga_1
0162AC  123C0043                  MOVE.B    #$43,D1             ;(#"C") 
0162B0  48E70060            __not_amiga_1:
                                  MOVEM.L   A1-A2,-(A7)         
0162B4  227900017146              MOVEA.L   $00017146,A1         ADDR_SCRIPT_NAME ; file name buffer address
0162BA  247C00000000              MOVEA.L   #0,A2               
0162C0  1018                __L05C9:
                                  MOVE.B    (A0)+,D0            
0162C2  12C0                      MOVE.B    D0,(A1)+            
0162C4  B03C002E                  CMP.B     #$2E,D0             ;(#".") 
0162C8  66000004                  BNE       4(PC)                __not_separator
0162CC  2449                      MOVEA.L   A1,A2               
0162CE  4A00                __not_separator:
                                  TST.B     D0                  
0162D0  6600FFEE                  BNE       -18(PC)              __L05C9
0162D4  B5FC00000000              CMPA.L    #0,A2               
0162DA  67000004                  BEQ       4(PC)                __L05CB
0162DE  1481                      MOVE.B    D1,(A2)             
0162E0  207900017146        __L05CB:
                                  MOVEA.L   $00017146,A0         ADDR_SCRIPT_NAME
0162E6  4CDF0600                  MOVEM.L   (A7)+,A1-A2         
0162EA  4A42                      TST.W     D2                  
0162EC  6700FDD2                  BEQ       -558(PC)             __L05BF
0162F0  61000514                  BSR       1300(PC)             L060D
0162F4  66000004                  BNE       4(PC)                __id_script_notzero
0162F8  4E75                      RTS                           
0162FA  33C200017176        __id_script_notzero:
                                  MOVE.W    D2,$00017176         W_DEPACKED_FILE_ID
016300  7206                      MOVEQ     #6,D1               
016302  43F900017484              LEA       $00017484,A1         L071C
016308  303C0002                  MOVE.W    #2,D0               
01630C  61000844                  BSR       2116(PC)             L0629
016310  6100098A                  BSR       2442(PC)             FN_DISK_READ_BYTES
016314  43F900017484              LEA       $00017484,A1         L071C
01631A  4A690004                  TST.W     4(A1)               
01631E  6700FDB4                  BEQ       -588(PC)             __L05C0
016322  22790001709C              MOVEA.L   $0001709C,A1         L0676
016328  223900017484              MOVE.L    $00017484,D1         L071C
01632E  028100FFFFFF              ANDI.L    #$FFFFFF,D1         
016334  13F90001748400017080      MOVE.B    $00017484,$00017080  L071C L066C
01633E  6A000010                  BPL       16(PC)               __L05CD
016342  048100000006              SUBI.L    #6,D1               
016348  61000046                  BSR       70(PC)               L05D0
01634C  6000000C                  BRA       12(PC)               __L05CE
016350  048100000006        __L05CD:
                                  SUBI.L    #6,D1               
016356  61000944                  BSR       2372(PC)             FN_DISK_READ_BYTES
01635A  6100089E            __L05CE:
                                  BSR       2206(PC)             FN_DISK_CLOSE_FILE
01635E  61000460                  BSR       1120(PC)             __L0608
016362  34290000                  MOVE.W    0(A1),D2            
016366  B47900017176              CMP.W     $00017176,D2         W_DEPACKED_FILE_ID
01636C  6700001A                  BEQ       26(PC)               _depack_file_id_equal
016370  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016374  4EB90000AC2C              JSR       $0000AC2C            __L0003
01637A  6100048A                  BSR       1162(PC)             L060D
01637E  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016382  66000004                  BNE       4(PC)                _depack_file_id_equal
016386  4E75                      RTS                           
016388  61000502            _depack_file_id_equal:
                                  BSR       1282(PC)             L0614
01638C  4E75                      RTS                           
01638E  4E71                      NOP                           
016390  48E7FFFE            L05D0:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016394  13FC0001000170FE          MOVE.B    #1,$000170FE         L06A0
01639C  103900017080              MOVE.B    $00017080,D0         L066C
0163A2  020000F0                  ANDI.B    #$F0,D0             
0163A6  B03C00A0                  CMP.B     #$A0,D0              ; check if file is packed (from 1st byte)
0163AA  6600000A                  BNE       10(PC)               __L05D1 ; not packed
0163AE  610000CA                  BSR       202(PC)              FN_DEPACK_FILE
0163B2  60000074                  BRA       116(PC)              L05D7
0163B6  2C01                __L05D1:
                                  MOVE.L    D1,D6               
0163B8  2A49                      MOVEA.L   A1,A5               
0163BA  DC89                      ADD.L     A1,D6               
0163BC  2C46                      MOVEA.L   D6,A6               
0163BE  28790000AC5E              MOVEA.L   $0000AC5E,A4         L000E
0163C4  D9FC00007D00              ADDA.L    #$7D00,A4           
0163CA  264C                      MOVEA.L   A4,A3               
0163CC  33FC00010001707E          MOVE.W    #1,$0001707E         L066B
0163D4  0839000600017080          BTST      #6,$00017080         L066C
0163DC  6700000A                  BEQ       10(PC)               __L05D2
0163E0  33FC00080001707E          MOVE.W    #8,$0001707E         L066B
0163E8  36390001707E        __L05D2:
                                  MOVE.W    $0001707E,D3         L066B
0163EE  4A43                L05D3:TST.W     D3                  
0163F0  6B000036                  BMI       54(PC)               L05D7
0163F4  61000038                  BSR       56(PC)               L05D8
0163F8  1800                      MOVE.B    D0,D4               
0163FA  6B000016                  BMI       22(PC)               __L05D5
0163FE  4884                      EXT.W     D4                  
016400  5344                      SUBQ.W    #1,D4               
016402  6100002A            __L05D4:
                                  BSR       42(PC)               L05D8
016406  6100004A                  BSR       74(PC)               __L05DB
01640A  51CCFFF6                  DBF       D4,-10(PC)           __L05D4
01640E  6000FFDE                  BRA       -34(PC)              L05D3
016412  0244007F            __L05D5:
                                  ANDI.W    #$7F,D4             
016416  5344                      SUBQ.W    #1,D4               
016418  61000014                  BSR       20(PC)               L05D8
01641C  61000034            __L05D6:
                                  BSR       52(PC)               L05DB
016420  51CCFFFA                  DBF       D4,-6(PC)            __L05D6
016424  6000FFC8                  BRA       -56(PC)              L05D3
016428  4CDF7FFF            L05D7:MOVEM.L   (A7)+,A0-A6/D0-D7   
01642C  4E75                      RTS                           
01642E  B7CC                L05D8:CMPA.L    A4,A3               
016430  6C000006                  BGE       6(PC)                __L05DA
016434  101B                __L05D9:
                                  MOVE.B    (A3)+,D0            
016436  4E75                      RTS                           
016438  22790000AC5E        __L05DA:
                                  MOVEA.L   $0000AC5E,A1         L000E
01643E  223C00007D00              MOVE.L    #$7D00,D1           
016444  61000856                  BSR       2134(PC)             FN_DISK_READ_BYTES
016448  26790000AC5E              MOVEA.L   $0000AC5E,A3         L000E
01644E  6000FFE4                  BRA       -28(PC)              __L05D9
016452  BDC6                __L05DB:
                                  CMPA.L    D6,A6               
016454  6C00000C                  BGE       12(PC)               __L05DD
016458  1C80                __L05DC:
                                  MOVE.B    D0,(A6)             
01645A  DCF90001707E              ADDA.W    $0001707E,A6         L066B
016460  4E75                      RTS                           
016462  2C4D                __L05DD:
                                  MOVEA.L   A5,A6               
016464  DCF90001707E              ADDA.W    $0001707E,A6         L066B
01646A  9CC3                      SUBA.W    D3,A6               
01646C  04430001                  SUBI.W    #1,D3               
016470  6A00FFE6                  BPL       -26(PC)              __L05DC
016474  4244                      CLR.W     D4                  
016476  4E75                      RTS                           


; =============================================================================
; FN_DEPACK_FILE
; reads packed data from disk, in 32k byte chunks
; then proceeds to depack into (a1) 
016478  4E75                __depack_ended:
                                  RTS                           
01647A  2449                FN_DEPACK_FILE:
                                  MOVEA.L   A1,A2               
01647C  D5C1                      ADDA.L    D1,A2               
01647E  538A                      SUBQ.L    #1,A2               
016480  28790000AC5E              MOVEA.L   $0000AC5E,A4         L000E
016486  D9FC00007D00              ADDA.L    #$7D00,A4           
01648C  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016490  22790000AC5E              MOVEA.L   $0000AC5E,A1         L000E
016496  223C00007D00              MOVE.L    #$7D00,D1           
01649C  610007FE                  BSR       2046(PC)             FN_DISK_READ_BYTES
0164A0  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
0164A4  20790000AC5E              MOVEA.L   $0000AC5E,A0         L000E
0164AA  2A7C0001651E              MOVEA.L   #$0001651E,A5        ADDR_DEPACK_DIC
0164B0  2A98                      MOVE.L    (A0)+,(A5)          
0164B2  2B580004                  MOVE.L    (A0)+,4(A5)         
0164B6  4247                      CLR.W     D7                  
0164B8  B3CA                __L05E0:
                                  CMPA.L    A2,A1               
0164BA  6E00FFBC                  BGT       -68(PC)              __depack_ended
0164BE  7001                      MOVEQ     #1,D0               
0164C0  6166                      BSR.S     102(PC)              L05EA
0164C2  4A05                      TST.B     D5                  
0164C4  671E                      BEQ.S     30(PC)               __L05E3
0164C6  7400                      MOVEQ     #0,D2               
0164C8  7002                __L05E1:
                                  MOVEQ     #2,D0               
0164CA  615C                      BSR.S     92(PC)               L05EA
0164CC  D445                      ADD.W     D5,D2               
0164CE  BA7C0003                  CMP.W     #3,D5               
0164D2  67F4                      BEQ.S     -12(PC)              __L05E1
0164D4  7008                __L05E2:
                                  MOVEQ     #8,D0               
0164D6  6150                      BSR.S     80(PC)               L05EA
0164D8  12C5                      MOVE.B    D5,(A1)+            
0164DA  51CAFFF8                  DBF       D2,-8(PC)            __L05E2
0164DE  B3CA                      CMPA.L    A2,A1               
0164E0  6E00FF96                  BGT       -106(PC)             __depack_ended
0164E4  6140                __L05E3:
                                  BSR.S     64(PC)               L05E9
0164E6  4240                      CLR.W     D0                  
0164E8  10355000                  MOVE.B    0(A5,D5.W),D0       
0164EC  02450003                  ANDI.W    #3,D5               
0164F0  67000014                  BEQ       20(PC)               __L05E6
0164F4  3405                      MOVE.W    D5,D2               
0164F6  6130                      BSR.S     48(PC)               L05EA
0164F8  4445                __L05E4:
                                  NEG.W     D5                  
0164FA  12F150FF            __L05E5:
                                  MOVE.B    -1(A1,D5.W),(A1)+   
0164FE  51CAFFFA                  DBF       D2,-6(PC)            __L05E5
016502  6000FFB4                  BRA       -76(PC)              __L05E0
016506  61000020            __L05E6:
                                  BSR       32(PC)               L05EA
01650A  3605                      MOVE.W    D5,D3               
01650C  4242                      CLR.W     D2                  
01650E  6116                __L05E7:
                                  BSR.S     22(PC)               L05E9
016510  D445                      ADD.W     D5,D2               
016512  BA7C0007                  CMP.W     #7,D5               
016516  67F6                      BEQ.S     -10(PC)              __L05E7
016518  3A03                      MOVE.W    D3,D5               
01651A  5842                      ADDQ.W    #4,D2               
01651C  60DA                      BRA.S     -38(PC)              __L05E4
01651E                      ADDR_DEPACK_DIC:
                                  DCB.W     4,0 
016526  7003                L05E9:MOVEQ     #3,D0               
016528  9E00                L05EA:SUB.B     D0,D7               
01652A  6B06                      BMI.S     6(PC)                __L05EB
01652C  4245                      CLR.W     D5                  
01652E  E1BD                      ROL.L     D0,D5               
016530  4E75                      RTS                           
016532  DE00                __L05EB:
                                  ADD.B     D0,D7               
016534  4245                      CLR.W     D5                  
016536  EFBD                      ROL.L     D7,D5               
016538  4845                      SWAP      D5                  
01653A  B1CC                      CMPA.L    A4,A0               
01653C  6C000010                  BGE       16(PC)               __depack_next_32k_chunk
016540  3A18                __L05EC:
                                  MOVE.W    (A0)+,D5            
016542  4845                      SWAP      D5                  
016544  9007                      SUB.B     D7,D0               
016546  7E10                      MOVEQ     #$10,D7             
016548  E1BD                      ROL.L     D0,D5               
01654A  9E00                      SUB.B     D0,D7               
01654C  4E75                      RTS                           
01654E  48E7FFFE            __depack_next_32k_chunk:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016552  22790000AC5E              MOVEA.L   $0000AC5E,A1         L000E
016558  223C00007D00              MOVE.L    #$7D00,D1           
01655E  6100073C                  BSR       1852(PC)             FN_DISK_READ_BYTES
016562  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016566  20790000AC5E              MOVEA.L   $0000AC5E,A0         L000E
01656C  6000FFD2                  BRA       -46(PC)              __L05EC
; END of depack routine
; =============================================================================


; =============================================================================
; OPCODE 0xe2 -  offset 0x584e -  address $016570
016570  6100EB7E                  BSR       -5250(PC)            FN_READ_OPNAME
016574  6100EB6C                  BSR       -5268(PC)            L0548
016578  423900017103              CLR.B     $00017103            L06A4
01657E  4EB9000124C6              JSR       $000124C6            L03D8
016584  4A10                      TST.B     (A0)                
016586  6700000C                  BEQ       12(PC)               L05EE
01658A  0C100002                  CMPI.B    #2,(A0)             
01658E  67000004                  BEQ       4(PC)                L05EE
016592  4E75                      RTS                           
016594  30280004            L05EE:MOVE.W    4(A0),D0            
016598  66000004                  BNE       4(PC)                __L05EF
01659C  4E75                      RTS                           
01659E  30280002            __L05EF:
                                  MOVE.W    2(A0),D0            
0165A2  5240                      ADDQ.W    #1,D0               
0165A4  E248                      LSR.W     #1,D0               
0165A6  32280004                  MOVE.W    4(A0),D1            
0165AA  5241                      ADDQ.W    #1,D1               
0165AC  C2C0                      MULU      D0,D1               
0165AE  226EFFEC                  MOVEA.L   -20(A6),A1          
0165B2  D3E9000E                  ADDA.L    14(A1),A1           
0165B6  3C290004                  MOVE.W    4(A1),D6            
0165BA  D3E90000                  ADDA.L    0(A1),A1            
0165BE  5346                      SUBQ.W    #1,D6               
0165C0  2009                L05F0:MOVE.L    A1,D0               
0165C2  D099                      ADD.L     (A1)+,D0            
0165C4  B1C0                      CMPA.L    D0,A0               
0165C6  6C000006                  BGE       6(PC)                __L05F1
0165CA  93A9FFFC                  SUB.L     D1,-4(A1)           
0165CE  51CEFFF0            __L05F1:
                                  DBF       D6,-16(PC)           L05F0
0165D2  226EFFEC                  MOVEA.L   -20(A6),A1          
0165D6  D3E9000E                  ADDA.L    14(A1),A1           
0165DA  93A90006                  SUB.L     D1,6(A1)            
0165DE  93A9000C                  SUB.L     D1,12(A1)           
0165E2  48E70020                  MOVEM.L   A2,-(A7)            
0165E6  317C000F0002              MOVE.W    #$F,2(A0)           
0165EC  42680004                  CLR.W     4(A0)               
0165F0  41E80006                  LEA       6(A0),A0            
0165F4  2448                      MOVEA.L   A0,A2               
0165F6  D5C1                      ADDA.L    D1,A2               
0165F8  4207                      CLR.B     D7                  
0165FA  4EB900016606              JSR       $00016606            L05F2
016600  4CDF0400                  MOVEM.L   (A7)+,A2            
016604  4E75                      RTS                           
016606  2608                L05F2:MOVE.L    A0,D3               
016608  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
01660C  20390001709C              MOVE.L    $0001709C,D0         L0676
016612  30DA                __L05F3:
                                  MOVE.W    (A2)+,(A0)+         
016614  B5C0                      CMPA.L    D0,A2               
016616  6D00FFFA                  BLT       -6(PC)               __L05F3
01661A  93B90001709C              SUB.L     D1,$0001709C         L0676
016620  4A07                      TST.B     D7                  
016622  67000024                  BEQ       36(PC)               __L05F5
016626  2049                      MOVEA.L   A1,A0               
016628  5888                      ADDQ.L    #4,A0               
01662A  2479000170A4              MOVEA.L   $000170A4,A2         L0679
016630  2158FFF8            __L05F4:
                                  MOVE.L    (A0)+,-8(A0)        
016634  B1CA                      CMPA.L    A2,A0               
016636  6D00FFF8                  BLT       -8(PC)               __L05F4
01663A  598A                      SUBQ.L    #4,A2               
01663C  23CA000170A4              MOVE.L    A2,$000170A4         L0679
016642  5379000170A2              SUBQ.W    #1,$000170A2         L0678
016648  2479000170A4        __L05F5:
                                  MOVEA.L   $000170A4,A2         L0679
01664E  207900017094              MOVEA.L   $00017094,A0         L0674
016654  B698                __L05F6:
                                  CMP.L     (A0)+,D3            
016656  6C000006                  BGE       6(PC)                __L05F7
01665A  93A8FFFC                  SUB.L     D1,-4(A0)           
01665E  B1CA                __L05F7:
                                  CMPA.L    A2,A0               
016660  6D00FFF2                  BLT       -14(PC)              __L05F6
016664  4A07                      TST.B     D7                  
016666  67000064                  BEQ       100(PC)              L05FC
01666A  4240                      CLR.W     D0                  
01666C  2079000170A8              MOVEA.L   $000170A8,A0         L067A
016672  30300004            L05F8:MOVE.W    4(A0,D0.W),D0       
016676  67000054                  BEQ       84(PC)               L05FC
01667A  24700000                  MOVEA.L   0(A0,D0.W),A2       
01667E  B46AFFF0                  CMP.W     -16(A2),D2          
016682  66000014                  BNE       20(PC)               __L05F9
016686  48E77FFE                  MOVEM.L   A0-A6/D1-D7,-(A7)   
01668A  61000402                  BSR       1026(PC)             L0622
01668E  3003                      MOVE.W    D3,D0               
016690  4CDF7FFE                  MOVEM.L   (A7)+,A0-A6/D1-D7   
016694  6000FFDC                  BRA       -36(PC)              L05F8
016698  B6AAFFEC            __L05F9:
                                  CMP.L     -20(A2),D3          
01669C  6E00FFD4                  BGT       -44(PC)              L05F8
0166A0  93AAFFEC                  SUB.L     D1,-20(A2)          
0166A4  93AAFFF8                  SUB.L     D1,-8(A2)           
0166A8  226AFFEC                  MOVEA.L   -20(A2),A1          
0166AC  38290016                  MOVE.W    22(A1),D4           
0166B0  4444                      NEG.W     D4                  
0166B2  04440034                  SUBI.W    #$34,D4             
0166B6  B86AFFF6            __L05FA:
                                  CMP.W     -10(A2),D4          
0166BA  6F00000C                  BLE       12(PC)               __L05FB
0166BE  5944                      SUBQ.W    #4,D4               
0166C0  93B24000                  SUB.L     D1,0(A2,D4.W)       
0166C4  6000FFF0                  BRA       -16(PC)              __L05FA
0166C8  6000FFA8            __L05FB:
                                  BRA       -88(PC)              L05F8
0166CC  303900017104        L05FC:MOVE.W    $00017104,D0         W_06A5
0166D2  67000066                  BEQ       102(PC)              __L0601
0166D6  2079000170E0              MOVEA.L   $000170E0,A0         L_L0692
0166DC  2279000170C2              MOVEA.L   $000170C2,A1         L_L0682
0166E2  48E78000            __L05FD:
                                  MOVEM.L   D0,-(A7)            
0166E6  30300002                  MOVE.W    2(A0,D0.W),D0       
0166EA  30310006            L05FE:MOVE.W    6(A1,D0.W),D0       
0166EE  6700003E                  BEQ       62(PC)               __L0600
0166F2  24310008                  MOVE.L    8(A1,D0.W),D2       
0166F6  028200FFFFFF              ANDI.L    #$FFFFFF,D2         
0166FC  B483                      CMP.L     D3,D2               
0166FE  6300000E                  BLS       14(PC)               __L05FF
016702  9481                      SUB.L     D1,D2               
016704  3382000A                  MOVE.W    D2,10(A1,D0.W)      
016708  4842                      SWAP      D2                  
01670A  13820009                  MOVE.B    D2,9(A1,D0.W)       
01670E  24310012            __L05FF:
                                  MOVE.L    18(A1,D0.W),D2      
016712  028200FFFFFF              ANDI.L    #$FFFFFF,D2         
016718  B483                      CMP.L     D3,D2               
01671A  6300FFCE                  BLS       -50(PC)              L05FE
01671E  9481                      SUB.L     D1,D2               
016720  33820014                  MOVE.W    D2,20(A1,D0.W)      
016724  4842                      SWAP      D2                  
016726  13820013                  MOVE.B    D2,19(A1,D0.W)      
01672A  6000FFBE                  BRA       -66(PC)              L05FE
01672E  4CDF0001            __L0600:
                                  MOVEM.L   (A7)+,D0            
016732  30300004                  MOVE.W    4(A0,D0.W),D0       
016736  6600FFAA                  BNE       -86(PC)              __L05FD
01673A  4CDF7FFF            __L0601:
                                  MOVEM.L   (A7)+,A0-A6/D0-D7   
01673E  B68B                      CMP.L     A3,D3               
016740  6E000004                  BGT       4(PC)                __L0602
016744  97C1                      SUBA.L    D1,A3               
016746  4E75                __L0602:
                                  RTS                           



016748  48E7F8E0            OP_UNLOAD:
                                  MOVEM.L   A0-A2/D0-D4,-(A7)   
01674C  B47CFFFF                  CMP.W     #$FFFF,D2           
016750  67000068                  BEQ       104(PC)              L0607
016754  B46EFFF0                  CMP.W     -16(A6),D2          
016758  67000060                  BEQ       96(PC)               L0607
01675C  610000A8                  BSR       168(PC)              L060D
016760  66000058                  BNE       88(PC)               L0607
016764  227900017094              MOVEA.L   $00017094,A1         L0674
01676A  D2C0                      ADDA.W    D0,A1               
01676C  223C0FFFFFFF              MOVE.L    #$FFFFFFF,D1        
016772  2051                      MOVEA.L   (A1),A0             
016774  2608                      MOVE.L    A0,D3               
016776  247900017094              MOVEA.L   $00017094,A2         L0674
01677C  B5F9000170A4        L0604:CMPA.L    $000170A4,A2         L0679
016782  67000018                  BEQ       24(PC)               __L0605
016786  B1DA                      CMPA.L    (A2)+,A0            
016788  6C00FFF2                  BGE       -14(PC)              L0604
01678C  B2AAFFFC                  CMP.L     -4(A2),D1           
016790  6F00FFEA                  BLE       -22(PC)              L0604
016794  222AFFFC                  MOVE.L    -4(A2),D1           
016798  6000FFE2                  BRA       -30(PC)              L0604
01679C  B2BC0FFFFFFF        __L0605:
                                  CMP.L     #$FFFFFFF,D1        
0167A2  66000008                  BNE       8(PC)                __L0606
0167A6  22390001709C              MOVE.L    $0001709C,D1         L0676
0167AC  2441                __L0606:
                                  MOVEA.L   D1,A2               
0167AE  9288                      SUB.L     A0,D1               
0167B0  1E3C0001                  MOVE.B    #1,D7               
0167B4  4EB900016606              JSR       $00016606            L05F2
0167BA  4CDF071F            L0607:MOVEM.L   (A7)+,A0-A2/D0-D4   
0167BE  4E75                      RTS                           
0167C0  48E7C0C0            __L0608:
                                  MOVEM.L   A0-A1/D0-D1,-(A7)   
0167C4  6000001C                  BRA       28(PC)               __L060A
0167C8  43F900010956              LEA       $00010956,A1         L0319
0167CE  41F900010C4A              LEA       $00010C4A,A0         __L0335
0167D4  303900010C4C              MOVE.W    $00010C4C,D0         __L0336
0167DA  9159                __L0609:
                                  SUB.W     D0,(A1)+            
0167DC  B3C8                      CMPA.L    A0,A1               
0167DE  6F00FFFA                  BLE       -6(PC)               __L0609
0167E2  48E7E0E0            __L060A:
                                  MOVEM.L   A0-A2/D0-D2,-(A7)   
0167E6  6100A170                  BSR       -24208(PC)           __L031A
0167EA  4CDF0707                  MOVEM.L   (A7)+,A0-A2/D0-D2   
0167EE  60000010                  BRA       16(PC)               __L060C
0167F2  43F900010956              LEA       $00010956,A1         L0319
0167F8  D159                __L060B:
                                  ADD.W     D0,(A1)+            
0167FA  B3C8                      CMPA.L    A0,A1               
0167FC  6F00FFFA                  BLE       -6(PC)               __L060B
016800  4CDF0303            __L060C:
                                  MOVEM.L   (A7)+,A0-A1/D0-D1   
016804  4E75                      RTS                           
016806  48E77880            L060D:MOVEM.L   A0/D1-D4,-(A7)      
01680A  207900017094              MOVEA.L   $00017094,A0         L0674
016810  3839000170A2              MOVE.W    $000170A2,D4         L0678
016816  5344                      SUBQ.W    #1,D4               
016818  4243                      CLR.W     D3                  
01681A  3204                __L060E:
                                  MOVE.W    D4,D1               
01681C  D243                      ADD.W     D3,D1               
01681E  E249                      LSR.W     #1,D1               
016820  3001                      MOVE.W    D1,D0               
016822  E540                      ASL.W     #2,D0               
016824  22700000                  MOVEA.L   0(A0,D0.W),A1       
016828  B4690000                  CMP.W     0(A1),D2            
01682C  6700001A                  BEQ       26(PC)               __L0611
016830  6200000A                  BHI       10(PC)               __L060F
016834  3801                      MOVE.W    D1,D4               
016836  5344                      SUBQ.W    #1,D4               
016838  60000006                  BRA       6(PC)                __L0610
01683C  3601                __L060F:
                                  MOVE.W    D1,D3               
01683E  5243                      ADDQ.W    #1,D3               
016840  B843                __L0610:
                                  CMP.W     D3,D4               
016842  6C00FFD6                  BGE       -42(PC)              __L060E
016846  72FF                      MOVEQ     #$FF,D1             
016848  4CDF011E            __L0611:
                                  MOVEM.L   (A7)+,A0/D1-D4      
01684C  4E75                      RTS                           
01684E  48E780C0            L0612:MOVEM.L   A0-A1/D0,-(A7)      
016852  2079000170A8              MOVEA.L   $000170A8,A0         L067A
016858  4240                      CLR.W     D0                  
01685A  5C40                __L0613:
                                  ADDQ.W    #6,D0               
01685C  43F00000                  LEA       0(A0,D0.W),A1       
016860  3340FFFE                  MOVE.W    D0,-2(A1)           
016864  42A9FFFA                  CLR.L     -6(A1)              
016868  B3F9000170AC              CMPA.L    $000170AC,A1         L067B
01686E  6D00FFEA                  BLT       -22(PC)              __L0613
016872  42680004                  CLR.W     4(A0)               
016876  33FC0001000170B6          MOVE.W    #1,$000170B6         L067E
01687E  33FC0006000170B8          MOVE.W    #6,$000170B8         L067F
016886  4CDF0301                  MOVEM.L   (A7)+,A0-A1/D0      
01688A  4E75                      RTS                           
01688C  48E70020            L0614:
                                  MOVEM.L   A2,-(A7)            
016890  08010000                  BTST      #0,D1               ; depack data size is even ?
016894  67000004                  BEQ       4(PC)                __L0615
016898  5281                      ADDQ.L    #1,D1               ; depack data size is odd => add 1 to make it even
01689A  D3B90001709C        __L0615:
                                  ADD.L     D1,$0001709C         L0676
0168A0  24790001709C              MOVEA.L   $0001709C,A2         L0676
0168A6  B5F900017066              CMPA.L    $00017066,A2         L065D
0168AC  6E0000A8                  BGT       168(PC)              __L061C
0168B0  32290000                  MOVE.W    0(A1),D1            
0168B4  207900017094              MOVEA.L   $00017094,A0         L0674
0168BA  2458                __L0616:
                                  MOVEA.L   (A0)+,A2            
0168BC  B1F9000170A4              CMPA.L    $000170A4,A0         L0679
0168C2  6200001A                  BHI       26(PC)               __L0618
0168C6  B26A0000                  CMP.W     0(A2),D1            
0168CA  6200FFEE                  BHI       -18(PC)              __L0616
0168CE  2479000170A4              MOVEA.L   $000170A4,A2         L0679
0168D4  25620004            __L0617:
                                  MOVE.L    -(A2),4(A2)         
0168D8  B5C8                      CMPA.L    A0,A2               
0168DA  6C00FFF8                  BGE       -8(PC)               __L0617
0168DE  5988                __L0618:
                                  SUBQ.L    #4,A0               
0168E0  2089                      MOVE.L    A1,(A0)             
0168E2  58B9000170A4              ADDQ.L    #4,$000170A4         L0679
0168E8  5279000170A2              ADDQ.W    #1,$000170A2         L0678
0168EE  3039000170A2              MOVE.W    $000170A2,D0         L0678
0168F4  B079000170A0              CMP.W     $000170A0,D0         L0677
0168FA  6E00004E                  BGT       78(PC)               __L061B
0168FE  4EB90000CCDA              JSR       $0000CCDA            __L013C
016904  4EB90000CBB4              JSR       $0000CBB4            __L012E
01690A  0C390001000170DE          CMPI.B    #1,$000170DE         L0690
016912  6700002A                  BEQ       42(PC)               L0619
016916  0C390003000170DE          CMPI.B    #3,$000170DE         L0690
01691E  6700001E                  BEQ       30(PC)               L0619
016922  0C390005000170DE          CMPI.B    #5,$000170DE         L0690
01692A  67000012                  BEQ       18(PC)               L0619
01692E  0C390002000170DE          CMPI.B    #2,$000170DE         L0690
016936  67000006                  BEQ       6(PC)                L0619
01693A  60000008                  BRA       8(PC)                __L061A
01693E  4EB90000CD98        L0619:JSR       $0000CD98            L0145
016944  4CDF0400            __L061A:
                                  MOVEM.L   (A7)+,A2            
016948  4E75                      RTS                           
01694A  203C00000013        __L061B:
                                  MOVE.L    #$13,D0             
016950  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016956  203C0000000B        __L061C:
                                  MOVE.L    #$B,D0              
01695C  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016962  2079000170A8        L061D:MOVEA.L   $000170A8,A0         L067A
016968  30305004                  MOVE.W    4(A0,D5.W),D0       
01696C  3239000170B8              MOVE.W    $000170B8,D1         L067F
016972  31815004                  MOVE.W    D1,4(A0,D5.W)       
016976  33F01004000170B8          MOVE.W    4(A0,D1.W),$000170B8 L067F
01697E  31801004                  MOVE.W    D0,4(A0,D1.W)       
016982  5279000170B6              ADDQ.W    #1,$000170B6         L067E
016988  3039000170B6              MOVE.W    $000170B6,D0         L067E
01698E  B079000170B4              CMP.W     $000170B4,D0         L067D
016994  6E0000E0                  BGT       224(PC)              __L0620
016998  2079000170A8        __L061E:
                                  MOVEA.L   $000170A8,A0         L067A
01699E  48E70020                  MOVEM.L   A2,-(A7)            
0169A2  2479000170B0              MOVEA.L   $000170B0,A2         L067C
0169A8  D4E90012                  ADDA.W    18(A1),A2           
0169AC  240A                      MOVE.L    A2,D2               
0169AE  D4E90016                  ADDA.W    22(A1),A2           
0169B2  D4FC0034                  ADDA.W    #$34,A2             
0169B6  218A1000                  MOVE.L    A2,0(A0,D1.W)       
0169BA  204A                      MOVEA.L   A2,A0               
0169BC  7000                      MOVEQ     #0,D0               
0169BE  30290014                  MOVE.W    20(A1),D0           
0169C2  D5C0                      ADDA.L    D0,A2               
0169C4  B5F9000170BA              CMPA.L    $000170BA,A2         L0680
0169CA  6C0000B6                  BGE       182(PC)              __L0621
0169CE  23CA000170B0              MOVE.L    A2,$000170B0         L067C
0169D4  4CDF0400                  MOVEM.L   (A7)+,A2            
0169D8  427008FE            __L061F:
                                  CLR.W     -2(A0,D0.L)         
0169DC  5580                      SUBQ.L    #2,D0               
0169DE  6600FFF8                  BNE       -8(PC)               __L061F
0169E2  9488                      SUB.L     A0,D2               
0169E4  3142FFF6                  MOVE.W    D2,-10(A0)          
0169E8  3142FFE4                  MOVE.W    D2,-28(A0)          
0169EC  3142FFE2                  MOVE.W    D2,-30(A0)          
0169F0  4280                      CLR.L     D0                  
0169F2  30290004                  MOVE.W    4(A1),D0            
0169F6  068000000002              ADDI.L    #2,D0               
0169FC  D089                      ADD.L     A1,D0               
0169FE  2140FFF8                  MOVE.L    D0,-8(A0)           
016A02  31690000FFF0              MOVE.W    0(A1),-16(A0)       
016A08  2149FFEC                  MOVE.L    A1,-20(A0)          
016A0C  11690002FFD2              MOVE.B    2(A1),-46(A0)       
016A12  117C0001FFFE              MOVE.B    #1,-2(A0)           
016A18  117C0001FFFF              MOVE.B    #1,-1(A0)           
016A1E  117C00FFFFFC              MOVE.B    #$FF,-4(A0)         
016A24  317CFFFFFFE6              MOVE.W    #$FFFF,-26(A0)      
016A2A  3141FFF2                  MOVE.W    D1,-14(A0)          
016A2E  117C0002FFDC              MOVE.B    #2,-36(A0)          
016A34  4268FFE8                  CLR.W     -24(A0)             
016A38  4268FFF4                  CLR.W     -12(A0)             
016A3C  4268FFDE                  CLR.W     -34(A0)             
016A40  4268FFE0                  CLR.W     -32(A0)             
016A44  4228FFFD                  CLR.B     -3(A0)              
016A48  4228FFDB                  CLR.B     -37(A0)             
016A4C  117C00FFFFDA              MOVE.B    #$FF,-38(A0)        
016A52  117C0000FFD6              MOVE.B    #0,-42(A0)          
016A58  4228FFD4                  CLR.B     -44(A0)             
016A5C  4228FFD3                  CLR.B     -45(A0)             
016A60  4268FFD8                  CLR.W     -40(A0)             
016A64  4228FFD1                  CLR.B     -47(A0)             
016A68  4268FFCE                  CLR.W     -50(A0)             
016A6C  4268FFCC                  CLR.W     -52(A0)             
016A70  4228FFD0                  CLR.B     -48(A0)             
016A74  4E75                      RTS                           
016A76  203C00000005        __L0620:
                                  MOVE.L    #5,D0               
016A7C  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016A82  203C0000000C        __L0621:
                                  MOVE.L    #$C,D0              
016A88  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016A8E  48E70802            L0622:MOVEM.L   A6/D4,-(A7)         
016A92  2079000170A8              MOVEA.L   $000170A8,A0         L067A
016A98  2C700000                  MOVEA.L   0(A0,D0.W),A6       
016A9C  BDFC00000000              CMPA.L    #0,A6               
016AA2  6700009A                  BEQ       154(PC)              __L0627
016AA6  48E7FFFE                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016AAA  6100BACE                  BSR       -17714(PC)           __L03DF
016AAE  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016AB2  206EFFEC                  MOVEA.L   -20(A6),A0          
016AB6  7400                      MOVEQ     #0,D2               
016AB8  34280012                  MOVE.W    18(A0),D2           
016ABC  D4680016                  ADD.W     22(A0),D2           
016AC0  06420034                  ADDI.W    #$34,D2             
016AC4  7800                      MOVEQ     #0,D4               
016AC6  38280014                  MOVE.W    20(A0),D4           
016ACA  204E                      MOVEA.L   A6,A0               
016ACC  91C2                      SUBA.L    D2,A0               
016ACE  D484                      ADD.L     D4,D2               
016AD0  43F02800                  LEA       0(A0,D2.L),A1       
016AD4  2239000170B0              MOVE.L    $000170B0,D1         L067C
016ADA  30D9                __L0623:
                                  MOVE.W    (A1)+,(A0)+         
016ADC  B3C1                      CMPA.L    D1,A1               
016ADE  6D00FFFA                  BLT       -6(PC)               __L0623
016AE2  95B9000170B0              SUB.L     D2,$000170B0         L067C
016AE8  2079000170A8              MOVEA.L   $000170A8,A0         L067A
016AEE  4241                      CLR.W     D1                  
016AF0  38301004            L0624:MOVE.W    4(A0,D1.W),D4       
016AF4  B044                      CMP.W     D4,D0               
016AF6  66000004                  BNE       4(PC)                __L0625
016AFA  3601                      MOVE.W    D1,D3               
016AFC  3204                __L0625:
                                  MOVE.W    D4,D1               
016AFE  67000010                  BEQ       16(PC)               __L0626
016B02  BDF01000                  CMPA.L    0(A0,D1.W),A6       
016B06  6200FFE8                  BHI       -24(PC)              L0624
016B0A  95B01000                  SUB.L     D2,0(A0,D1.W)       
016B0E  60E0                      BRA.S     -32(PC)              L0624
016B10  31B000043004        __L0626:
                                  MOVE.W    4(A0,D0.W),4(A0,D3.W) 
016B16  42B00000                  CLR.L     0(A0,D0.W)          
016B1A  31B9000170B80004          MOVE.W    $000170B8,4(A0,D0.W) L067F
016B22  33C0000170B8              MOVE.W    D0,$000170B8         L067F
016B28  4CDF4010                  MOVEM.L   (A7)+,A6/D4         
016B2C  5379000170B6              SUBQ.W    #1,$000170B6         L067E
016B32  BA40                      CMP.W     D0,D5               
016B34  67000014                  BEQ       20(PC)               __L0628
016B38  2C705000                  MOVEA.L   0(A0,D5.W),A6       
016B3C  4E75                      RTS                           
016B3E  4CDF4010            __L0627:
                                  MOVEM.L   (A7)+,A6/D4         
016B42  423900017081              CLR.B     $00017081            B_066D
016B48  4E75                      RTS                           
016B4A  508F                __L0628:
                                  ADDQ.L    #8,A7               
016B4C  3A03                      MOVE.W    D3,D5               
016B4E  6000A53A                  BRA       -23238(PC)           L033F
016B52  48E72000            L0629:MOVEM.L   D2,-(A7)            
016B56  23C80001708A              MOVE.L    A0,$0001708A         L0670
016B5C  33C000017190              MOVE.W    D0,$00017190         L06E1
016B62  343C0008                  MOVE.W    #8,D2               
016B66  0500                      BTST      D2,D0               
016B68  67000016                  BEQ       22(PC)               L062B
016B6C  6100007A                  BSR       122(PC)              FN_DISK_OPEN_FILE
016B70  6700000E                  BEQ       14(PC)               L062B
016B74  303C0000            __L062A:
                                  MOVE.W    #0,D0               
016B78  610000AC                  BSR       172(PC)              FN_DISK_CREATE_FILE
016B7C  60000054                  BRA       84(PC)               L062D


016B80  343C0009            L062B:
                                  MOVE.W    #9,D2                ; d2 <- #9
016B84  0500                      BTST      D2,D0                ; test bit 9
016B86  67000012                  BEQ       18(PC)               L062C
016B8A  6100005C                  BSR       92(PC)               FN_DISK_OPEN_FILE
016B8E  6600FFE4                  BNE       -28(PC)              __L062A
016B92  610000BE                  BSR       190(PC)              L0634
016B96  6000003A                  BRA       58(PC)               L062D


; =============================================================================
; FN_DISK_OPEN_FILE
016B9A  4EB90000BFCE        L062C:
                                  JSR       $0000BFCE            L00AC
016BA0  23C00001708E              MOVE.L    D0,$0001708E         L_FILE_HANDLE
016BA6  6B000030                  BMI       48(PC)               L062E
016BAA  303900017190              MOVE.W    $00017190,D0         L06E1
016BB0  343C000A                  MOVE.W    #$A,D2              
016BB4  0500                      BTST      D2,D0               
016BB6  6700001A                  BEQ       26(PC)               L062D
016BBA  48E78000                  MOVEM.L   D0,-(A7)            
016BBE  20390001708E              MOVE.L    $0001708E,D0         L_FILE_HANDLE
016BC4  4EB90000C01A              JSR       $0000C01A            SYS_DISK_FSEEK
016BCA  4CDF0001                  MOVEM.L   (A7)+,D0            
016BCE  6B000008                  BMI       8(PC)                L062E
016BD2  4CDF0004            L062D:MOVEM.L   (A7)+,D2            
016BD6  4E75                      RTS                           
016BD8  203C00000001        L062E:MOVE.L    #1,D0               
016BDE  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016BE4  6000FFB4                  BRA       -76(PC)              L062C


; =============================================================================
; FN_DISK_OPEN_FILE
016BE8  48E78000            FN_DISK_OPEN_FILE:
                                  MOVEM.L   D0,-(A7)            
016BEC  48C0                      EXT.L     D0                  
016BEE  4EB90000C03C              JSR       $0000C03C            SYS_DISK_FOPEN_R
016BF4  4CDF0001                  MOVEM.L   (A7)+,D0            
016BF8  4E75                      RTS                           


; =============================================================================
; D0 - file handle
016BFA  48E78000            FN_DISK_CLOSE_FILE:
                                  MOVEM.L   D0,-(A7)            
016BFE  20390001708E              MOVE.L    $0001708E,D0         L_FILE_HANDLE
016C04  4EB90000C07C              JSR       $0000C07C            SYS_DISK_CLOSE_FILE
016C0A  4A80                      TST.L     D0                  
016C0C  4CDF0001                  MOVEM.L   (A7)+,D0            
016C10  6B000004                  BMI       4(PC)                __L0631
016C14  4E75                      RTS                           
016C16  203C0000000E        __L0631:
                                  MOVE.L    #$E,D0              
016C1C  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016C22  6000FFD6                  BRA       -42(PC)              FN_DISK_CLOSE_FILE


; =============================================================================
016C26  48E78000            FN_DISK_CREATE_FILE:
                                  MOVEM.L   D0,-(A7)            
016C2A  48C0                      EXT.L     D0                  
016C2C  4EB90000C090              JSR       $0000C090            SYS_DISK_CREATE_FILE
016C32  23C00001708E              MOVE.L    D0,$0001708E         L_FILE_HANDLE
016C38  4CDF0001                  MOVEM.L   (A7)+,D0            
016C3C  6B000004                  BMI       4(PC)                __L0633
016C40  4E75                      RTS                           
016C42  203C00000008        __L0633:
                                  MOVE.L    #8,D0               
016C48  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016C4E  6000FFD6                  BRA       -42(PC)              FN_DISK_CREATE_FILE
016C52  48E78000            L0634:MOVEM.L   D0,-(A7)            
016C56  48C0                      EXT.L     D0                  
016C58  4EB90000C090              JSR       $0000C090            SYS_DISK_CREATE_FILE
016C5E  23C00001708E              MOVE.L    D0,$0001708E         L_FILE_HANDLE
016C64  4CDF0001                  MOVEM.L   (A7)+,D0            
016C68  6B000004                  BMI       4(PC)                __L0635
016C6C  4E75                      RTS                           
016C6E  203C00000008        __L0635:
                                  MOVE.L    #8,D0               
016C74  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016C7A  6000FFD6                  BRA       -42(PC)              L0634
016C7E  4EB90000C0B0        L0636:JSR       $0000C0B0            L00B4
016C84  4A80                      TST.L     D0                  
016C86  6B000004                  BMI       4(PC)                __L0637
016C8A  4E75                      RTS                           
016C8C  203C00000009        __L0637:
                                  MOVE.L    #9,D0               
016C92  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016C98  6000FFE4                  BRA       -28(PC)              L0636


; =============================================================================
; D1 num_bytes
; A1 dest buffer 
016C9C  48E7FFFE            FN_DISK_READ_BYTES:
                                  MOVEM.L   A0-A6/D0-D7,-(A7)   
016CA0  20390001708E              MOVE.L    $0001708E,D0         L_FILE_HANDLE
016CA6  4EB90000C0CE              JSR       $0000C0CE            SYS_DISK_READ_BYTES
016CAC  4A80                      TST.L     D0                  
016CAE  4CDF7FFF                  MOVEM.L   (A7)+,A0-A6/D0-D7   
016CB2  6B000004                  BMI       4(PC)                __L0639
016CB6  4E75                      RTS                           
016CB8  203C0000000D        __L0639:
                                  MOVE.L    #$D,D0              
016CBE  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016CC4  6000FFD6                  BRA       -42(PC)              FN_DISK_READ_BYTES


; =============================================================================
016CC8  48E78000            FN_DISK_WRITE_BYTES:
                                  MOVEM.L   D0,-(A7)            
016CCC  20390001708E              MOVE.L    $0001708E,D0         L_FILE_HANDLE
016CD2  4EB90000C0E2              JSR       $0000C0E2            SYS_DISK_WRITE_BYTES
016CD8  4A80                      TST.L     D0                  
016CDA  4CDF0001                  MOVEM.L   (A7)+,D0            
016CDE  6B000004                  BMI       4(PC)                __L063B
016CE2  4E75                      RTS                           
016CE4  203C00000007        __L063B:
                                  MOVE.L    #7,D0               
016CEA  4EF900016FF8              JMP       $00016FF8            FN_SHOW_FATAL_ERROR
016CF0  6000FFD6                  BRA       -42(PC)              FN_DISK_WRITE_BYTES




016CF4  13FC0001000170FC    L063C:MOVE.B    #1,$000170FC         L069E
016CFC  52390001718B              ADDQ.B    #1,$0001718B         L06DC
016D02  66000008                  BNE       8(PC)                __L063D
016D06  53390001718B              SUBQ.B    #1,$0001718B         L06DC
016D0C  4A390001718C        __L063D:
                                  TST.B     $0001718C            L06DD
016D12  67000046                  BEQ       70(PC)               L063F
016D16  6B000030                  BMI       48(PC)               __L063E
016D1A  53390001718F              SUBQ.B    #1,$0001718F         L06E0
016D20  66000038                  BNE       56(PC)               L063F
016D24  13F90001718E0001718F      MOVE.B    $0001718E,$0001718F  L06DF L06E0
016D2E  4EB9000141E4              JSR       $000141E4            __L04E2
016D34  53390001718D              SUBQ.B    #1,$0001718D         L06DE
016D3A  6600001E                  BNE       30(PC)               L063F
016D3E  42390001718C              CLR.B     $0001718C            L06DD
016D44  60000014                  BRA       20(PC)               L063F
016D48  207C0000ACEE        __L063E:
                                  MOVEA.L   #$0000ACEE,A0        L0023
016D4E  4EB90000BF2C              JSR       $0000BF2C            L00A4
016D54  42390001718C              CLR.B     $0001718C            L06DD
016D5A  4A3900017D04        L063F:TST.B     $00017D04            L0724
016D60  67000012                  BEQ       18(PC)               L0640
016D64  6B00000E                  BMI       14(PC)               L0640
016D68  4243                      CLR.W     D3                  
016D6A  267C00017D04              MOVEA.L   #$00017D04,A3        L0724
016D70  61000128                  BSR       296(PC)              L064E
016D74  4A3900017D14        L0640:TST.B     $00017D14            L0726
016D7A  67000012                  BEQ       18(PC)               L0641
016D7E  6B00000E                  BMI       14(PC)               L0641
016D82  7601                      MOVEQ     #1,D3               
016D84  267C00017D14              MOVEA.L   #$00017D14,A3        L0726
016D8A  6100010E                  BSR       270(PC)              L064E
016D8E  4A3900017D24        L0641:TST.B     $00017D24            L0728
016D94  67000016                  BEQ       22(PC)               L0642
016D98  6B000012                  BMI       18(PC)               L0642
016D9C  7602                      MOVEQ     #2,D3               
016D9E  267C00017D24              MOVEA.L   #$00017D24,A3        L0728
016DA4  610000F4                  BSR       244(PC)              L064E
016DA8  6100CEA2                  BSR       -12638(PC)           __L04B1
016DAC  4A3900017D34        L0642:TST.B     $00017D34            L072A
016DB2  67000012                  BEQ       18(PC)               L0643
016DB6  6B00000E                  BMI       14(PC)               L0643
016DBA  7603                      MOVEQ     #3,D3               
016DBC  267C00017D34              MOVEA.L   #$00017D34,A3        L072A
016DC2  610000D6                  BSR       214(PC)              L064E
016DC6  4A39000171AA        L0643:TST.B     $000171AA            L06EA
016DCC  67000006                  BEQ       6(PC)                __L0644
016DD0  6100016E                  BSR       366(PC)              __L0653
016DD4  4239000170FC        __L0644:
                                  CLR.B     $000170FC            L069E
016DDA  4E75                      RTS                           
016DDC  4A39000170CF        __L0645:
                                  TST.B     $000170CF            L0688
016DE2  6B0000B4                  BMI       180(PC)              __L064D
016DE6  67000082                  BEQ       130(PC)              __L064A
016DEA  4EB90000B6F2              JSR       $0000B6F2            L005B
016DF0  4A02                      TST.B     D2                  
016DF2  67000010                  BEQ       16(PC)               __L0646
016DF6  13C2000170D2              MOVE.B    D2,$000170D2         L068B
016DFC  13FC0028000170D3          MOVE.B    #$28,$000170D3      ;(#"(")  L068C
016E04  4A39000170D3        __L0646:
                                  TST.B     $000170D3            L068C
016E0A  67000012                  BEQ       18(PC)               L0647
016E0E  5339000170D3              SUBQ.B    #1,$000170D3         L068C
016E14  66000008                  BNE       8(PC)                L0647
016E18  4239000170D2              CLR.B     $000170D2            L068B
016E1E  5239000170D0        L0647:ADDQ.B    #1,$000170D0         L0689
016E24  6600006C                  BNE       108(PC)              __L064C
016E28  4840                      SWAP      D0                  
016E2A  3001                      MOVE.W    D1,D0               
016E2C  4A39000170D1              TST.B     $000170D1            L068A
016E32  6600000C                  BNE       12(PC)               __L0648
016E36  B0B9000170D6              CMP.L     $000170D6,D0         L068D
016E3C  67000044                  BEQ       68(PC)               L064B
016E40  23C0000170D6        __L0648:
                                  MOVE.L    D0,$000170D6         L068D
016E46  0C390002000170CF          CMPI.B    #2,$000170CF         L0688
016E4E  67000008                  BEQ       8(PC)                __L0649
016E52  4EB90000D2CA              JSR       $0000D2CA            L0180
016E58  4EB90000CF82        __L0649:
                                  JSR       $0000CF82            L0161
016E5E  13FC0001000170CF          MOVE.B    #1,$000170CF         L0688
016E66  6000001A                  BRA       26(PC)               L064B
016E6A  4EB90000D2CA        __L064A:
                                  JSR       $0000D2CA            L0180
016E70  13FC00FF000170CF          MOVE.B    #$FF,$000170CF       L0688
016E78  23FCFFFFFFFF000170D6      MOVE.L    #$FFFFFFFF,$000170D6 L068D
016E82  4239000170D1        L064B:CLR.B     $000170D1            L068A
016E88  13FC00FF000170D0          MOVE.B    #$FF,$000170D0       L0689
016E90  4E75                      RTS                           
016E92  5339000170D0        __L064C:
                                  SUBQ.B    #1,$000170D0         L0689
016E98  4E75                __L064D:
                                  RTS                           
016E9A  536B0004            L064E:SUBQ.W    #1,4(A3)            
016E9E  67000040                  BEQ       64(PC)               L0650
016EA2  302B0006                  MOVE.W    6(A3),D0            
016EA6  48C0                      EXT.L     D0                  
016EA8  322B0008                  MOVE.W    8(A3),D1            
016EAC  48C1                      EXT.L     D1                  
016EAE  D081                      ADD.L     D1,D0               
016EB0  6B00002E                  BMI       46(PC)               L0650
016EB4  B0BC00007FFF              CMP.L     #$7FFF,D0           
016EBA  6F000008                  BLE       8(PC)                __L064F
016EBE  203C00007FFF              MOVE.L    #$7FFF,D0           
016EC4  37400006            __L064F:
                                  MOVE.W    D0,6(A3)            
016EC8  302B000A                  MOVE.W    10(A3),D0           
016ECC  D06B000C                  ADD.W     12(A3),D0           
016ED0  6B00000E                  BMI       14(PC)               L0650
016ED4  3740000A                  MOVE.W    D0,10(A3)           
016ED8  4EB90000BA0E              JSR       $0000BA0E            L0083
016EDE  4E75                      RTS                           
016EE0  426B0006            L0650:CLR.W     6(A3)               
016EE4  426B000A                  CLR.W     10(A3)              
016EE8  177C00800001              MOVE.B    #$80,1(A3)          
016EEE  422B0002                  CLR.B     2(A3)               
016EF2  422B0000                  CLR.B     0(A3)               
016EF6  4EB90000BA0E              JSR       $0000BA0E            L0083
016EFC  4E75                      RTS                           


; =============================================================================
016EFE  47F900017D04        FN_INIT_2:
                                  LEA       $00017D04,A3         L0724
016F04  61000022                  BSR       34(PC)               L0652
016F08  47F900017D14              LEA       $00017D14,A3         L0726
016F0E  61000018                  BSR       24(PC)               L0652
016F12  47F900017D24              LEA       $00017D24,A3         L0728
016F18  6100000E                  BSR       14(PC)               L0652
016F1C  47F900017D34              LEA       $00017D34,A3         L072A
016F22  61000004                  BSR       4(PC)                L0652
016F26  4E75                      RTS                           


016F28  422B0000            L0652:CLR.B     0(A3)               
016F2C  422B0002                  CLR.B     2(A3)               
016F30  177C00800001              MOVE.B    #$80,1(A3)          
016F36  426B0006                  CLR.W     6(A3)               
016F3A  426B000A                  CLR.W     10(A3)              
016F3E  4E75                      RTS


016F40  4A79000171BC        __L0653:
                                  TST.W     $000171BC            L06EF
016F46  66000040                  BNE       64(PC)               __L0654
016F4A  4A79000171C0              TST.W     $000171C0            L06F1
016F50  66000078                  BNE       120(PC)              __L0657
016F54  4A79000171BE              TST.W     $000171BE            L06F0
016F5A  6700008E                  BEQ       142(PC)              L0659
016F5E  6B00008A                  BMI       138(PC)              L0659
016F62  3039000171C4              MOVE.W    $000171C4,D0         L06F3
016F68  3239000171AE              MOVE.W    $000171AE,D1         L06EC
016F6E  9240                      SUB.W     D0,D1               
016F70  6B000078                  BMI       120(PC)              L0659
016F74  67000074                  BEQ       116(PC)              L0659
016F78  33C1000171AE              MOVE.W    D1,$000171AE         L06EC
016F7E  5379000171BE              SUBQ.W    #1,$000171BE         L06F0
016F84  6000004A                  BRA       74(PC)               L0658
016F88  3039000171C2        __L0654:
                                  MOVE.W    $000171C2,D0         L06F2
016F8E  3239000171AE              MOVE.W    $000171AE,D1         L06EC
016F94  D240                      ADD.W     D0,D1               
016F96  B079000171B0              CMP.W     $000171B0,D0         L06ED
016F9C  6F000010                  BLE       16(PC)               __L0655
016FA0  33FC0001000171BC          MOVE.W    #1,$000171BC         L06EF
016FA8  3239000171B0              MOVE.W    $000171B0,D1         L06ED
016FAE  B27C7FFF            __L0655:
                                  CMP.W     #$7FFF,D1           
016FB2  6F000006                  BLE       6(PC)                __L0656
016FB6  323C7FFF                  MOVE.W    #$7FFF,D1           
016FBA  33C1000171AE        __L0656:
                                  MOVE.W    D1,$000171AE         L06EC
016FC0  5379000171BC              SUBQ.W    #1,$000171BC         L06EF
016FC6  60000008                  BRA       8(PC)                L0658
016FCA  5379000171C0        __L0657:
                                  SUBQ.W    #1,$000171C0         L06F1
016FD0  1039000171AE        L0658:MOVE.B    $000171AE,D0         L06EC
016FD6  4EB90000F1CE              JSR       $0000F1CE            __L027D
016FDC  1039000171AB              MOVE.B    $000171AB,D0         L06EB
016FE2  4EB90000F20C              JSR       $0000F20C            __L0280
016FE8  4E75                      RTS                           
016FEA  4EB90000F19C        L0659:JSR       $0000F19C            L027A
016FF0  4239000171AA              CLR.B     $000171AA            L06EA
016FF6  4E75                      RTS                           


; =============================================================================
; d0: error number
; displays fatal error message and quits
016FF8  42390001707D        FN_SHOW_FATAL_ERROR:
                                  CLR.B     $0001707D            B_L066A
016FFE  48E78000                  MOVEM.L   D0,-(A7)            
017002  4240                      CLR.W     D0                  
017004  4241                      CLR.W     D1                  
017006  4EB90000BC6E              JSR       $0000BC6E            SYS_VT52_SET_CURSOR
01700C  4CDF0001                  MOVEM.L   (A7)+,D0            
017010  48E78000                  MOVEM.L   D0,-(A7)            
017014  2F3C00017058              MOVE.L    #$00017058,-(A7)     STR_ERROR
01701A  3F3C0009                  MOVE.W    #9,-(A7)       ; Cconws - Write a string to the standard output device    
01701E  4E41                      TRAP      #1     				 ; GEMDOS                  
017020  5C8F                      ADDQ.L    #6,A7               
017022  4CDF0001                  MOVEM.L   (A7)+,D0            
017026  48C0                      EXT.L     D0                  
017028  4EB900011844              JSR       $00011844            L037D
01702E  33FC00000000ACEE          MOVE.W    #0,$0000ACEE         L0023
017036  33FC07000000ACF4          MOVE.W    #$700,$0000ACF4      __L0024
01703E  33FC00700000AD0C          MOVE.W    #$70,$0000AD0C       __L0025
017046  4EB900013F84              JSR       $00013F84            L04CE
01704C  4EB90000B792              JSR       $0000B792            L0067
017052  4EF900011010              JMP       $00011010            FN_TERMINATE_PROGRAM
; =============================================================================
017058  4572726F72200000    STR_ERROR:
                                  DC.B      $45,$72,$72,$6F,$72,$20,$00,$00 ; 'E', 'r', 'r', 'o', 'r', ' ', $0, $0
017060  0000                      DC.B      $00,$00 


; =============================================================================
 DATA 
017062                      L065C:DCB.W     2,0 
017066                      L065D:DCB.W     2,0 
01706A  0000                L065E:DC.B      $00,$00 
01706C  0000                L065F:DC.B      $00,$00 
01706E  00                  L0660:DC.B      $00 
01706F  00                  L0661:DC.B      $00 
017070  00                  L0662:DC.B      $00 
017071  00                  L0663:DC.B      $00 
017072  FFFF                L0664:DC.B      $FF,$FF 
017074  0000                L0665:DC.B      $00,$00 
017076  0000                L0666:DC.B      $00,$00 
017078  0000                L0667:DC.B      $00,$00 
01707A  0000                L0668:DC.B      $00,$00 
01707C  00                  B_IS_AMIGA:
                                  DC.B      $00 
01707D  00                  B_L066A:
                                  DC.B      $00 
01707E  0000                L066B:DC.B      $00,$00 
017080  00                  L066C:DC.B      $00 
017081  00                  B_066D:DC.B      $00 
017082                      L066E:DCB.W     2,0 
017086  013F00C7            L066F:DC.B      $01,$3F,$00,$C7 
01708A                      L0670:DCB.W     2,0 
01708E                      L_FILE_HANDLE:DCB.W     2,0 
017092  00                  L0672:DC.B      $00 
017093  00                  L0673:DC.B      $00 
017094                      L0674:DCB.W     2,0 
017098                      L0675:DCB.W     2,0 
01709C                      L0676:DCB.W     2,0 
0170A0  0000                L0677:DC.B      $00,$00 
0170A2  0000                L0678:DC.B      $00,$00 
0170A4                      L0679:DCB.W     2,0 
0170A8                      L067A:DCB.W     2,0 
0170AC                      L067B:DCB.W     2,0 
0170B0                      L067C:DCB.W     2,0 
0170B4  0000                L067D:DC.B      $00,$00 
0170B6  0000                L067E:DC.B      $00,$00 
0170B8  0000                L067F:DC.B      $00,$00 
0170BA                      L0680:DCB.W     2,0 
0170BE                      L0681:DCB.W     2,0 
0170C2                      L_L0682:DCB.W     2,0 ; contient adresse de qqch
0170C6  0000                W_L0683:DC.B      $00,$00 
0170C8  0000                L0684:DC.B      $00,$00 
0170CA  0000                L0685:DC.B      $00,$00 
0170CC  0000                L0686:DC.B      $00,$00 
0170CE  00                  L0687:DC.B      $00 
0170CF  FF                  L0688:DC.B      $FF 
0170D0  00                  L0689:DC.B      $00 
0170D1  00                  L068A:DC.B      $00 
0170D2  00                  L068B:DC.B      $00 
0170D3  000000              L068C:DC.B      $00,$00,$00 
0170D6  FFFF                L068D:DC.B      $FF,$FF 
0170D8  FFFF                L068E:DC.B      $FF,$FF 
0170DA                      L068F:DCB.W     2,0 
0170DE  00                  L0690:DC.B      $00 
0170DF  00                  L0691:DC.B      $00 
0170E0                      L_L0692:DCB.W     2,0 ; contient adresse de qqch
0170E4  0000                L0693:DC.B      $00,$00 
0170E6  0000                L0694:DC.B      $00,$00 
0170E8                      L0695:DCB.W     2,0 
0170EC  0000                L0696:DC.B      $00,$00 
0170EE  0000                L0697:DC.B      $00,$00 
0170F0  0000                L0698:DC.B      $00,$00 
0170F2  0000                L0699:DC.B      $00,$00 
0170F4  0000                L069A:DC.B      $00,$00 
0170F6  0000                L069B:DC.B      $00,$00 
0170F8  0000                L069C:DC.B      $00,$00 
0170FA  0000                L069D:DC.B      $00,$00 
0170FC  00                  L069E:DC.B      $00 
0170FD  00                  L069F:DC.B      $00 
0170FE  0000                L06A0:DC.B      $00,$00 
017100  00                  L06A1:DC.B      $00 
017101  00                  L06A2:DC.B      $00 
017102  00                  L06A3:DC.B      $00 
017103  00                  L06A4:DC.B      $00 
017104  0000                W_06A5:DC.B      $00,$00 
017106  00                  L06A6:DC.B      $00 
017107  00                  B_06A7:DC.B      $00 
017108  0000                L06A8:DC.B      $00,$00 
01710A  0000                L06A9:DC.B      $00,$00 
01710C  0000                L06AA:DC.B      $00,$00 
01710E  0000                L06AB:DC.B      $00,$00 
017110  0000                L06AC:DC.B      $00,$00 
017112                      L06AD:DCB.W     2,0 
017116  0000                L06AE:DC.B      $00,$00 
017118  0000                L06AF:DC.B      $00,$00 
01711A  0000                L06B0:DC.B      $00,$00 
01711C  0000                L06B1:DC.B      $00,$00 
01711E  0000                L06B2:DC.B      $00,$00 
017120  0000                L06B3:DC.B      $00,$00 
017122  0000                L06B4:DC.B      $00,$00 
017124  0000                L06B5:DC.B      $00,$00 
017126  0000                L06B6:DC.B      $00,$00 
017128  0000                L06B7:DC.B      $00,$00 
01712A  0000                L06B8:DC.B      $00,$00 
01712C  00                  L06B9:DC.B      $00 
01712D  0000                L06BA:DC.B      $00,$00 
01712F  00                  L06BB:DC.B      $00 
017130                      L06BC:DCB.W     2,0 
017134  0000                L06BD:DC.B      $00,$00 
017136                      L06BE:DCB.W     3,0 
01713C                      L06BF:DCB.W     2,0 
017140                      DW_06C0:DCB.W     2,0 
017144  0000                W_06C1:DC.B      $00,$00 
017146                      ADDR_SCRIPT_NAME:
                                  DCB.W     2,0 
01714A                      ADDR_CURRENT_PATH:DCB.W     2,0 
01714E                      L06C4:DCB.W     2,0 
017152                      L06C5:DCB.W     2,0 
017156  0000                L06C6:DC.B      $00,$00 
017158  0000                L06C7:DC.B      $00,$00 
01715A  0000                L06C8:DC.B      $00,$00 
01715C  0000                L06C9:DC.B      $00,$00 
01715E  0000                L06CA:DC.B      $00,$00 
017160  0000                L06CB:DC.B      $00,$00 
017162  0000                L06CC:DC.B      $00,$00 
017164  0000                L06CD:DC.B      $00,$00 
017166                      L06CE:DCB.W     5,0 
017170  0000                L06CF:DC.B      $00,$00 
017172                      L06D0:DCB.W     2,0 
017176  0000                W_DEPACKED_FILE_ID:
                                  DC.B      $00,$00 ; W=depacked file ID ?
017178                      L06D2:DCB.W     2,0 
01717C  0000                L06D3:DC.B      $00,$00 
01717E  0000                L06D4:DC.B      $00,$00 
017180  0000                L06D5:DC.B      $00,$00 
017182  0000                L06D6:DC.B      $00,$00 
017184  0000                L06D7:DC.B      $00,$00 
017186  0000                L06D8:DC.B      $00,$00 
017188  00                  L06D9:DC.B      $00 
017189  00                  L06DA:DC.B      $00 
01718A  00                  L06DB:DC.B      $00 
01718B  00                  L06DC:DC.B      $00 
01718C  00                  L06DD:DC.B      $00 
01718D  00                  L06DE:DC.B      $00 
01718E  00                  L06DF:DC.B      $00 
01718F  00                  L06E0:DC.B      $00 
017190                      L06E1:DCB.W     3,0 
017196  887100000000              DC.B      $88,$71,$00,$00,$00,$00 
01719C  00                  L06E2:DC.B      $00 
01719D  00                  L06E3:DC.B      $00 
01719E  0000                L06E4:DC.B      $00,$00 
0171A0  0000                L06E5:DC.B      $00,$00 
0171A2  0000                L06E6:DC.B      $00,$00 
0171A4  0000                L06E7:DC.B      $00,$00 
0171A6  0000                L06E8:DC.B      $00,$00 
0171A8  0000                L06E9:DC.B      $00,$00 
0171AA  00                  L06EA:DC.B      $00 
0171AB  000000              L06EB:DC.B      $00,$00,$00 
0171AE  0000                L06EC:DC.B      $00,$00 
0171B0                      L06ED:DCB.W     4,0 
0171B8                      L06EE:DCB.W     2,0 
0171BC  0000                L06EF:DC.B      $00,$00 
0171BE  0000                L06F0:DC.B      $00,$00 
0171C0  0000                L06F1:DC.B      $00,$00 
0171C2  0000                L06F2:DC.B      $00,$00 
0171C4                      L06F3:DCB.W     10,0
0171D8  0000                L06F4:DC.B      $00,$00 
0171DA  00                  L06F5:DC.B      $00 
0171DB  8000                L06F6:DC.B      $80,$00 
0171DD  00                  L06F7:DC.B      $00 
0171DE  0000                L06F8:DC.B      $00,$00 
0171E0                      L06F9:DCB.W     2,0 
0171E4                      L06FA:DCB.W     3,0 
0171EA  00                  L06FB:DC.B      $00 
0171EB  00                  L06FC:DC.B      $00 
0171EC                      L06FD:DCB.W     2,0 
0171F0                      L06FE:DCB.W     12,0
017208  00                        DC.B      $00 
017209  00                  L06FF:DC.B      $00 
01720A  00                  L0700:DC.B      $00 
01720B  000000              L0701:DC.B      $00,$00,$00 
01720E  0000                L0702:DC.B      $00,$00 
017210                      L0703:DCB.W     2,0 
017214                      L0704:DCB.W     2,0 
017218  0000                L0705:DC.B      $00,$00 
01721A  0000                L0706:DC.B      $00,$00 
01721C  0000                L0707:DC.B      $00,$00 
01721E  0000                L0708:DC.B      $00,$00 
017220  0000                L0709:DC.B      $00,$00 
017222  0000                L070A:DC.B      $00,$00 
017224                      L070B:DCB.W     2,0 
017228  0000                L070C:DC.B      $00,$00 
01722A  0000                L070D:DC.B      $00,$00 
01722C                      L070E:DCB.W     2,0 
017230  0000                L070F:DC.B      $00,$00 
017232  0000                L0710:DC.B      $00,$00 
017234  0000                L0711:DC.B      $00,$00 
017236  0000                L0712:DC.B      $00,$00 
017238  0000                L0713:DC.B      $00,$00 
01723A  0000                L0714:DC.B      $00,$00 
01723C  00                  L0715:DC.B      $00 
01723D  00                  L0716:DC.B      $00 
01723E  0000                L0717:DC.B      $00,$00 
017240  0000                L0718:DC.B      $00,$00 
017242  0000                L0719:DC.B      $00,$00 
017244  008040C020A060E0    L071A:DC.B      $00,$80,$40,$C0,$20,$A0,$60,$E0 
01724C  109050D030B070F0          DC.B      $10,$90,$50,$D0,$30,$B0,$70,$F0 
017254  088848C828A868E8          DC.B      $08,$88,$48,$C8,$28,$A8,$68,$E8 
01725C  189858D838B878F8          DC.B      $18,$98,$58,$D8,$38,$B8,$78,$F8 
017264  048444C424A464E4          DC.B      $04,$84,$44,$C4,$24,$A4,$64,$E4 
01726C  149454D434B474F4          DC.B      $14,$94,$54,$D4,$34,$B4,$74,$F4 
017274  0C8C4CCC2CAC6CEC          DC.B      $0C,$8C,$4C,$CC,$2C,$AC,$6C,$EC 
01727C  1C9C5CDC3CBC7CFC          DC.B      $1C,$9C,$5C,$DC,$3C,$BC,$7C,$FC 
017284  028242C222A262E2          DC.B      $02,$82,$42,$C2,$22,$A2,$62,$E2 
01728C  129252D232B272F2          DC.B      $12,$92,$52,$D2,$32,$B2,$72,$F2 
017294  0A8A4ACA2AAA6AEA          DC.B      $0A,$8A,$4A,$CA,$2A,$AA,$6A,$EA 
01729C  1A9A5ADA3ABA7AFA          DC.B      $1A,$9A,$5A,$DA,$3A,$BA,$7A,$FA 
0172A4  068646C626A666E6          DC.B      $06,$86,$46,$C6,$26,$A6,$66,$E6 
0172AC  169656D636B676F6          DC.B      $16,$96,$56,$D6,$36,$B6,$76,$F6 
0172B4  0E8E4ECE2EAE6EEE          DC.B      $0E,$8E,$4E,$CE,$2E,$AE,$6E,$EE 
0172BC  1E9E5EDE3EBE7EFE          DC.B      $1E,$9E,$5E,$DE,$3E,$BE,$7E,$FE 
0172C4  018141C121A161E1          DC.B      $01,$81,$41,$C1,$21,$A1,$61,$E1 
0172CC  119151D131B171F1          DC.B      $11,$91,$51,$D1,$31,$B1,$71,$F1 
0172D4  098949C929A969E9          DC.B      $09,$89,$49,$C9,$29,$A9,$69,$E9 
0172DC  199959D939B979F9          DC.B      $19,$99,$59,$D9,$39,$B9,$79,$F9 
0172E4  058545C525A565E5          DC.B      $05,$85,$45,$C5,$25,$A5,$65,$E5 
0172EC  159555D535B575F5          DC.B      $15,$95,$55,$D5,$35,$B5,$75,$F5 
0172F4  0D8D4DCD2DAD6DED          DC.B      $0D,$8D,$4D,$CD,$2D,$AD,$6D,$ED 
0172FC  1D9D5DDD3DBD7DFD          DC.B      $1D,$9D,$5D,$DD,$3D,$BD,$7D,$FD 
017304  038343C323A363E3          DC.B      $03,$83,$43,$C3,$23,$A3,$63,$E3 
01730C  139353D333B373F3          DC.B      $13,$93,$53,$D3,$33,$B3,$73,$F3 
017314  0B8B4BCB2BAB6BEB          DC.B      $0B,$8B,$4B,$CB,$2B,$AB,$6B,$EB 
01731C  1B9B5BDB3BBB7BFB          DC.B      $1B,$9B,$5B,$DB,$3B,$BB,$7B,$FB 
017324  078747C727A767E7          DC.B      $07,$87,$47,$C7,$27,$A7,$67,$E7 
01732C  179757D737B777F7          DC.B      $17,$97,$57,$D7,$37,$B7,$77,$F7 
017334  0F8F4FCF2FAF6FEF          DC.B      $0F,$8F,$4F,$CF,$2F,$AF,$6F,$EF 
01733C  1F9F5FDF3FBF7FFF          DC.B      $1F,$9F,$5F,$DF,$3F,$BF,$7F,$FF 


; =============================================================================
 BSS 
017344                            DS.B      252 
017440                      L071B:DS.B      68
017484                      L071C:DS.B      6 
01748A                      L071D:DS.W      1 
01748C                      L071E:DS.W      1 
01748E                      L071F:DS.L      1 
017492                      L0720:DS.L      1 
017496                      L0721:DS.B      1006
017884                      L0722:DS.B      512 
017A84                      L0723:DS.B      640 


017D04                      L0724:DS.B      1 
017D05                      L0725:DS.B      15


017D14                      L0726:DS.B      1 
017D15                      L0727:DS.B      15


017D24                      L0728:DS.B      1 
017D25                      L0729:DS.B      15


017D34                      L072A:DS.B      1 
017D35                      L072B:DS.B      15


017D44                      BUF_SCRIPT_NAME:
                                  DS.B      256 
017E44                      BUF_CURRENT_PATH:
                                  DS.B      256 
017F44                      BUF_L072E_272:
                                  DS.B      272 
018054                      BUF_L072F_1028:
                                  DS.B      1028
ZUEND: END
