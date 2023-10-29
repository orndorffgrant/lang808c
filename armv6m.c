#include "armv6m.h"
#include "ir.h"
#include "symbols.h"

#define ADDS_OPCODE 0b000110
#define MOV_OPCODE0 0b001000
#define MOV_OPCODE1 0b001001
#define LSLS_OPCODE0 0b000000
#define LSLS_OPCODE1 0b000001
#define DATA_OPCODE 0b010000
#define DATA_ANDS_OPCODE 0b0000

void add_armv6m_inst(ARMv6Op op, MachineCodeFunction *code_func) {
    code_func->ops[code_func->len] = op;
    code_func->len++;
}

void adds(int rd, int rn, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ADDS_OPCODE << 10) | (rm << 6) | (rn << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void mov(int rd, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (MOV_OPCODE0 << 10) | (rd << 8) | (imm);
    add_armv6m_inst(op, code_func);
}
void lsls(int rd, int rm, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LSLS_OPCODE0 << 10) | (imm << 6) | (rm << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void ands(int rd, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (DATA_OPCODE << 10) | (DATA_ANDS_OPCODE << 6) | (rm << 3) | (rd);
    add_armv6m_inst(op, code_func);
}

// Returns register that will have the arg value
int arg_to_rX(IRValue *arg, int r, MachineCodeFunction *code_func) {
    switch (arg->type) {
        case irv_temp: {
            return arg->temp_num + 2;
        }
        case irv_immediate: {
            if (arg->immediate_value <= 0xFF) {
                mov(r, arg->immediate_value, code_func);
                return r;
            } else if (arg->immediate_value <= 0xFFFF) {
                mov(r, (arg->immediate_value & 0xFF00) >> 8, code_func);
                lsls(r, r, 8, code_func);
                int extra_scratch_r = 1 ? r == 0 : 0;
                mov(extra_scratch_r, arg->immediate_value & 0xFF, code_func);
                ands(r, extra_scratch_r, code_func);
                return r;
            }
            // TODO
            return r;
        }
        // default: PANIC("UNHANDLED IR VALUE: %d\n", arg->type);
    }
}
int result_rx(IRValue *result) {
    if (result->type == irv_temp) {
        return result->temp_num + 2;
    }
    return 0;
}
void rx_to_result(IRValue *result, int r, MachineCodeFunction *code_func) {
    switch (result->type) {
        case irv_temp: {
            return;
        }
        case irv_immediate: {
            PANIC("IR RESULT CAN'T BE IMMEDIATE");
        }
        //default: PANIC("UNHANDLED IR VALUE: %d\n", arg->type);
    }
}

void ir_to_armv6m_inst(IROp *ir_op, MachineCodeFunction *code_func) {
    switch (ir_op->opcode) {
        case ir_add: {
            int rn = arg_to_rX(&ir_op->arg1, 0, code_func);
            int rm = arg_to_rX(&ir_op->arg2, 1, code_func);
            int rd = result_rx(&ir_op->result);
            adds(rd, rn, rm, code_func);
            rx_to_result(&ir_op->result, rd, code_func);
            break;
        }
        case ir_copy: {
            if (ir_op->result.type == irv_temp) {
                int rd = result_rx(&ir_op->result);
                int rn = arg_to_rX(&ir_op->arg1, rd, code_func);
            } else {
                int rn = arg_to_rX(&ir_op->arg1, 0, code_func);
                rx_to_result(&ir_op->result, rn, code_func);
            }
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
void print_uint16_t_binary(uint16_t i) {
    printf("%d", (i & 0x8000) >> 15);
    printf("%d", (i & 0x4000) >> 14);
    printf("%d", (i & 0x2000) >> 13);
    printf("%d", (i & 0x1000) >> 12);
    printf("%d", (i & 0x800) >> 11);
    printf("%d", (i & 0x400) >> 10);
    printf("%d", (i & 0x200) >> 9);
    printf("%d", (i & 0x100) >> 8);
    printf("%d", (i & 0x80) >> 7);
    printf("%d", (i & 0x40) >> 6);
    printf("%d", (i & 0x20) >> 5);
    printf("%d", (i & 0x10) >> 4);
    printf("%d", (i & 0x8) >> 3);
    printf("%d", (i & 0x4) >> 2);
    printf("%d", (i & 0x2) >> 1);
    printf("%d", i & 0x1);
}
void print_op_machine_code(ARMv6Op *op) {
    if (op->label != 0) {
        printf("%2d: ", op->label);
    } else {
        printf("    ");
    }
    switch ((op->code >> 10) & 0b111111) {
        case ADDS_OPCODE:
            printf(
                "ADDS R%d, R%d, R%d",
                (op->code & 0b0000000000000111) >> 0,
                (op->code & 0b0000000000111000) >> 3,
                (op->code & 0b0000000111000000) >> 6
            );
            printf("\t(");
            print_uint16_t_binary(op->code);
            printf(")\n");
            break;
        case MOV_OPCODE0:
        case MOV_OPCODE1:
            printf(
                "MOV R%d, #0x%x",
                (op->code & 0b0000011100000000) >> 8,
                (op->code & 0b0000000011111111) >> 0
            );
            printf("\t(");
            print_uint16_t_binary(op->code);
            printf(")\n");
            break;
        case LSLS_OPCODE0:
        case LSLS_OPCODE1:
            printf(
                "LSLS R%d, R%d, #0x%x",
                (op->code & 0b0000000000000111) >> 0,
                (op->code & 0b0000000000111000) >> 3,
                (op->code & 0b0000011111000000) >> 6
            );
            printf("\t(");
            print_uint16_t_binary(op->code);
            printf(")\n");
            break;
        case DATA_OPCODE: {
            switch ((op->code >> 6) & 0b1111) {
                case DATA_ANDS_OPCODE:
                    printf(
                        "ANDS R%d, R%d",
                        (op->code & 0b0000000000000111) >> 0,
                        (op->code & 0b0000000000111000) >> 3
                    );
                    printf("\t(");
                    print_uint16_t_binary(op->code);
                    printf(")\n");
                    break;
                default: PANIC("UNIDENTIFIED ARMv6-M DATA OPCODE: %x\n", op->code);
            }
            break;
        }
        default: PANIC("UNIDENTIFIED ARMv6-M OPCODE: %x\n", op->code);
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