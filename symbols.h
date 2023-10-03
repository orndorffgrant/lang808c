#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "common.h"

typedef enum {
    int_u8,
    int_u16,
    int_u32
} IntType;

typedef struct _BitEnumItem {
    StringRef name;
    int value;
} BitEnumItem;
typedef struct _BitEnum {
    int width;
    int be_items_index;
    int be_items_len;
} BitEnum;
typedef enum {
    bfi_int,
    bfi_enum,
    bfi_unused
} BitFieldItemType;
typedef struct _BitFieldItem {
    BitFieldItemType type;
    StringRef name;
    int width;
    BitEnum be;
} BitFieldItem;
typedef struct _BitField {
    int width;
    int bf_items_index;
    int bf_items_len;
} BitField;
typedef enum {
    si_int,
    si_bf,
    si_unused
} StructItemType;
typedef struct _StructItem {
    // TODO add address
    StructItemType type;
    StringRef name;
    IntType int_type;
    BitField bf;
} StructItem;
typedef struct _MemoryMappedPeripheral {
    StringRef name;
    int base_address;
    int interrupt_number;
    int struct_items_index;
    int struct_items_len;
} MemoryMappedPeripheral;

typedef struct _Variable {
    StringRef name;
    IntType int_type;
    int initial_value;
} Variable;

typedef struct _FunctionArg {
    StringRef name;
    IntType int_type;
} FunctionArg;
typedef struct _Function {
    StringRef name;
    int func_args_index;
    int func_args_len;
    int func_vars_index;
    int func_vars_len;
    bool returns;
    IntType return_type;
} Function;
typedef struct _InterruptHandler {
    int interrupt_number;
    int func_index;
} InterruptHandler;

typedef struct _SymbolTable {
    MemoryMappedPeripheral mmps[1024];
    int mmps_num;
    StructItem struct_items[1024];
    int struct_items_num;
    BitFieldItem bitfield_items[1024];
    int bitfield_items_num;
    BitEnumItem bitenum_items[1024];
    int bitenum_items_num;

    Variable static_vars[1024];
    int static_vars_num;
    Function functions[1024];
    int functions_num;
    FunctionArg func_args[1024];
    int func_args_num;
    Variable function_vars[1024];
    int function_vars_num;

    InterruptHandler interrupt_handlers[1024];
    int interrupt_handlers_num;
} SymbolTable;

int add_mmp(SymbolTable *symbols, MemoryMappedPeripheral item);
int add_struct_item(SymbolTable *symbols, StructItem item);
int add_bitfield_item(SymbolTable *symbols, BitFieldItem item);
int add_bitenum_item(SymbolTable *symbols, BitEnumItem item);
int add_function(SymbolTable *symbols, Function item);
int add_function_arg(SymbolTable *symbols, FunctionArg item);
int add_static_variable(SymbolTable *symbols, Variable item);
int add_function_variable(SymbolTable *symbols, Variable item);

int find_mmp_index(SymbolTable *symbols, StringRef *name);
int find_struct_item_index(SymbolTable *symbols, int mmp_index, StringRef *name);
int find_bitfield_item_index(SymbolTable *symbols, int si_index, StringRef *name);
int find_bitenum_item_index(SymbolTable *symbols, int bfi_index, StringRef *name);
int find_function_index(SymbolTable *symbols, StringRef *name);
int find_function_arg(SymbolTable *symbols, int func_index, StringRef *name);
int find_function_variable(SymbolTable *symbols, int func_index, StringRef *name);
int find_static_variable(SymbolTable *symbols, StringRef *name);

#endif