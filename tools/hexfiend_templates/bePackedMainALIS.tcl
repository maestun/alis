big_endian
section "Header" {
	# magic, should be 'a1'	
	uint8 "isPacked"
	
	uint24 "depackedLength"
	
	# should be zero
	uint16 "scriptID"
}
section "AlisVM" {	
	# add this to allocated vram address, it'll be the address of the script vram addresses
	uint16 "script_data_tab_len dword count from vram start"

	uint16 "script_vram_tab_len scripts vram count"

	uint32 "unused dword 0"

	uint32 "max allocatable bytes for script vram"
	
	uint32 "offset from end of vram allocatable space to main script data"
}
section "DepackedDictionary" {	
	# used to depack this script
	hex 8 "depackDictionary"
}
section "PackedData" {
	bytes eof "packedData"
}