#include <stdbool.h>

#include "symbols.h"
#include "common.h"

ADD_SYMBOL_FN(mmp, MemoryMappedPeripheral)
ADD_SYMBOL_FN(struct_item, StructItem)
ADD_SYMBOL_FN(bitfield, BitField)
ADD_SYMBOL_FN(bitfield_item, BitFieldItem)
ADD_SYMBOL_FN(bitenum, BitEnum)
ADD_SYMBOL_FN(bitenum_item, BitEnumItem)

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