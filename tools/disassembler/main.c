#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disasm_formatter.h"

uint8_t *disasm_buf = NULL;
int disasm_size = 0;
int disasm_pc = 0;
FILE *disasm_fout = NULL;
int disasm_pass = 1;

void readexec_opcodes();

uint16_t get_le16(uint8_t *p) { return p[0] | (p[1] << 8); }
uint16_t get_be16(uint8_t *p) { return (p[0] << 8) | p[1]; }

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.ao> <output.asm>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        printf("Failed to open %s\n", argv[1]);
        return 1;
    }

    fseek(fin, 0, SEEK_END);
    disasm_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    disasm_buf = (uint8_t *)malloc(disasm_size);
    fread(disasm_buf, 1, disasm_size, fin);
    fclose(fin);

    disasm_fout = fopen(argv[2], "w");
    if (!disasm_fout) {
        printf("Failed to open %s for writing\n", argv[2]);
        return 1;
    }

    int code_loc_offset = get_le16(disasm_buf + 6);
    if (code_loc_offset == 0) {
        code_loc_offset = get_be16(disasm_buf + 6);
    }

    if (disasm_buf[6] == 0 && disasm_buf[7] != 0) {
        code_loc_offset = get_be16(disasm_buf + 6);
    }

    uint32_t asset_table_offset = get_le16(disasm_buf + 16);
    if (disasm_buf[16] == 0 && disasm_buf[17] != 0) {
        asset_table_offset = get_be16(disasm_buf + 16);
    }

    if (asset_table_offset == 0 || asset_table_offset > disasm_size) {
        asset_table_offset = disasm_size;
    }

    fprintf(disasm_fout, "; ALIS Script Disassembly\n");
    fprintf(disasm_fout, "; Header Info\n");
    fprintf(disasm_fout, "; Code offset: 0x%X\n", code_loc_offset);
    fprintf(disasm_fout, "; Asset table offset: 0x%X\n\n", asset_table_offset);

    char header_name[256];
    sprintf(header_name, "%s_header.bin", argv[2]);
    FILE *fheader = fopen(header_name, "wb");
    if (fheader) {
        fwrite(disasm_buf, 1, code_loc_offset, fheader);
        fclose(fheader);
    }
    fprintf(disasm_fout, ".incbin \"%s\"\n\n", header_name);

    int code_start = code_loc_offset;
    uint32_t code_end = asset_table_offset;

    disasm_pass = 1;
    disasm_pc = code_start;
    while (disasm_pc < code_end) {
        int last_pc = disasm_pc;
        readexec_opcodes();
        if (disasm_pc == last_pc) break;
        if (disasm_pc >= code_end) break;
    }

    disasm_pass = 2;
    disasm_pc = code_start;
    while (disasm_pc < code_end) {
        int last_pc = disasm_pc;
        readexec_opcodes();
        if (disasm_pc == last_pc) break;
    }

    if (code_end < disasm_size) {
        fprintf(disasm_fout, "\n; --- ASSETS ---\n");
        char assets_name[256];
        sprintf(assets_name, "%s_assets.bin", argv[2]);
        FILE *fassets = fopen(assets_name, "wb");
        if (fassets) {
            fwrite(disasm_buf + code_end, 1, disasm_size - code_end, fassets);
            fclose(fassets);
        }
        fprintf(disasm_fout, ".incbin \"%s\"\n", assets_name);
    }

    fclose(disasm_fout);
    free(disasm_buf);
    return 0;
}
