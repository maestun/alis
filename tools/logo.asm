; $33568 -> $387a0
_logo_header:
	$33568	00 40 17 00 00 16 00 00 00 00 00 00 00 16 00 00 02 68 00 20 00 18 00 20
			data

_logo_start:
	$33584	0a 00 00 0c 						
			cjmp24 #$10	_interon	; jump 16 bytes further
	$33588	1F	72	14	00	00	04	4B	00	01	3F	66	42 ; ???

_interon:
	$33594	64 								
			cinteron
	$33595	62 								
			cscanon
	$33596	bc 00 0a 						
			ctoblack oimmb #$a
	$33598	1e 00 01 14 0e					
			cstore oimmb #1 sdirw #$e
__loop1:
	$3359e	20 00 01 14 0e 					
			cadd oimmb #1 sdirw #$e
	$335a3	1f 38 14 0e 4e 02 03 e8 3a		
			ceval oeval odirw #$e oinfeg oimmw #$03e8 ofin ; ofin saute le prochain opcode
	$335ac  15 f0							
			cbnz8 #-16 ; branch if not zero __loop1
	$335ae	49 00 00 00 00 00 fb 00 0a 00	
			cputnat oimmb #$00 oimmb #$00 oimmb #$fb oimmb #$0a oimmb #$03
	$335b9	49 02 00 a0 00 03 00 2b	00 10 00 01		
			cputnat oimmw #$00a0 oimmb #$03 oimmb #$2b oimmb #$10 oimmb #$01
	$335c5	49 00 00 00 00 00 00 00 05 00 00 
			cputnat oimmb #$0 oimmb #$0 oimmb #0 oimmb #5 oimmb #0
	$335d0	42 
			cstop ; quitte l'execution de ce script en ajoutant 4 à la vraie pile

	$335d2	BE 00 06 00 32 42 1E 00 02 06 FF FE
