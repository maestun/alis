import sys
import re
import struct
import os

def parse_mappings(content, table_name):
    mapping = {}
    try:
        m = re.search(r'sOpcode\s+' + table_name + r'\[\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
        if m:
            for match in re.finditer(r'DECL_OPCODE\((0x[0-9a-fA-F]+),\s*([^,]+),', m.group(1)):
                val = int(match.group(1), 16)
                name = match.group(2)
                mapping[f"{name}_{val:02x}"] = val
    except Exception as e:
        print("Error parsing", table_name, e)
    return mapping

def assemble(input_asm, output_bin):
    with open(input_asm, 'r') as f:
        lines = f.readlines()

    print("Parsing opcodes from src/opcodes.c")
    try:
        with open("../../src/opcodes.c", 'r') as f:
            content = f.read()
    except FileNotFoundError:
        with open("src/opcodes.c", 'r') as f:
            content = f.read()

    opcodes = parse_mappings(content, "opcodes")
    opernames = parse_mappings(content, "opernames")
    storenames = parse_mappings(content, "storenames")
    addnames = parse_mappings(content, "addnames")
    esc1names = parse_mappings(content, "esc1names")
    esc2names = parse_mappings(content, "esc2names")
    esc3names = parse_mappings(content, "esc3names")

    if not opcodes:
        print("Failed to parse opcodes!")
        return

    all_tokens = {}
    for name, val in opernames.items(): all_tokens[name] = val
    for name, val in storenames.items(): all_tokens[name] = val
    for name, val in addnames.items(): all_tokens[name] = val
    for name, val in esc1names.items(): all_tokens[name] = val
    for name, val in esc2names.items(): all_tokens[name] = val
    for name, val in esc3names.items(): all_tokens[name] = val

    current_pc = 0
    labels = {}
    inst_info = []

    for line in lines:
        orig_line = line
        line = line.split(';')[0].strip()
        if not line: continue

        m = re.match(r'loc_([0-9a-fA-F]+):', line)
        if m:
            labels[f"loc_{m.group(1)}"] = current_pc
            line = line[m.end():].strip()
            if not line: continue

        if line.startswith('.incbin'):
            m = re.match(r'\.incbin\s+"([^"]+)"', line)
            if m:
                filename = m.group(1)
                try:
                    size = os.path.getsize(filename)
                    inst_info.append(('incbin', filename, current_pc, size))
                    current_pc += size
                except:
                    print(f"Failed to stat {filename}")
            continue

        tokens = line.split()
        if not tokens: continue

        op = tokens[0]
        start_pc = current_pc

        if op == 'cdata_28':
            current_pc += 1
            for tok in tokens[1:]:
                if tok.startswith('0x'): current_pc += 1
        elif op in opcodes:
            current_pc += 1
            for tok in tokens[1:]:
                if tok.startswith('loc_'):
                    if op.startswith('cbra') or op.startswith('cjump'): current_pc += 2
                    else: current_pc += 1
                elif tok.startswith('0x'):
                    current_pc += 1
                else:
                    current_pc += 1

        inst_info.append(('inst', tokens, start_pc, current_pc - start_pc))

    output = bytearray()

    for kind, data, start_pc, length in inst_info:
        if kind == 'incbin':
            with open(data, 'rb') as f:
                output.extend(f.read())
        elif kind == 'inst':
            tokens = data
            op = tokens[0]
            if op == 'cdata_28':
                output.append(0x28)
                for tok in tokens[1:]:
                    if tok.startswith('0x'):
                        output.append(int(tok, 16))
            elif op in opcodes:
                output.append(opcodes[op])
                for tok in tokens[1:]:
                    if tok.startswith('loc_'):
                        target_pc = labels.get(tok, int(tok[4:], 16))
                        offset = target_pc - start_pc

                        if op.startswith('cbra') or op.startswith('cjump'):
                            output.extend(struct.pack('>h', offset))
                        else:
                            output.append(offset & 0xFF)
                    elif tok.startswith('0x'):
                        output.append(int(tok, 16))
                    else:
                        val = all_tokens.get(tok)
                        if val is not None:
                            output.append(val)
                        else:
                            print(f"Unknown token: {tok}")
                            output.append(0)

    with open(output_bin, 'wb') as f:
        f.write(output)

    print(f"Successfully assembled to {output_bin}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: reassembler.py <input.asm> <output.ao>")
        sys.exit(1)
    assemble(sys.argv[1], sys.argv[2])
