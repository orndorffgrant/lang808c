#include <stdbool.h>

#include "symbols.h"
#include "common.h"

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
int find_function_variable(SymbolTable *symbols, int func_index, StringRef *name) {
    Function *func = &symbols->functions[func_index];
    int begin = func->func_vars_index;
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