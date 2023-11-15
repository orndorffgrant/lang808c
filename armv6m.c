#include "armv6m.h"
#include "common.h"
#include "ir.h"
#include "symbols.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NVIC_ISER 0xe000e100
#define NVIC_ICPR 0xe000e280

#define R_ARG1 0
#define R_ARG2_DEST 1 // could probably combine arg2 and dest registers
#define R_TEMP_OFFSET 2
#define R_SP 13

#define C_ALWAYS 0b1110
#define C_EQUALS 0b0000
#define C_LESSTHAN 0b1011

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
#define ORRS_OPCODE 0b0100001100
#define ORRS_OPCODE_OFFSET 6
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
#define CMP_OPCODE 0b0100001010
#define CMP_OPCODE_OFFSET 6
#define MRS_INIT 0b1111001111101111
#define MRS_OPCODE 0b1000
#define MRS_OPCODE_OFFSET 12
#define PUSH_OPCODE 0b1011010
#define PUSH_OPCODE_OFFSET 9
#define POP_OPCODE 0b1011110
#define POP_OPCODE_OFFSET 9
#define BL_INIT_OPCODE 0b11110
#define BL_INIT_OPCODE_OFFSET 11
#define BL_FIN_OPCODE 0b11010
#define BL_FIN_OPCODE_OFFSET 11
#define B_OPCODE 0b1101
#define B_OPCODE_OFFSET 12
#define B_ALWAYS_OPCODE 0b11100
#define B_ALWAYS_OPCODE_OFFSET 11

void print_op_machine_code(SymbolTable *symbols, ARMv6Op *op, int i);

