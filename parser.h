#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbols.h"

#ifndef PRINT_PARSE_TREE
#define PRINT_PARSE_TREE 1
#endif
#define PARSE_TREE_PRINT(...) if (PRINT_PARSE_TREE) printf(__VA_ARGS__)
#define PARSE_TREE_INDENT(n) for (int i = 0; i < (n); i++) { PARSE_TREE_PRINT("  "); }
#define PARSE_TREE_PRINT_TOKEN_TYPE(token_type) PARSE_TREE_PRINT("%s", token_type_to_static_string(token_type))

int parse(Token *tokens, int token_num, SymbolTable *symbols);

#endif