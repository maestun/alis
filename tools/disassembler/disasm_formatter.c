#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern uint8_t *disasm_buf;
extern int disasm_size;
extern int disasm_pc;
extern FILE *disasm_fout;

int current_opcode_start = 0;
int pending_labels[10000];
int pending_labels_count = 0;

void add_label(int target) {
    if (target < 0 || target >= disasm_size) return;
    for (int i=0; i<pending_labels_count; i++) {
        if (pending_labels[i] == target) return;
    }
    pending_labels[pending_labels_count++] = target;
}

int has_label(int target) {
    for (int i=0; i<pending_labels_count; i++) {
        if (pending_labels[i] == target) return 1;
    }
    return 0;
}

void get8_raw() { if (disasm_pc < disasm_size) disasm_pc++; }
void get16_raw() { get8_raw(); get8_raw(); }
void get24_raw() { get8_raw(); get8_raw(); get8_raw(); }

int get_branch_target(int size) {
    int offset = 0;
    if (size == 1) {
        offset = (int8_t)disasm_buf[disasm_pc];
    } else if (size == 2) {
        offset = (int16_t)((disasm_buf[disasm_pc] << 8) | disasm_buf[disasm_pc+1]);
    } else if (size == 3) {
        offset = (int32_t)((disasm_buf[disasm_pc] << 16) | (disasm_buf[disasm_pc+1] << 8) | disasm_buf[disasm_pc+2]);
        if (offset & 0x800000) offset |= 0xff000000;
    }
    return current_opcode_start + offset;
}
