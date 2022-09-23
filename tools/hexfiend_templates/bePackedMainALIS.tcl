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
	uint16 "vm.specs.max_prog"
	# uint16 "script_data_tab_len dword count from vram start"

	uint16 "vm.specs.max_ent"
	# uint16 "script_vram_tab_len scripts vram count"

	uint32 "vm.specs.max_host_ram"
	# uint32 "unused dword 0"

	uint32 "vm.specs.debsprit_offset"
	# uint32 "max allocatable bytes for script vram"
	
	uint32 "vm.specs.finsprit_offset"
	# uint32 "offset from end of vram allocatable space to main script data"
}
section "DepackedDictionary" {	
	# used to depack this script
	hex 8 "dic"
}
section "PackedData" {
	bytes eof "packedData"
}