#include "armv6m.h"
#include "ir.h"
#include "symbols.h"

#define ADDS_OPCODE 0b000110

void add_armv6m_inst(ARMv6Op op, MachineCodeFunction *code_func) {
    code_func->ops[code_func->len] = op;
    code_func->len++;
}

void adds(int rd, int rn, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ADDS_OPCODE << 10) | (rm << 6) | (rn << 3) | (rd);
    add_armv6m_inst(op, code_func);
}

void arg_to_rX(IRValue *arg, int r, MachineCodeFunction *code_func) {
    switch (arg->type) {
        case irv_immediate: {

        }
    }
}
void rx_to_result(IRValue *result, int r, MachineCodeFunction *code_func) {
}

void ir_to_armv6m_inst(IROp *ir_op, MachineCodeFunction *code_func) {
    switch (ir_op->opcode) {
        case ir_add: {
            arg_to_rX(&ir_op->arg1, 0, code_func);
            arg_to_rX(&ir_op->arg2, 1, code_func);
            adds(0, 0, 1, code_func);
            rx_to_result(&ir_op->arg2, 1, code_func);
            break;
        }

    }

}

void ir_to_armv6m_function(SymbolTable *symbols, MachineCodeFunction *code_func, int func_index) {
    Function *func = &symbols->functions[func_index];
    for (int i = 0; i < func->ir_code_len; i++) {
        ir_to_armv6m_inst(&symbols->ir_code[func->ir_code_index + i], code_func);
    }
}

void ir_to_armv6m(SymbolTable *symbols, MachineCode *code) {
    printf("Generating ARMv6-M machine code\n");
    for (int i = 0; i < symbols->functions_num; i++) {
        ir_to_armv6m_function(symbols, &code->functions[i], i);
    }
}
void print_op_machine_code(ARMv6Op *op) {
    if (op->label != 0) {
        printf("%2d: ", op->label);
    } else {
        printf("    ");
    }
    switch ((op->code >> 10) & 0b111111) {
        case ADDS_OPCODE:
            printf("ADDS R%d, R%d, R%d\n", (op->code >> 6) & 0b111, (op->code >> 3) & 0xb111, op->code & 0xb111);
            break;
        //default: PANIC("UNIDENTIFIED ARMv6-M OPCODE: %d\n", op->opcode);
    }
}
void print_function_machine_code(SymbolTable *symbols, MachineCodeFunction *code_func, int func_index) {
    Function *func = &symbols->functions[func_index];
    STRINGREF_TO_CSTR1(&func->name, 512);
    printf("function %s ARMv6-M\n", cstr1);
    for (int i = 0; i < code_func->len; i++) {
        print_op_machine_code(&code_func->ops[i]);
    }
    printf("\n");
}
void print_all_machine_code(SymbolTable *symbols, MachineCode *code) {
    for (int i = 0; i < symbols->functions_num; i++) {
        print_function_machine_code(symbols, &code->functions[i], i);
    }
}