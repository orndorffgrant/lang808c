#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "common.h"

#define MAX_SYMBOLS 32768

typedef struct _Symbol {
    StringRef lexeme;
    long intliteral_value;
} Symbol;

typedef struct _SymbolTable {
    Symbol symbols[MAX_SYMBOLS];
    int symbol_num;
} SymbolTable;

int add_symbol(SymbolTable *symbols, Symbol symbol);

#endif