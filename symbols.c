#include <stdbool.h>

#include "ir.h"
#include "symbols.h"
#include "common.h"


// These are helpers to add items to each of the arrays in the SymbolTable
int add_mmp(SymbolTable *symbols, MemoryMappedPeripheral item) {
  symbols->mmps[symbols->mmps_num] = item;
  return symbols->mmps_num++;
}
int add_struct_item(SymbolTable *symbols, StructItem item) {
  symbols->struct_items[symbols->struct_items_num] = item;
  return symbols->struct_items_num++;
}
int add_bitfield_item(SymbolTable *symbols, BitFieldItem item) {
  symbols->bitfield_items[symbols->bitfield_items_num] = item;
  return symbols->bitfield_items_num++;
}
int add_bitenum_item(SymbolTable *symbols, BitEnumItem item) {
  symbols->bitenum_items[symbols->bitenum_items_num] = item;
  return symbols->bitenum_items_num++;
}
int add_function(SymbolTable *symbols, Function item) {
  symbols->functions[symbols->functions_num] = item;
  return symbols->functions_num++;
}
int add_function_arg(SymbolTable *symbols, FunctionArg item) {
  symbols->func_args[symbols->func_args_num] = item;
  return symbols->func_args_num++;
}
int add_static_variable(SymbolTable *symbols, Variable item) {
  symbols->static_vars[symbols->static_vars_num] = item;
  return symbols->static_vars_num++;
}
int add_function_variable(SymbolTable *symbols, Variable item) {
  symbols->function_vars[symbols->function_vars_num] = item;
  return symbols->function_vars_num++;
}

int add_function_ir(SymbolTable *symbols, int func_index, IROp item) {
  symbols->ir_code[symbols->ir_len] = item;
  int index = symbols->ir_len;
  symbols->ir_len++;
  if (symbols->functions[func_index].ir_code_index == -1) {
    symbols->functions[func_index].ir_code_index = index;
  }
  symbols->functions[func_index].ir_code_len++;
}

// These are helpers to find items by name (StringRef) in each of the arrays in the SymbolTable
// They all return -1 if an item with the given name cannot be found
int find_mmp_index(SymbolTable *symbols, StringRef *name) {
    for (int i = 0; i < symbols->mmps_num; i++) {
        if (string_ref_eq(name, &symbols->mmps[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_struct_item_index(SymbolTable *symbols, int mmp_index, StringRef *name) {
    MemoryMappedPeripheral *mmp = &symbols->mmps[mmp_index];
    int begin = mmp->struct_items_index;
    if (begin == -1) {
        return -1;
    }
    int end = mmp->struct_items_index + mmp->struct_items_len;
    for (int i = begin; i < end; i++) {
        if (string_ref_eq(name, &symbols->struct_items[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_bitfield_item_index(SymbolTable *symbols, int si_index, StringRef *name) {
    StructItem *si = &symbols->struct_items[si_index];
    int begin = si->bf.bf_items_index;
    if (begin == -1) {
        return -1;
    }
    int end = si->bf.bf_items_index + si->bf.bf_items_len;
    for (int i = begin; i < end; i++) {
        if (string_ref_eq(name, &symbols->bitfield_items[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_bitenum_item_index(SymbolTable *symbols, int bfi_index, StringRef *name) {
    BitFieldItem *bfi = &symbols->bitfield_items[bfi_index];
    int begin = bfi->be.be_items_index;
    if (begin == -1) {
        return -1;
    }
    int end = bfi->be.be_items_index + bfi->be.be_items_len;
    for (int i = begin; i < end; i++) {
        if (string_ref_eq(name, &symbols->bitenum_items[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_function_index(SymbolTable *symbols, StringRef *name) {
    for (int i = 0; i < symbols->functions_num; i++) {
        if (string_ref_eq(name, &symbols->functions[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_function_arg(SymbolTable *symbols, int func_index, StringRef *name) {
    Function *func = &symbols->functions[func_index];
    int begin = func->func_args_index;
    if (begin == -1) {
        return -1;
    }
    int end = func->func_args_index + func->func_args_len;
    for (int i = begin; i < end; i++) {
        if (string_ref_eq(name, &symbols->func_args[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_function_variable(SymbolTable *symbols, int func_index, StringRef *name) {
    Function *func = &symbols->functions[func_index];
    int begin = func->func_vars_index;
    if (begin == -1) {
        return -1;
    }
    int end = func->func_vars_index + func->func_vars_len;
    for (int i = begin; i < end; i++) {
        if (string_ref_eq(name, &symbols->function_vars[i].name)) {
            return i;
        }
    }
    return -1;
}
int find_static_variable(SymbolTable *symbols, StringRef *name) {
    for (int i = 0; i < symbols->static_vars_num; i++) {
        if (string_ref_eq(name, &symbols->static_vars[i].name)) {
            return i;
        }
    }
    return -1;
}


void print_ir_value(SymbolTable *symbols, IRValue *value) {
    switch (value->type) {
        case irv_mmp_struct_item: {
            STRINGREF_TO_CSTR1(&symbols->mmps[value->mmp_index].name, 512);
            STRINGREF_TO_CSTR2(&symbols->struct_items[value->mmp_struct_item_index].name, 512);
            printf("0x%x(%s.%s)", symbols->struct_items[value->mmp_struct_item_index].address, cstr1, cstr2);
            break;
        }
        case irv_immediate: {
            printf("0x%x", value->immediate_value);
            break;
        }
        case irv_static_variable: {
            STRINGREF_TO_CSTR1(&symbols->static_vars[value->static_variable_index].name, 512);
            printf("static %s", cstr1);
            break;
        }
        case irv_local_variable: {
            STRINGREF_TO_CSTR1(&symbols->function_vars[value->local_variable_index].name, 512);
            printf("local %s", cstr1);
            break;
        }

        default: PANIC("UNIDENTIFIED IR VALUE: %d\n", value->type);
    }
}
void print_irop(SymbolTable *symbols, int irop_index) {
    IROp *op = &symbols->ir_code[irop_index];
    switch (op->opcode) {
        case ir_add:
            print_ir_value(symbols, &op->result);
            printf(" = ");
            print_ir_value(symbols, &op->arg1);
            printf(" + ");
            print_ir_value(symbols, &op->arg2);
            printf("\n");
            break;
        case ir_copy:
            print_ir_value(symbols, &op->result);
            printf(" = ");
            print_ir_value(symbols, &op->arg1);
            printf("\n");
            break;
        default: PANIC("UNIDENTIFIED IR OP: %d\n", op->opcode);
    }
}
void print_function_ir(SymbolTable *symbols, int func_index) {
    Function *func = &symbols->functions[func_index];
    STRINGREF_TO_CSTR1(&func->name, 512);
    printf("IR for function: %s\n", cstr1);
    for (int j = func->ir_code_index; j < (func->ir_code_index + func->ir_code_len); j++) {
        print_irop(symbols, j);
    }
    printf("\n");
}
void print_all_ir(SymbolTable *symbols) {
    for (int i = 0; i < symbols->functions_num; i++) {
        print_function_ir(symbols, i);
    }
}