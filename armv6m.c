#include "armv6m.h"
#include "common.h"
#include "ir.h"
#include "symbols.h"
#include <stdlib.h>

#define R_ARG1 0
#define R_ARG2_DEST 1 // could probably combine arg2 and dest registers
#define R_TEMP_OFFSET 2

#define ADDS_OPCODE 0b000110
#define ADDS_OPCODE_OFFSET 10
#define ADDS_IMM_OPCODE 0b00110
#define ADDS_IMM_OPCODE_OFFSET 11
#define MOV_OPCODE 0b00100
#define MOV_OPCODE_OFFSET 11
#define LSLS_OPCODE 0b00000
#define LSLS_OPCODE_OFFSET 11
#define ANDS_OPCODE 0b0100000000
#define ANDS_OPCODE_OFFSET 6
#define STR_OPCODE 0b01100
#define STR_OPCODE_OFFSET 11
#define STRH_OPCODE 0b10000
#define STRH_OPCODE_OFFSET 11
#define STRB_OPCODE 0b01110
#define STRB_OPCODE_OFFSET 11

void print_op_machine_code(ARMv6Op *op);

void add_armv6m_inst(ARMv6Op op, MachineCodeFunction *code_func) {
    code_func->ops[code_func->len] = op;
    code_func->len++;
}

