big_endian
section "Header" {
	# 0...1 : word
	uint16 "scriptID"
	# 2 : byte
	uint8 "read at 18e96, in vram-0x2e"
	# 3 : byte
	uint8 "??"
	# 4...5 : word
	uint16 "code start offset - 2"
	# 6...9 : dword
	uint32 "offset to subscript routine"
	# 10...13 : dword
	uint32 "offset to interrupt routine"
	# 14...17 : dword
	uint32 "???"
	# 18...19 : word
	uint16 "word #3 read at 18e32"
	# 20...21 : word
	uint16 "ram to allocate"
	# 22...23 : word
	uint16 "word #5 read at 18e38"
}
section "ALISData" {
	bytes eof "ALISData"
}