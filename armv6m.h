#ifndef ARMV6M_H
#define ARMV6M_H

#include <stdint.h>

#include "symbols.h"

typedef struct _ARMv6Op {
    int address;
    int label;
    int target_label;
    int target_function;
    uint16_t code;
} ARMv6Op;

typedef struct _MachineCodeFunction {
    ARMv6Op ops[512];
    int len;
} MachineCodeFunction;

typedef struct _MachineCode {
    MachineCodeFunction functions[16];
} MachineCode;

void ir_to_armv6m(SymbolTable *symbols, MachineCode *code);
void print_all_machine_code(SymbolTable *symbols, MachineCode *code);
void write_function_object_code(SymbolTable *symbols, MachineCode *code);

#endif