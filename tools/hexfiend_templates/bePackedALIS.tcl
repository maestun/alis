big_endian
section "Header" {
	uint8 "isPacked"
	uint24 "depackedLength"
	uint16 "scriptID"
}
section "DepackDictionary" {
	hex 8 "dic"
}
section "PackedData" {
	bytes eof "packedData"
}