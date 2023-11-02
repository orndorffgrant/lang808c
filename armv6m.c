#include "armv6m.h"
#include "common.h"
#include "ir.h"
#include "symbols.h"
#include <stdlib.h>

#define R_ARG1 0
#define R_ARG2_DEST 1 // could probably combine arg2 and dest registers
#define R_TEMP_OFFSET 2
#define R_SP 13

#define ADDS_OPCODE 0b0001100
#define ADDS_OPCODE_OFFSET 9
#define ADDS_IMM_OPCODE 0b00110
#define ADDS_IMM_OPCODE_OFFSET 11
#define SUBS_OPCODE 0b0001101
#define SUBS_OPCODE_OFFSET 9
#define SUBS_IMM_OPCODE 0b00111
#define SUBS_IMM_OPCODE_OFFSET 11
#define MOV_OPCODE 0b00100
#define MOV_OPCODE_OFFSET 11
#define MOV_R_OPCODE 0b01000110
#define MOV_R_OPCODE_OFFSET 8
#define LSLS_OPCODE 0b00000
#define LSLS_OPCODE_OFFSET 11
#define LSLS_R_OPCODE 0b0100000010
#define LSLS_R_OPCODE_OFFSET 6
#define ANDS_OPCODE 0b0100000000
#define ANDS_OPCODE_OFFSET 6
#define STR_OPCODE 0b01100
#define STR_OPCODE_OFFSET 11
#define STRH_OPCODE 0b10000
#define STRH_OPCODE_OFFSET 11
#define STRB_OPCODE 0b01110
#define STRB_OPCODE_OFFSET 11
#define LDR_OPCODE 0b01101
#define LDR_OPCODE_OFFSET 11
#define LDRH_OPCODE 0b10001
#define LDRH_OPCODE_OFFSET 11
#define LDRB_OPCODE 0b01111
#define LDRB_OPCODE_OFFSET 11

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
void subs(int rd, int rn, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (SUBS_OPCODE << SUBS_OPCODE_OFFSET) | (rm << 6) | (rn << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void subs_imm(int rdn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (SUBS_IMM_OPCODE << SUBS_IMM_OPCODE_OFFSET) | (rdn << 8) | (imm);
    add_armv6m_inst(op, code_func);
}
void mov(int rd, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (MOV_OPCODE << MOV_OPCODE_OFFSET) | (rd << 8) | (imm);
    add_armv6m_inst(op, code_func);
}
void mov_r(int rd, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    int D = (rd & 0x8) >> 3;
    int rd_short = rd & 0x7;
    op.code = (MOV_R_OPCODE << MOV_R_OPCODE_OFFSET) | (D << 7) | (rm << 3) | (rd_short);
    add_armv6m_inst(op, code_func);
}
void lsls(int rd, int rm, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LSLS_OPCODE << LSLS_OPCODE_OFFSET) | (imm << 6) | (rm << 3) | (rd);
    add_armv6m_inst(op, code_func);
}
void lsls_r(int rdn, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LSLS_R_OPCODE << LSLS_R_OPCODE_OFFSET) | (rm << 3) | (rdn);
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
void ldr(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LDR_OPCODE << LDR_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
    add_armv6m_inst(op, code_func);
}
void ldrh(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LDRH_OPCODE << LDRH_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
    add_armv6m_inst(op, code_func);
}
void ldrb(int rt, int rn, int imm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (LDRB_OPCODE << LDRB_OPCODE_OFFSET) | (imm << 6) | (rn << 3) | (rt);
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
int arg_to_rX(SymbolTable *symbols, IRValue *arg, int r, MachineCodeFunction *code_func) {
    switch (arg->type) {
        case irv_temp: {
            return arg->temp_num + R_TEMP_OFFSET;
        }
        case irv_immediate: {
            immediate_to_rX(arg->immediate_value, r, code_func);
            return r;
        }
        case irv_function:
            PANIC("IR NON-FUNCTION ARG CAN'T BE FUNCTION");
        case irv_mmp_struct_item: {
            StructItem *si = &symbols->struct_items[arg->mmp_struct_item_index];
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
                ldrb(r, R_ARG2_DEST, 0, code_func);
            } else if (width == 16) {
                ldrh(r, R_ARG2_DEST, 0, code_func);
            } else if (width == 32) {
                ldr(r, R_ARG2_DEST, 0, code_func);
            } else {
                PANIC("INVALID WIDTH OF STRUCT ITEM\n");
            }
            return r;
        }
        case irv_static_variable: {
            Variable *var = &symbols->static_vars[arg->static_variable_index];
            immediate_to_rX(var->address, r, code_func);
            if (var->int_type == int_u8) {
                ldrb(r, r, 0, code_func);
            } else if (var->int_type == int_u16) {
                ldrh(r, r, 0, code_func);
            } else if (var->int_type == int_u32) {
                ldr(r, r, 0, code_func);
            } else {
                PANIC("INVALID INT TYPE OF STATIC VARIABLE\n");
            }
            return r;
        }
        case irv_local_variable: {
            Function *func = &symbols->functions[arg->func_index];
            Variable *var = &symbols->function_vars[arg->local_variable_index];
            int local_var_num = arg->local_variable_index - func->func_vars_index;
            int sp_offset = (local_var_num + 1) * 4;
            mov_r(R_ARG2_DEST, R_SP, code_func);
            subs_imm(R_ARG2_DEST, sp_offset, code_func);
            if (var->int_type == int_u8) {
                ldrb(r, R_ARG2_DEST, 0, code_func);
            } else if (var->int_type == int_u16) {
                ldrh(r, R_ARG2_DEST, 0, code_func);
            } else if (var->int_type == int_u32) {
                ldr(r, R_ARG2_DEST, 0, code_func);
            } else {
                PANIC("INVALID INT TYPE OF STATIC VARIABLE\n");
            }
            return r;
        }
        case irv_function_argument: {
            Function *func = &symbols->functions[arg->func_index];
            FunctionArg *func_arg = &symbols->func_args[arg->func_arg_index];
            int func_arg_num = arg->func_arg_index - func->func_args_index;
            int sp_offset = func_arg_num * 4;
            mov_r(R_ARG2_DEST, R_SP, code_func);
            if (func_arg->int_type == int_u8) {
                ldrb(r, R_ARG2_DEST, sp_offset, code_func);
            } else if (func_arg->int_type == int_u16) {
                ldrh(r, R_ARG2_DEST, sp_offset, code_func);
            } else if (func_arg->int_type == int_u32) {
                ldr(r, R_ARG2_DEST, sp_offset, code_func);
            } else {
                PANIC("INVALID INT TYPE OF STATIC VARIABLE\n");
            }
            return r;
        }
        default: PANIC("UNHANDLED ARG IR VALUE: %d\n", arg->type);
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
        case irv_local_variable: {
            Function *func = &symbols->functions[result->func_index];
            Variable *var = &symbols->function_vars[result->local_variable_index];
            int local_var_num = result->local_variable_index - func->func_vars_index;
            int sp_offset = (local_var_num + 1) * 4;
            mov_r(R_ARG2_DEST, R_SP, code_func);
            subs_imm(R_ARG2_DEST, sp_offset, code_func);
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
        default: PANIC("UNHANDLED RESULT IR VALUE: %d\n", result->type);
    }
}

void ir_to_armv6m_inst(SymbolTable *symbols, IROp *ir_op, MachineCodeFunction *code_func) {
    switch (ir_op->opcode) {
        case ir_add: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            adds(rd, rn, rm, code_func);
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_subtract: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            subs(rd, rn, rm, code_func);
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_shift_left: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            if (rd != rn) {
                lsls_r(rn, rm, code_func);
                mov_r(rd, rn, code_func);
            } else {
                lsls_r(rd, rm, code_func);
            }
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_bitwise_and: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            if (rd != rn) {
                ands(rn, rm, code_func);
                mov_r(rd, rn, code_func);
            } else {
                ands(rd, rm, code_func);
            }
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_copy: {
            if (ir_op->result.type == irv_temp) {
                int rd = result_rx(&ir_op->result);
                int rn = arg_to_rX(symbols, &ir_op->arg1, rd, code_func);
            } else {
                int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
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
    } else if ((op->code >> SUBS_OPCODE_OFFSET) == SUBS_OPCODE) {
        printf(
            "SUBS R%d, R%d, R%d      ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000000111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> SUBS_IMM_OPCODE_OFFSET) == SUBS_IMM_OPCODE) {
        printf(
            "SUBS R%d, #0x%x        ",
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
    } else if ((op->code >> MOV_R_OPCODE_OFFSET) == MOV_R_OPCODE) {
        int D = ((op->code & 0b10000000) >> 4);
        int rd_short = op->code & 0b111;
        int rd = D | rd_short;
        printf(
            "MOV R%d, R%d           ",
            rd,
            (op->code & 0b0000000001111000) >> 3
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
    } else if ((op->code >> LSLS_R_OPCODE_OFFSET) == LSLS_R_OPCODE) {
        printf(
            "LSLS R%d, R%d          ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3
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
    } else if ((op->code >> LDR_OPCODE_OFFSET) == LDR_OPCODE) {
        printf(
            "LDR R%d, [R%d + #0x%x]  ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> LDRH_OPCODE_OFFSET) == LDRH_OPCODE) {
        printf(
            "LDRH R%d, [R%d + #0x%x] ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            (op->code & 0b0000011111000000) >> 6
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> LDRB_OPCODE_OFFSET) == LDRB_OPCODE) {
        printf(
            "LDRB R%d, [R%d + #0x%x] ",
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