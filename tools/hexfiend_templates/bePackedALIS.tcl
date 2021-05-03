big_endian
section "Header" {
	uint8 "isPacked"
	uint24 "depackedLength"
	uint16 "scriptID"
	hex 8 "depackDictionary"
}
section "PackedData" {
	bytes eof "packedData"
}