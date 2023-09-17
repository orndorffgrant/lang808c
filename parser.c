#include "parser.h"
#include "common.h"
#include "lexer.h"


int match(TokenType token_type, Token *tokens, int next_token) {
    if (tokens[next_token].type != token_type) {
        PANIC(
            "Expected %s but found %s",
            token_type_to_static_string(token_type),
            token_type_to_static_string(tokens[next_token].type)
        );
    }
    PARSE_TREE_PRINT("%s", token_type_to_static_string(token_type));
    return next_token + 1;
}

int statement_mmp_def_structure_item_bf_item_enum_item(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("              BitEnumItem:\n");
    PARSE_TREE_PRINT("                ");
    // TODO actually process
    next_token = match(t_id, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("                ");
    next_token = match(t_equals, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("                ");
    // TODO actually process
    next_token = match(t_inthexliteral, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("                ");
    next_token = match(t_semicolon, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_structure_item_bf_item_enum(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("            BitEnumDef:\n");
    PARSE_TREE_PRINT("              ");
    // TODO actually process
    next_token = match(t_be, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("              ");
    next_token = match(t_leftbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");

    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of bf items
        next_token = statement_mmp_def_structure_item_bf_item_enum_item(tokens, next_token);
    }

    PARSE_TREE_PRINT("          ");
    next_token = match(t_rightbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_structure_item_bf_item(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("          BitFieldItem:\n");
    if (tokens[next_token].type == t_id) {
        PARSE_TREE_PRINT("            ");
        // TODO actually process
        next_token = match(t_id, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    } else {
        PARSE_TREE_PRINT("            ");
        // TODO actually process
        next_token = match(t_unused, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    }
    PARSE_TREE_PRINT("            ");
    next_token = match(t_colon, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    if (tokens[next_token].type == t_intdecliteral) {
        PARSE_TREE_PRINT("            ");
        // TODO actually process
        next_token = match(t_intdecliteral, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    } else {
        // TODO
        next_token = statement_mmp_def_structure_item_bf_item_enum(tokens, next_token);
    }
    PARSE_TREE_PRINT("            ");
    next_token = match(t_semicolon, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_structure_item_bf(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("        BitFieldDef:\n");
    PARSE_TREE_PRINT("          ");
    // TODO actually process
    next_token = match(t_bf, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("          ");
    next_token = match(t_leftbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");

    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of bf items
        next_token = statement_mmp_def_structure_item_bf_item(tokens, next_token);
    }

    PARSE_TREE_PRINT("          ");
    next_token = match(t_rightbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_structure_item(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("      StructureItem:\n");
    if (tokens[next_token].type == t_id) {
        PARSE_TREE_PRINT("        ");
        // TODO actually process
        next_token = match(t_id, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    } else {
        PARSE_TREE_PRINT("        ");
        // TODO actually process
        next_token = match(t_unused, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    }
    PARSE_TREE_PRINT("        ");
    next_token = match(t_colon, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    if (tokens[next_token].type == t_inttype) {
        PARSE_TREE_PRINT("        ");
        // TODO actually process
        next_token = match(t_inttype, tokens, next_token);
        PARSE_TREE_PRINT("\n");
    } else {
        next_token = statement_mmp_def_structure_item_bf(tokens, next_token);
    }
    PARSE_TREE_PRINT("        ");
    next_token = match(t_semicolon, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_structure(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("    Structure:\n");
    PARSE_TREE_PRINT("      ");
    next_token = match(t_leftbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of structure items
        next_token = statement_mmp_def_structure_item(tokens, next_token);
    }
    PARSE_TREE_PRINT("      ");
    next_token = match(t_rightbrace, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_opt_interrupt_num(Token *tokens, int next_token) {
    if (tokens[next_token].type != t_bang) {
        PARSE_TREE_PRINT("    NoInterruptNum\n");
        return next_token;
    }
    PARSE_TREE_PRINT("    InterruptNum:\n");
    PARSE_TREE_PRINT("      ");
    next_token = match(t_bang, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("      ");
    // TODO actually process
    next_token = match(t_intdecliteral, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def_base_address(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("    BaseAddress:\n");
    PARSE_TREE_PRINT("      ");
    next_token = match(t_at, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    PARSE_TREE_PRINT("      ");
    // TODO actually process
    next_token = match(t_inthexliteral, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    return next_token;
}
int statement_mmp_def(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("  MemoryMappedPeripheralDefinition:\n");
    PARSE_TREE_PRINT("    ");
    next_token = match(t_mmp, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    // TODO actually process
    PARSE_TREE_PRINT("    ");
    next_token = match(t_id, tokens, next_token);
    PARSE_TREE_PRINT("\n");
    next_token = statement_mmp_def_base_address(tokens, next_token);
    next_token = statement_mmp_def_opt_interrupt_num(tokens, next_token);
    next_token = statement_mmp_def_structure(tokens, next_token);
    return next_token;
}
int statement(Token *tokens, int next_token) {
    PARSE_TREE_PRINT("RootStatement:\n");
    switch (tokens[next_token].type) {
        case t_mmp:
            return statement_mmp_def(tokens, next_token);
        default:
            PANIC("invalid token at beginning of statement");
    }
}

int parse(Token *tokens, int token_num) {
    int next_token = 0;
    while (next_token < token_num) {
        next_token = statement(tokens, next_token);
    }
}