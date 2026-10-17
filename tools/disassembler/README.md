# ALIS Disassembler & Reassembler

This directory contains tools to disassemble and reassemble ALIS virtual machine scripts (`.ao` and `.io` files).

## Disassembler

The disassembler is written in C to exactly match the parsing logic of the main ALIS engine. It is generated dynamically from the core VM opcodes by `gen_disasm.py`.

### Building

```bash
cd tools/disassembler
mkdir build && cd build
cmake ..
make
```

### Usage

```bash
./build/disassembler <input.ao> <output.asm>
```

This will produce three files:
- `<output.asm>`: The pseudo-assembly source code.
- `<output.asm>_header.bin`: The exact raw bytes of the 24-byte script header.
- `<output.asm>_assets.bin`: The exact raw bytes of the embedded asset table (if present).

The disassembly syntax supports standard GNU-style `.incbin` directives to link the binaries back in perfectly. Labels are automatically generated (e.g., `loc_XXXXXX`) and branched against to keep the output human-readable and modifiable.

## Reassembler

The reassembler (`reassembler.py`) compiles the pseudo-assembly back into a 1:1 identical script binary.

### Usage

```bash
python3 reassembler.py <input.asm> <output.ao>
```

It parses the opcode tables from `src/opcodes.c` to accurately determine byte values. All absolute jump labels (`loc_XXXXXX`) are correctly translated back into relative 8-bit or 16-bit branch offsets according to the `cbra`/`cjump` constraints.

*Note: You must run `reassembler.py` from the root of the repository or from the `tools/disassembler` folder so it can locate `src/opcodes.c`.*
