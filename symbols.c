#include "symbols.h"

int add_symbol(SymbolTable *symbols, Symbol symbol) {
    symbols->symbols[symbols->symbol_num] = symbol;
    int index = symbols->symbol_num;
    symbols->symbol_num += 1;
    return index;
}