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
    BitFieldItem bitfield_items[1024];
    int bitfield_items_num;
    BitEnumItem bitenum_items[1024];
    int bitenum_items_num;
} SymbolTable;

int add_mmp(SymbolTable *symbols, MemoryMappedPeripheral item);
int add_struct_item(SymbolTable *symbols, StructItem item);
int add_bitfield_item(SymbolTable *symbols, BitFieldItem item);
int add_bitenum_item(SymbolTable *symbols, BitEnumItem item);

int find_mmp_index(SymbolTable *symbols, StringRef *name);
int find_struct_item_index(SymbolTable *symbols, int mmp_index, StringRef *name);
int find_bitfield_item_index(SymbolTable *symbols, int si_index, StringRef *name);
int find_bitenum_item_index(SymbolTable *symbols, int bfi_index, StringRef *name);

#endif