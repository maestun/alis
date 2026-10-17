import re
import sys

def main():
    if len(sys.argv) < 3:
        print("Usage: gen_disasm.py <opcodes.c> <output.c>")
        sys.exit(1)

    in_file = sys.argv[1]
    out_file = sys.argv[2]

    with open(in_file, 'r') as f:
        content = f.read()

    def find_table(name):
        m = re.search(f'sOpcode\\s+{name}\\[\\]\\s*=\\s*\\{{(.*?)\\}};', content, re.DOTALL)
        if not m: return []
        table_str = m.group(1)
        entries = []
        for line in table_str.split('\n'):
            line = line.strip()
            if not line or line.startswith('//'): continue
            m2 = re.match(r'DECL_OPCODE\((0x[0-9a-fA-F]+),\s*([^,]+),\s*".*"\),?', line)
            if m2:
                entries.append((int(m2.group(1), 16), m2.group(2)))
        return entries

    opcodes = find_table('opcodes')
    opernames = find_table('opernames')
    storenames = find_table('storenames')
    addnames = find_table('addnames')
    esc1names = find_table('esc1names')
    esc2names = find_table('esc2names')
    esc3names = find_table('esc3names')

    with open(out_file, 'w') as f:
        f.write('''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disasm_formatter.h"

extern uint8_t *disasm_buf;
extern int disasm_size;
extern int disasm_pc;
extern FILE *disasm_fout;
extern int current_opcode_start;
extern int disasm_pass;

void get8() {
    if (disasm_pc < disasm_size) {
        if (disasm_pass == 2) fprintf(disasm_fout, " 0x%02X", disasm_buf[disasm_pc]);
        disasm_pc++;
    }
}
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

void disasm_ctab_28() {
    if (disasm_pass == 2) fprintf(disasm_fout, " cdata_28");
    while (disasm_pc < disasm_size && disasm_buf[disasm_pc] != 0) {
        get8();
    }
    get8(); // Read null terminator
}
''')

        def write_stubs(table, prefix=""):
            defined = set()
            for val, name in table:
                full_name = f"{prefix}{name}_{val:x}"
                if full_name in defined: continue
                defined.add(full_name)

                m = re.search(f'static void {name}\\(void\\)\\s*\\{{(.*?)}}', content, re.DOTALL)
                read_calls = []
                if m:
                    body = m.group(1)
                    if 'readexec_opernames' in body: read_calls.append('readexec_opernames();')
                    if 'readexec_storenames' in body: read_calls.append('readexec_storenames();')
                    if 'readexec_addnames' in body: read_calls.append('readexec_addnames();')
                    if 'readexec_esc1names' in body: read_calls.append('readexec_esc1names();')
                    if 'readexec_esc2names' in body: read_calls.append('readexec_esc2names();')
                    if 'readexec_esc3names' in body: read_calls.append('readexec_esc3names();')

                    if 'get8' in body: read_calls.append('get8();')
                    if 'get16' in body: read_calls.append('get16();')
                    if 'get24' in body: read_calls.append('get24();')
                    if 'get32' in body: read_calls.append('get32();')

                if name == 'ctab' and val == 0x28: continue

                f.write(f'void disasm_{full_name}() {{\n')
                print_name = f"{prefix}{name}_{val:02x}"
                f.write(f'    if (disasm_pass == 2) fprintf(disasm_fout, " {print_name}");\n')

                if name in ['cjump', 'cbra']:
                    f.write(f'    int target = get_branch_target(2);\n')
                    f.write(f'    if (disasm_pass == 1) add_label(target);\n')
                    f.write(f'    else fprintf(disasm_fout, " loc_%06X", target);\n')
                    f.write(f'    disasm_pc += 2;\n')
                elif name in ['cjmpfalse', 'cjmptrue']:
                    f.write(f'    int target = get_branch_target(1);\n')
                    f.write(f'    if (disasm_pass == 1) add_label(target);\n')
                    f.write(f'    else fprintf(disasm_fout, " loc_%06X", target);\n')
                    f.write(f'    disasm_pc += 1;\n')
                else:
                    for call in read_calls:
                        f.write(f'    {call}\n')

                f.write('}\n')

        write_stubs(opcodes)
        write_stubs(opernames, "oper_")
        write_stubs(storenames, "store_")
        write_stubs(addnames, "add_")
        write_stubs(esc1names, "esc1_")
        write_stubs(esc2names, "esc2_")
        write_stubs(esc3names, "esc3_")

        def write_dispatcher(table, func_name, prefix=""):
            f.write(f'void {func_name}() {{\n')
            f.write('    if (disasm_pc >= disasm_size) return;\n')
            if prefix == "":
                f.write('    current_opcode_start = disasm_pc;\n')
                f.write('    if (disasm_pass == 2) {\n')
                f.write('        if (has_label(disasm_pc)) fprintf(disasm_fout, "loc_%06X:\\n", disasm_pc);\n')
                f.write('        fprintf(disasm_fout, "    ");\n')
                f.write('    }\n')

            f.write('    uint8_t opcode = disasm_buf[disasm_pc++];\n')
            f.write('    switch(opcode) {\n')
            for val, name in table:
                if name == 'ctab' and val == 0x28:
                    f.write(f'        case {val}: disasm_ctab_28(); break;\n')
                elif name == 'opcodes': continue
                else:
                    f.write(f'        case {val}: disasm_{prefix}{name}_{val:x}(); break;\n')
            f.write('        default:\n')
            f.write(f'            if (disasm_pass == 2) fprintf(disasm_fout, " unknown_%02x", opcode);\n')
            f.write('            break;\n')
            f.write('    }\n')
            if prefix == "":
                f.write('    if (disasm_pass == 2) fprintf(disasm_fout, "\\n");\n')
            f.write('}\n')

        write_dispatcher(opcodes, "readexec_opcodes")
        write_dispatcher(opernames, "readexec_opernames", "oper_")
        write_dispatcher(storenames, "readexec_storenames", "store_")
        write_dispatcher(addnames, "readexec_addnames", "add_")
        write_dispatcher(esc1names, "readexec_esc1names", "esc1_")
        write_dispatcher(esc2names, "readexec_esc2names", "esc2_")
        write_dispatcher(esc3names, "readexec_esc3names", "esc3_")

if __name__ == "__main__":
    main()
