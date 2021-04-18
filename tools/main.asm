; $2d279 -> $33568
_main_header:
	$2d278	00 00 17 00 00 16 00 00 4D C9 00 00 03 21 00 00 4E AE 00 20 42 72 00 20
			data

_main_start:
	$2d290	46 00 0E 40 00 00 00 00 00 7F 00 56 00 00 00 00 00 FF 00 7D 00 00 00 00 00 00 00 FF FF 00 01 00 00 00 00 FF
			cdefsc 00 0E 40 00 00 00 00 00 7F 00 56 00 00 00 00 00 FF 00 7D 00 00 00 00 00 00 00 FF FF 00 01 00 00 00 00 FF
	$2d2b4	46 00 42 40 00 00 00 00 00 00 00 C7 00 00 00 00 01 3F 00 C7 00 00 00 00 00 00 00 FF FF 00 01 00 00 00 00 FF
			cdefsc 00 42 40 00 00 00 00 00 00 00 C7 00 00 00 00 01 3F 00 C7 00 00 00 00 00 00 00 FF FF 00 01 00 00 00 00 FF
	$2d2d8	29 00 80 01 01 00 5A
			cdim #$0080 #$01 #$01 #$005a ; cdim offset counter byte2 [words]
	$2d2df	29 13 82 00 01
			cdim #$1382 #$00 #$01
	$2d2e4	29 14 04 00 01
			cdim #$1404 #$00 #$01
	$2d2e9	29 14 86 00 01

	$2d2ee	29 15 0C 00 08

	$2d2f3	29 15 36 00 02

	$2d2f8	29 15 42 00 02
			; ...
			; shiteloads of cdim
			; ...
	$2d54b	1e 00 0c 38 00 01 3a 0e 3c e0
			cstore oimmb #$0c seval oimmb #$01 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$01] <- #$0c 
	$2d555	1e 00 08 38 00 02 3a 0e 3c e0
			cstore oimmb #$08 seval oimmb #$02 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$02] <- #$08
	$2d55f	1e 00 0c 38 00 03 3a 0e 3c e0
			cstore oimmb #$0c seval oimmb #$03 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$03] <- #$0c 
	$2d569	1e 00 0e 38 00 04 3a 0e 3c e0
			cstore oimmb #$0e seval oimmb #$04 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$04] <- #$0e
	$2d573	1e 00 0a 38 00 05 3a 0e 3c e0
			cstore oimmb #$0a seval oimmb #$05 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$05] <- #$0a
	$2d57d	1e 00 0f 38 00 06 3a 0e 3c e0
			cstore oimmb #$0f seval oimmb #$06 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$06] <- #$0f 
	$2d587	1e 00 0b 38 00 07 3a 0e 3c e0
			cstore oimmb #$0b seval oimmb #$07 ofin sloctc #$3ce0
			; ram[ #$3ce0 + #$07] <- #$0b
	$2d591	07 00 00 db
			cjsr24 #$db ; jump subroutine $2d670
	$2d595	07 00 02 39 
			cjsr #$239	; jump subroutine $2d7d2
			; 32 00 09 A0 08 
			; FA 0A 00 00 CD 1E 72 08 
			; 42 40 2E 08 42 40 0C 00 
			; FF 9C 00 B8 00 0A 00 30 
			; 00 0C 00 3B 00 0E 00 46 
			; 00 0F 00 4F 00 10 00 58 
			; 00 11 00 63 00 12 00 6C 
			; 00 17 00 70 00 19 00 74
			; 00 1E 00 78 00 1F 00 7C
			; 00 20 00 80 0A 00 00 88 
			; 1E 06 15 08 06 42 51 07
			; 00 40 D5 0A 00 00 79 1E 
			; 06 15 08 06 42 51 07 00
			; 40 FB 0A 00 00 6A 1E 72
			; 06 42 6C 32 00 10 BF 0A
			; 00 00 5D 1E 72 06 42 51
			; 07 00 40 E1 0A 00 00 50
			; 1E 06 15 08 06 42 51 07
			; 00 41 A8 0A 00 00 41 1E
			; 72 06 42 5B 32 00 31 84
			; 0A 00 00 34 32 00 2C F1
			; 0A 00 00 2C 32 00 2F C7
			; 0A 00 00 24 32 00 3A 1C
			; 0A 00 00 1C 07 00 44 30
			; 0A 00 00 14 32 00 46 42
			; 0A 00 00 0C 32 00 42 21
			; 0A 00 00 04 32 00 2E 85
			; 66 42
	$2d670 	1e 00 00 06 42 6d
			cstore oimmb #$0 slocb #$426d
	$2d676	1f 38 78 52 02 07 d0 3a
			ceval oeval omodel oinf oimmw #$07d0 ofin
	$2d67e	14 00 00 3a
			cbz24 #$3a
	$2d682	1e 00 01 06 42 6d
			cstore oimmb #$1 slocb #$426d
	$2d688	1f 38 78 5a 00 0a 4a 00 01 3a
			ceval oeval omodel omod oimmb #$0a oegal oimmb #$01 ofin
	$2d692	14 00 00 06
			cbz24 #$06
	$2d696	1e 00 01 06 42 38
			cstore oimmb #$01 slocb #$4238
	$2d69c	1f 38 78 54 02 03 f1 3a
			ceval oeval omodel osup oimmw #$03f1 ofin
	$2d6a5	14 00 00 0a
			cbz24 #$0a
	$2d6a8  1e 00 01 06 42 35
			cstore oimmb #$01 slocb #$4235
	$2d6ae	0a 00 00 06
			cjmp24 #$6 ; jump at $2b6b8
	$2d6b2 	1e 00 01 06 42 36
			cstore oimmb #$01 slocb #$4236
	$2d6b8	0a 00 00 bf
			cjmp24 #$bf ; jump at $2d77b
			; ...
	$2d77b	1f 38 00 00 76 54 02 02 d0 3a
			ceval oeval oimmb #$00 ofree osup oimmw #$02d0 ofin
	$2d785	14 00 00 0a
			cbz24 #$0a
	$2d789 	1e 00 01 06 42 39
			cstore oimmb #$01 slocb #$4239
	$2d78f  0a 00 00 3e
			cjmp24 #$3e
			; ...
	$2d7d1	11 
			cret

			; ...
			; logo tourne
			; ...
	$2d826	1f 38 74 42 00 0c 4c 00 00 3a
			ceval oeval oshiftkey oand oimmb #$0c odiff oimmb #$00 ofin
	$2d830	14 00 00 0f 
			cbz24 #$0f
			; ...
	$2d844	42
			cstop ; retour à script logo ??

	$2d86b	$12 $ba
			cbz8 #$ffba ; $2d826
			; ...
	$32047	1f 38 74 42 00 0c 4c 00 00 3a
			ceval oeval oshiftkey oand oimmb #$0c odiff oimmb #$00 ofin
	$32051	14 00 00 83
			cbz24 #$83
			; ...
	$320d8	1f 38 6a 4a 00 1b 3a
			ceval oeval oinkey 
