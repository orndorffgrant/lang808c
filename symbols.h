#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "common.h"
#include "ir.h"

// An enum for the different supported integer types
typedef enum {
    int_u8,
    int_u16,
    int_u32
} IntType;

// A struct representing one item in a BitEnum. e.g. "value = 4;"
typedef struct _BitEnumItem {
    StringRef name;
    int value;
} BitEnumItem;

// A struct representing a BitEnum.
// It references its values by indexes into the array of BitEnumItems in the SymbolTable.
typedef struct _BitEnum {
    int width;
    int be_items_index;
    int be_items_len;
} BitEnum;

// An enum for the different types of sub-fields allowed in a BitField
typedef enum {
    bfi_int,
    bfi_enum,
    bfi_unused
} BitFieldItemType;

// A struct representing one item in a BitField. e.g. "sub_field: 2;"
typedef struct _BitFieldItem {
    BitFieldItemType type;
    StringRef name;
    int width;
    BitEnum be;
} BitFieldItem;

// A struct representing a BitField.
// It references its sub-fields by indexes into the array of BitFieldItems in the SymbolTable.
typedef struct _BitField {
    int width;
    int bf_items_index;
    int bf_items_len;
} BitField;

// An enum for the different types of fields allowed in a MemoryMappedPeripheral struct
typedef enum {
    si_int,
    si_bf,
    si_unused
} StructItemType;

// A struct representing one item in a MemoryMappedPeripheral struct. e.g. "field: u16;"
typedef struct _StructItem {
    // TODO add address
    StructItemType type;
    StringRef name;
    IntType int_type;
    BitField bf;
} StructItem;

// A struct representing a MemoryMappedPeripheral.
// It references its fields by indexes into the array of StructItems in the SymbolTable.
typedef struct _MemoryMappedPeripheral {
    StringRef name;
    int base_address;
    int interrupt_number;
    int struct_items_index;
    int struct_items_len;
} MemoryMappedPeripheral;

// A struct representing a variable, static or local
typedef struct _Variable {
    StringRef name;
    IntType int_type;
    int initial_value;
} Variable;

// A struct representing one argument to a function
typedef struct _FunctionArg {
    StringRef name;
    IntType int_type;
} FunctionArg;

// A struct representing one function
// It references its arguments by indexes into the array of FunctionArgs in the SymbolTable.
// It references its local variables by indexes into the array of local Variables in the SymbolTable.
typedef struct _Function {
    StringRef name;
    int func_args_index;
    int func_args_len;
    int func_vars_index;
    int func_vars_len;
    bool returns;
    IntType return_type;
    int ir_code_index;
    int ir_code_len;
} Function;

// A struct representing an interrupt handler
// It references its function by an index into the array of Functions in the SymbolTable.
typedef struct _InterruptHandler {
    int interrupt_number;
    int func_index;
} InterruptHandler;

// All types of symbols are kept in flat arrays
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

    IROp ir_code[65536];
    int ir_len;
} SymbolTable;


// These helper functions are defined in symbols.c

int add_mmp(SymbolTable *symbols, MemoryMappedPeripheral item);
int add_struct_item(SymbolTable *symbols, StructItem item);
int add_bitfield_item(SymbolTable *symbols, BitFieldItem item);
int add_bitenum_item(SymbolTable *symbols, BitEnumItem item);
int add_function(SymbolTable *symbols, Function item);
int add_function_arg(SymbolTable *symbols, FunctionArg item);
int add_static_variable(SymbolTable *symbols, Variable item);
int add_function_variable(SymbolTable *symbols, Variable item);

int add_function_ir(SymbolTable *symbols, int func_index, IROp item);

int find_mmp_index(SymbolTable *symbols, StringRef *name);
int find_struct_item_index(SymbolTable *symbols, int mmp_index, StringRef *name);
int find_bitfield_item_index(SymbolTable *symbols, int si_index, StringRef *name);
int find_bitenum_item_index(SymbolTable *symbols, int bfi_index, StringRef *name);
int find_function_index(SymbolTable *symbols, StringRef *name);
int find_function_arg(SymbolTable *symbols, int func_index, StringRef *name);
int find_function_variable(SymbolTable *symbols, int func_index, StringRef *name);
int find_static_variable(SymbolTable *symbols, StringRef *name);

#endif