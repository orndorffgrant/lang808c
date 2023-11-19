#include "linker.h"
#include "armv6m.h"

int link(SymbolTable *symbols, MachineCode *code, uint8_t *dest) {
    int len = 0;
    // for all code in each function, assign address
    for (int i = 0; i < symbols->functions_num; i++) {
        for (int j = 0; j < code->functions[i].len; j++) {
            code->functions[i].ops[j].address = len;
            len += 2; // every instruction is two bytes;
        }
    }
    // for all code in each function, find BLs and fill in
    for (int i = 0; i < symbols->functions_num; i++) {
        for (int j = 0; j < code->functions[i].len; j++) {
            ARMv6Op *op = &code->functions[i].ops[j];
            if (op->target_function) {
                int curr_address = op->address;
                int target_address = code->functions[op->target_function].ops[0].address;
                int offset = (target_address - curr_address) - 4; // TODO double check -4
                // TODO surgery on the actual instruction
            }
        }
    }
    // construct vector table, add to dest
    // for all code in each function, put in dest
    return len;
}
void hex(uint8_t *code, int len);