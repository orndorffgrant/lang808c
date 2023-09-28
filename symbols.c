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