int next_label = 0;
void add_armv6m_inst(ARMv6Op op, MachineCodeFunction *code_func) {
    if (next_label) {
        op.label = next_label;
        next_label = 0;
    }
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
void orrs(int rd, int rm, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (ORRS_OPCODE << ORRS_OPCODE_OFFSET) | (rm << 3) | (rd);
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
void cmp(int rm, int rn, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (CMP_OPCODE << CMP_OPCODE_OFFSET) | (rm << 3) | (rn);
    add_armv6m_inst(op, code_func);
}
void mrs(int rd, int spec_reg, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = MRS_INIT;
    add_armv6m_inst(op, code_func);
    op.code = (MRS_OPCODE << MRS_OPCODE_OFFSET) | (rd << 8) | (spec_reg);
    add_armv6m_inst(op, code_func);
}
void push(int r, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (PUSH_OPCODE << PUSH_OPCODE_OFFSET) | (1 << r);
    add_armv6m_inst(op, code_func);
}
void push_lr(MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (PUSH_OPCODE << PUSH_OPCODE_OFFSET) | (1 << 8);
    add_armv6m_inst(op, code_func);
}
void pop(int r, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (POP_OPCODE << POP_OPCODE_OFFSET) | (1 << r);
    add_armv6m_inst(op, code_func);
}
void pop_pc(MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.code = (POP_OPCODE << POP_OPCODE_OFFSET) | (1 << 8);
    add_armv6m_inst(op, code_func);
}
void bl(int target_function, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.target_function = target_function;
    op.code = (BL_INIT_OPCODE << BL_INIT_OPCODE_OFFSET);
    add_armv6m_inst(op, code_func);
    op.target_function = 0;
    op.code = (BL_FIN_OPCODE << BL_FIN_OPCODE_OFFSET);
    add_armv6m_inst(op, code_func);
}
void b(int cond, int target_label, MachineCodeFunction *code_func) {
    ARMv6Op op = {0};
    op.target_label = target_label;
    if (cond == C_ALWAYS) {
        op.code = (B_ALWAYS_OPCODE << B_ALWAYS_OPCODE_OFFSET);
    } else {
        op.code = (B_OPCODE << B_OPCODE_OFFSET) | (cond << 8);
    }
    add_armv6m_inst(op, code_func);
}

void immediate_to_rX(uint32_t imm, int r, MachineCodeFunction *code_func) {
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
            int sp_offset = (func_arg_num + 1) * 4;
            mov_r(R_ARG2_DEST, R_SP, code_func);
            if (func_arg->int_type == int_u8) {
                ldrb(r, R_ARG2_DEST, sp_offset, code_func);
            } else if (func_arg->int_type == int_u16) {
                ldrh(r, R_ARG2_DEST, sp_offset >> 1, code_func);
            } else if (func_arg->int_type == int_u32) {
                ldr(r, R_ARG2_DEST, sp_offset >> 2, code_func);
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

int next_condition = C_ALWAYS;
void ir_to_armv6m_inst(SymbolTable *symbols, IROp *ir_op, MachineCodeFunction *code_func, int func_index) {
    if (ir_op->label) {
        next_label = ir_op->label;
    }
    switch (ir_op->opcode) {

        // Data operations
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

        // Comparison
        case ir_equals: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            cmp(rn, rm, code_func);
            // mrs(rd, 0, code_func);
            next_condition = C_EQUALS;
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }
        case ir_less_than: {
            int rn = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            int rm = arg_to_rX(symbols, &ir_op->arg2, R_ARG2_DEST, code_func);
            int rd = result_rx(&ir_op->result);
            cmp(rm, rn, code_func);
            // mrs(rd, 0, code_func);
            next_condition = C_LESSTHAN;
            rx_to_result(symbols, &ir_op->result, rd, code_func);
            break;
        }

        // Branches
        case ir_goto: {
            b(C_ALWAYS, ir_op->target_label, code_func);
            break;
        }
        case ir_if: {
            b(next_condition, ir_op->target_label, code_func);
            next_condition = C_ALWAYS;
            break;
        }

        // Copy
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

        // Functions
        case ir_param: {
            int r = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            push(r, code_func);
            break;
        }
        case ir_call: {
            Function *func = &symbols->functions[ir_op->arg1.func_index];
            int r = result_rx(&ir_op->result);
            bl(ir_op->arg1.func_index, code_func);
            for (int i = 0; i < func->func_args_len; i++) {
                pop(R_ARG2_DEST, code_func);
            }
            if (r != 0) {
                mov_r(r, 0, code_func);
            }
            rx_to_result(symbols, &ir_op->result, r, code_func);
            break;
        }
        case ir_return: {
            // find interrupt handler if it exists
            for (int i = 0; i < symbols->interrupt_handlers_num; i++) {
                InterruptHandler *handler = &symbols->interrupt_handlers[i];
                if (handler->func_index == func_index) {
                    // This function is a handler, clear the interrupt flag
                    immediate_to_rX(NVIC_ICPR, R_ARG1, code_func);
                    ldr(R_ARG2_DEST, R_ARG1, 0, code_func);
                    mov(2, 1, code_func);
                    lsls(2, 2, handler->interrupt_number, code_func);
                    orrs(R_ARG2_DEST, 2, code_func);
                    str(R_ARG2_DEST, R_ARG1, 0, code_func);
                }
            }
            // normal return
            int r = arg_to_rX(symbols, &ir_op->arg1, R_ARG1, code_func);
            mov_r(0, r, code_func);
            pop_pc(code_func);
            break;
        }
        default: PANIC("IR OP: %x\n", ir_op->opcode);
    }
}

void ir_to_armv6m_function(SymbolTable *symbols, MachineCodeFunction *code_func, int func_index) {
    Function *func = &symbols->functions[func_index];
    push_lr(code_func);
    for (int i = 0; i < func->ir_code_len; i++) {
        ir_to_armv6m_inst(symbols, &symbols->ir_code[func->ir_code_index + i], code_func, func_index);
    }
}

void fill_local_branches(MachineCodeFunction *code_func) {
    for (int i = 0; i < code_func->len; i++) {
        if (code_func->ops[i].target_label) {
            ARMv6Op *branch = &code_func->ops[i];
            for (int j = 0; j < code_func->len; j++) {
                if (code_func->ops[j].label == branch->target_label) {
                    if ((branch->code >> B_OPCODE_OFFSET) == B_OPCODE) {
                        // technically *2, but lose last bit so /2
                        // Also PC is already +4 beyond current instruction, so -2
                        int8_t offset = (j - i) - 2;
                        branch->code |= (uint8_t)offset;
                    } else { // B_ALWAYS
                        // technically *2, but lose last bit so /2
                        // Also PC is already +4 beyond current instruction, so -2
                        int16_t offset = (j - i) - 2;
                        branch->code |= (((uint16_t)offset) & 0b0000011111111111);
                    }
                }
            }
        }
    }
}

void ir_to_armv6m(SymbolTable *symbols, MachineCode *code) {
    for (int i = 0; i < symbols->functions_num; i++) {
        ir_to_armv6m_function(symbols, &code->functions[i], i);
    }

    // Enable interrupts at end of ____init if we have any
    MachineCodeFunction *init_code = &code->functions[0];
    if (symbols->interrupt_handlers_num > 0) {
        immediate_to_rX(NVIC_ISER, R_ARG1, init_code);
        ldr(R_ARG2_DEST, R_ARG1, 0, init_code);
        for (int i = 0; i < symbols->interrupt_handlers_num; i++) {
            InterruptHandler *int_handler = &symbols->interrupt_handlers[i];
            mov(2, 1, init_code);
            lsls(2, 2, int_handler->interrupt_number, init_code);
            orrs(R_ARG2_DEST, 2, init_code);
        }
        str(R_ARG2_DEST, R_ARG1, 0, init_code);
    }
    // loop forever
    next_label = 99999;
    b(C_ALWAYS, 99999, init_code);

    // fill in branches
    for (int i = 0; i < symbols->functions_num; i++) {
        fill_local_branches(&code->functions[i]);
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

bool double_op = false;
uint16_t op_init = 0;
ARMv6Op op_init_op = {0};
void print_op_machine_code(SymbolTable *symbols, ARMv6Op *op, int i) {
    if (!double_op) {
        if (op->target_label) {
            printf("<");
        } else {
            printf(" ");
        }
        if (op->target_label || op->label) {
            printf("-");
        } else {
            printf(" ");
        }
        if (op->label) {
            printf(">");
        } else {
            printf(" ");
        }
        printf("%3d: ", i << 1);
    } else {
        double_op = false;
    }
    if (op->code == MRS_INIT) {
        op_init = MRS_INIT;
        op_init_op = *op;
        double_op = true;
    } else if ((op_init == MRS_INIT) && ((op->code >> MRS_OPCODE_OFFSET) == MRS_OPCODE)) {
        printf(
            "MRS R%d, Spec0x%x      ",
            (op->code & 0b0000111100000000) >> 8,
            (op->code & 0b0000000011111111) >> 0
        );
        printf("\t(");
        print_uint16_t_binary(MRS_INIT);
        printf("  ");
        print_uint16_t_binary(op->code);
        printf(")\n");
        op_init = 0;
    } else if ((op->code >> BL_INIT_OPCODE_OFFSET) == BL_INIT_OPCODE) {
        op_init = op->code;
        op_init_op = *op;
        double_op = true;
    } else if (
        ((op_init >> BL_INIT_OPCODE_OFFSET) == BL_INIT_OPCODE)
        && ((op->code >> BL_FIN_OPCODE_OFFSET) == BL_FIN_OPCODE)
    ) {
        Function *func = &symbols->functions[op_init_op.target_function];
        STRINGREF_TO_CSTR1(&func->name, 512);
        printf("BL %s              ", cstr1);
        printf("\t(");
        print_uint16_t_binary(op_init);
        printf("  ");
        print_uint16_t_binary(op->code);
        printf(")\n");
        op_init = 0;
    } else if ((op->code >> B_OPCODE_OFFSET) == B_OPCODE) {
        int cond = op->code & 0b111100000000;
        cond = cond >> 8;
        uint16_t offset = op->code & 0b11111111;
        offset = offset << 1;
        if (offset & 0b100000000) {
            offset |= 0b1111111000000000;
        }
        int16_t offset_s = offset + 4;
        if (cond == C_EQUALS) {
            printf("BEQ %d               ", offset_s);
        } else if (cond == C_LESSTHAN) {
            printf("BLT %d               ", offset_s);
        }
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> B_ALWAYS_OPCODE_OFFSET) == B_ALWAYS_OPCODE) {
        uint16_t offset = op->code & 0b11111111111;
        offset = offset << 1;
        if (offset & 0b100000000000) {
            offset |= 0b1111000000000000;
        }
        int16_t offset_s = offset + 4;
        printf("B %d                 ", offset_s);
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> ADDS_OPCODE_OFFSET) == ADDS_OPCODE) {
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
    } else if ((op->code >> ORRS_OPCODE_OFFSET) == ORRS_OPCODE) {
        printf(
            "ORRS R%d, R%d          ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> STR_OPCODE_OFFSET) == STR_OPCODE) {
        printf(
            "STR R%d, [R%d + #0x%x]  ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            ((op->code & 0b0000011111000000) >> 6) << 2
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> STRH_OPCODE_OFFSET) == STRH_OPCODE) {
        printf(
            "STRH R%d, [R%d + #0x%x] ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            ((op->code & 0b0000011111000000) >> 6) << 1
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
            ((op->code & 0b0000011111000000) >> 6) << 2
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> LDRH_OPCODE_OFFSET) == LDRH_OPCODE) {
        printf(
            "LDRH R%d, [R%d + #0x%x] ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3,
            ((op->code & 0b0000011111000000) >> 6) << 1
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
    } else if ((op->code >> CMP_OPCODE_OFFSET) == CMP_OPCODE) {
        printf(
            "CMP R%d, R%d           ",
            (op->code & 0b0000000000000111) >> 0,
            (op->code & 0b0000000000111000) >> 3
        );
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> PUSH_OPCODE_OFFSET) == PUSH_OPCODE) {
        int registers = op->code & 0b111111111;
        // we only support one register at a time
        int r = -1;
        for (int i = 0; i < 9; i++) {
            if ((registers >> i) == 1) {
                r = i;
                break;
            }
        }
        if (r == 8) {
            printf("PUSH LR              ");
        } else {
            printf("PUSH R%d              ", r);
        }
        printf("\t("); print_uint16_t_binary(op->code); printf(")\n");
    } else if ((op->code >> POP_OPCODE_OFFSET) == POP_OPCODE) {
        int registers = op->code & 0b111111111;
        // we only support one register at a time
        int r = -1;
        for (int i = 0; i < 9; i++) {
            if ((registers >> i) == 1) {
                r = i;
                break;
            }
        }
        if (r == 8) {
            printf("POP PC               ");
        } else {
            printf("POP R%d               ", r);
        }
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
        print_op_machine_code(symbols, &code_func->ops[i], i);
    }
    printf("\n");
}
void print_all_machine_code(SymbolTable *symbols, MachineCode *code) {
    for (int i = 0; i < symbols->functions_num; i++) {
        print_function_machine_code(symbols, &code->functions[i], i);
    }
}

void write_function_object_code(SymbolTable *symbols, MachineCode *code) {
    for (int i = 0; i < symbols->functions_num; i++) {
        Function *func = &symbols->functions[i];
        MachineCodeFunction *code_func = &code->functions[i];
        STRINGREF_TO_CSTR1(&func->name, 512);
        char file_name[544] = {0};
        sprintf(file_name, "out-%s.o", cstr1);
        FILE *object_file = fopen(file_name, "w");
        for (int j = 0; j < code_func->len; j++) {
            fwrite(&code_func->ops[j].code, 2, 1, object_file);
        }
        fclose(object_file);
    }
}