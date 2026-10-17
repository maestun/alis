import re

def extract_funcs(filename):
    with open(filename, "r") as f:
        content = f.read()
    funcs = {}
    for match in re.finditer(r'void\s+([a-zA-Z0-9_]+)\s*\([^)]*\)\s*\{', content):
        name = match.group(1)
        start = match.end()
        braces = 1
        pos = start
        while pos < len(content) and braces > 0:
            if content[pos] == '{': braces += 1
            elif content[pos] == '}': braces -= 1
            pos += 1
        body = content[start:pos-1]
        funcs[name] = body
    return funcs

funcs = {}
for file in ["src/opcodes.c", "src/opernames.c", "src/storenames.c", "src/addnames.c", "src/escnames.c"]:
    funcs.update(extract_funcs(file))

def get_mappings(filename, tablename):
    with open(filename, "r") as f:
        content = f.read()
    table_match = re.search(r'const sAlisOpcode ' + tablename + r'\[\] = \{(.*?)\};', content, re.DOTALL)
    if not table_match: return {}
    mapping = {}
    for match in re.finditer(r'DECL_OPCODE\((0x[0-9a-fA-F]+),\s*(\w+),', table_match.group(1)):
        op = int(match.group(1), 16)
        name = match.group(2)
        mapping[op] = name
    return mapping

opcodes_map = get_mappings("src/opcodes.c", "opcodes")
opernames_map = get_mappings("src/opernames.c", "opernames")
storenames_map = get_mappings("src/storenames.c", "storenames")
addnames_map = get_mappings("src/addnames.c", "addnames")
esc1_map = get_mappings("src/escnames.c", "codesc1names")
esc2_map = get_mappings("src/escnames.c", "codesc2names")
esc3_map = get_mappings("src/escnames.c", "codesc3names")

c_code = """
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "alis.h"
#include "alis_private.h"

extern u8 *disasm_buf;
extern int disasm_size;
extern int disasm_pc;
extern FILE *disasm_fout;

// Provide dummy alis wrapper since our get8/etc depend on it to be simple
extern sAlisVM alis;

void get8() { if (disasm_pc < disasm_size) { fprintf(disasm_fout, " %02X", disasm_buf[disasm_pc++]); } }
void get16() { get8(); get8(); }
void get24() { get8(); get8(); get8(); }
void get32() { get16(); get16(); }

void readexec_opcodes();
void readexec_opernames();
void readexec_storenames();
void readexec_addnames();
void readexec_esc1names();
void readexec_esc2names();
void readexec_esc3names();
"""

def compile_func(name, body):
    out = f"void disasm_{name}() {{\n"
    lines = body.split('\n')
    for line in lines:
        if "readexec_opcode" in line: out += "    readexec_opcodes();\n"
        elif "readexec_opername" in line: out += "    readexec_opernames();\n"
        elif "readexec_storename" in line: out += "    readexec_storenames();\n"
        elif "readexec_addname" in line: out += "    readexec_addnames();\n"
        elif "readexec_codesc1name" in line: out += "    readexec_esc1names();\n"
        elif "readexec_codesc2name" in line: out += "    readexec_esc2names();\n"
        elif "readexec_codesc3name" in line: out += "    readexec_esc3names();\n"
        elif "xread8" in line or "read8" in line: out += "    get8();\n"
        elif "xread16" in line or "read16" in line: out += "    get16();\n"
        elif "xread24" in line or "read24" in line: out += "    get24();\n"
        elif "xread32" in line or "read32" in line: out += "    get32();\n"
        elif "pc++" in line: out += "    get8();\n"
        elif "pc +=" in line:
            m = re.search(r'pc \+= (\d+)', line)
            if m: out += f"    for(int i=0; i<{m.group(1)}; i++) get8();\n"
            else:
                m = re.search(r'pc\s*\+=\s*(\w+)', line)
                if m: out += f"    // WARNING: dynamic offset {m.group(1)}\n"
    out += "}\n\n"
    return out

for op, name in list(opcodes_map.items()) + list(opernames_map.items()) + list(storenames_map.items()) + list(addnames_map.items()) + list(esc1_map.items()) + list(esc2_map.items()) + list(esc3_map.items()):
    if name in funcs:
        c_code += compile_func(name, funcs[name])

def write_dispatch(tablename, mapping):
    c = f"void readexec_{tablename}() {{\n"
    c += "    if (disasm_pc >= disasm_size) return;\n"
    c += "    u8 op = disasm_buf[disasm_pc++];\n"
    c += f"    fprintf(disasm_fout, \" %02X [\"{tablename}\":%02X]\", op, op);\n"
    c += "    switch(op) {\n"
    for op, name in mapping.items():
        if name in funcs:
            c += f"        case 0x{op:02x}: disasm_{name}(); break;\n"
    c += "        default: fprintf(disasm_fout, \" [UNKNOWN]\"); break;\n"
    c += "    }\n"
    c += "}\n\n"
    return c

c_code += write_dispatch("opcodes", opcodes_map)
c_code += write_dispatch("opernames", opernames_map)
c_code += write_dispatch("storenames", storenames_map)
c_code += write_dispatch("addnames", addnames_map)
c_code += write_dispatch("esc1names", esc1_map)
c_code += write_dispatch("esc2names", esc2_map)
c_code += write_dispatch("esc3names", esc3_map)

with open("disasm_generated.c", "w") as f:
    f.write(c_code)
