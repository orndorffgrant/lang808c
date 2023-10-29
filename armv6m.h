#ifndef ARMV6M_H
#define ARMV6M_H

#include <stdint.h>

#include "symbols.h"

typedef struct _ARMv6Op {
    int label;
    int target_label;
    uint16_t code;
} ARMv6Op;

typedef struct _MachineCodeFunction {
    ARMv6Op ops[1024];
    int len;
} MachineCodeFunction;

typedef struct _MachineCode {
    MachineCodeFunction functions[64];
} MachineCode;

void ir_to_armv6m(SymbolTable *symbols, MachineCode *code);
void print_all_machine_code(SymbolTable *symbols, MachineCode *code);

#endif