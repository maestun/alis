#ifndef DISASM_FORMATTER_H
#define DISASM_FORMATTER_H

#include <stdint.h>

void add_label(int target);
int has_label(int target);
int get_branch_target(int size);

#endif