void adds(int rd, int rn, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ADDS_OPCODE << ADDS_OPCODE_OFFSET) | (rm << 6) | (rn << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void adds_imm(int rdn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ADDS_IMM_OPCODE << ADDS_IMM_OPCODE_OFFSET) | (rdn << 8) | (imm);
    add_armv6m_inst(op, code_func);
}
void mov(int rd, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (MOV_OPCODE << MOV_OPCODE_OFFSET) | (rd << 8) | (imm);
    add_armv6m_inst(op, code_func);
}
void lsls(int rd, int rm, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LSLS_OPCODE << LSLS_OPCODE_OFFSET) | (imm << 6) | (rm << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void ands(int rd, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ANDS_OPCODE << ANDS_OPCODE_OFFSET) | (rm << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void str(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (STR_OPCODE << STR_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
    add_armv6m_inst(op, code_func);
}
void strh(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (STRH_OPCODE << STRH_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
    add_armv6m_inst(op, code_func);
}
void strb(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (STRB_OPCODE << STRB_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
    add_armv6m_inst(op, code_func);
}

void immediate_to_rX(int imm, int r, MachineCodeFunction *code_func) {
    if (imm <= 0xFF) {
        mov(r, imm, code_func);
    } else if (imm <= 0xFFFF) {
        mov(r, (imm & 0xFF00) >> 8, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, imm & 0xFF, code_func);
    } else if (imm <= 0xFFFFFF) {
        mov(r, (imm & 0xFF0000) >> 16, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, (imm & 0xFF00) >> 8, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, imm & 0xFF, code_func);
    } else if (imm <= 0xFFFFFFFF) {
        mov(r, (imm & 0xFF000000) >> 24, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, (imm & 0xFF0000) >> 16, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, (imm & 0xFF00) >> 8, code_func);
        lsls(r, r, 8, code_func);
        adds_imm(r, imm & 0xFF, code_func);
    }
}

// Returns register that will have the arg value
int arg_to_rX(IRValue *arg, int r, MachineCodeFunction *code_func) {
    switch (arg->type) {
        case irv_temp: {
            return arg->temp_num + R_TEMP_OFFSET;
        }
        case irv_immediate: {
            immediate_to_rX(arg->immediate_value, r, code_func);
            return r;
        }
        // default: PANIC("UNHANDLED IR VALUE: %d\n", arg->type);
    }
}
int result_rx(IRValue *result) {
    if (result->type == irv_temp) {
        return result->temp_num + R_TEMP_OFFSET;
    }
    return R_ARG1;
}
void rx_to_result(SymbolTable *symbols, IRValue *result, int r, MachineCodeFunction *code_func) {
    switch (result->type) {
        case irv_temp: {
            return;
        }
        case irv_immediate:
            PANIC("IR RESULT CAN'T BE IMMEDIATE");
        case irv_function:
            PANIC("IR RESULT CAN'T BE FUNCTION");
        case irv_function_argument:
            PANIC("IR RESULT CAN'T BE FUNCTION ARGUMENT");
        case irv_mmp_struct_item: {
            StructItem *si = &symbols->struct_items[result->mmp_struct_item_index];
            immediate_to_rX(si->address, R_ARG2_DEST, code_func);
            int width = 0;
            if (si->type == si_bf) {
                width = si->bf.width;
            } else if (si->int_type == int_u8) {
                width = 8;
            } else if (si->int_type == int_u16) {
                width = 16;
            } else if (si->int_type == int_u32) {
                width = 32;
            }
            if (width == 8) {
                strb(r, R_ARG2_DEST, 0, code_func);
            } else if (width == 16) {
                strh(r, R_ARG2_DEST, 0, code_func);
            } else if (width == 32) {
                str(r, R_ARG2_DEST, 0, code_func);
            } else {
                PANIC("INVALID WIDTH OF STRUCT ITEM\n");
            }
            return;
        }
        case irv_static_variable: {
            Variable *var = &symbols->static_vars[result->static_variable_index];
            immediate_to_rX(var->address, R_ARG2_DEST, code_func);
            if (var->int_type == int_u8) {
                strb(r, R_ARG2_DEST, 0, code_func);
            } else if (var->int_type == int_u16) {
                strh(r, R_ARG2_DEST, 0, code_func);
            } else if (var->int_type == int_u32) {
                str(r, R_ARG2_DEST, 0, code_func);
            } else {
                PANIC("INVALID INT TYPE OF STATIC VARIABLE\n");
            }
            return;
        }
        //default: PANIC("UNHANDLED IR VALUE: %d\n", result->type);
    }
}

void ir_to_armv6m_inst(SymbolTable *symbols, IROp *ir_op, MachineCodeFunction *code_func) {
    switch (ir_op->opcode) {
        case ir_add: {
            int rn = arg_to_rX(&ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(&ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            adds(rd, rn, rm, code_func);
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_copy: {
            if (ir_op->result.type == irv_temp) {
                int rd = result_rx(&ir_op->result);
                int rn = arg_to_rX(&ir_op->arg1, rd, code_func);
            } else {
                int rn = arg_to_rX(&ir_op->arg1, R_ARG1, code_func);
                rx_to_result(symbols, &ir_op->result, rn, code_func);
            }
            break;
        }

    }

}

void ir_to_armv6m_function(SymbolTable *symbols, MachineCodeFunction *code_func, int func_index) {
    Function *func = &symbols->functions[func_index];
    for (int i = 0; i < func->ir_code_len; i++) {
        ir_to_armv6m_inst(symbols, &symbols->ir_code[func->ir_code_index + i], code_func);
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
    printf("%d ", (i & 0x1000) >> 12);
    printf("%d", (i & 0x800) >> 11);
    printf("%d", (i & 0x400) >> 10);
    printf("%d", (i & 0x200) >> 9);
    printf("%d ", (i & 0x100) >> 8);
    printf("%d", (i & 0x80) >> 7);
    printf("%d", (i & 0x40) >> 6);
    printf("%d", (i & 0x20) >> 5);
    printf("%d ", (i & 0x10) >> 4);
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
    if ((op->code >> ADDS_OPCODE_OFFSET) == ADDS_OPCODE) {
        printf(
            "ADDS R%d, R%d, R%d      ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000000111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> ADDS_IMM_OPCODE_OFFSET) == ADDS_IMM_OPCODE) {
        printf(
            "ADDS R%d, #0x%x        ",
            (op->code & 0b0000011100000000) >> 8,
            (op->code & 0b0000000011111111) >> 0
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> MOV_OPCODE_OFFSET) == MOV_OPCODE) {
        printf(
            "MOV R%d, #0x%x         ",
            (op->code & 0b0000011100000000) >> 8,
            (op->code & 0b0000000011111111) >> 0
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> LSLS_OPCODE_OFFSET) == LSLS_OPCODE) {
        printf(
            "LSLS R%d, R%d, #0x%x    ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> ANDS_OPCODE_OFFSET) == ANDS_OPCODE) {
        printf(
            "ANDS R%d, R%d          ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> STR_OPCODE_OFFSET) == STR_OPCODE) {
        printf(
            "STR R%d, [R%d + #0x%x]  ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> STRH_OPCODE_OFFSET) == STRH_OPCODE) {
        printf(
            "STRH R%d, [R%d + #0x%x] ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> STRB_OPCODE_OFFSET) == STRB_OPCODE) {
        printf(
            "STRB R%d, [R%d + #0x%x] ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else {
        PANIC("UNIDENTIFIED ARMv6-M OPCODE: %x\n", op->code);
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