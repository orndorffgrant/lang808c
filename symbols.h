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

typedef struct _SymbolTable {
    MemoryMappedPeripheral mmps[1024];
    int mmps_num;
    StructItem struct_items[1024];
    int struct_items_num;
    BitField bitfields[1024];
    int bitfields_num;
    BitFieldItem bitfield_items[1024];
    int bitfield_items_num;
    BitEnum bitenums[1024];
    int bitenums_num;
    BitEnumItem bitenum_items[1024];
    int bitenum_items_num;


} SymbolTable;

#define ADD_SYMBOL_FN_DEF(symbol_name, symbol_type) int add_##symbol_name(\
    SymbolTable *symbols,\
    symbol_type item)
#define ADD_SYMBOL_FN(symbol_name, symbol_type) \
    ADD_SYMBOL_FN_DEF(symbol_name, symbol_type) {\
    symbols->symbol_name##s[symbols->symbol_name##s_num] = item;\
    return symbols->symbol_name##s_num++;\
}

ADD_SYMBOL_FN_DEF(mmp, MemoryMappedPeripheral);
ADD_SYMBOL_FN_DEF(struct_item, StructItem);
ADD_SYMBOL_FN_DEF(bitfield, BitField);
ADD_SYMBOL_FN_DEF(bitfield_item, BitFieldItem);
ADD_SYMBOL_FN_DEF(bitenum, BitEnum);
ADD_SYMBOL_FN_DEF(bitenum_item, BitEnumItem);

#endif