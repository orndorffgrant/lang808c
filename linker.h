
#ifndef LINKER_H
#define LINKER_H

#include <stdint.h>

#include "armv6m.h"
#include "symbols.h"

int link(SymbolTable *symbols, MachineCode *code, uint8_t *dest);
void print_hex(uint8_t *code, int len);

#endif