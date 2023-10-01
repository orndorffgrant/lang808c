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