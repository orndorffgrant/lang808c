#include "parser.h"
#include "common.h"
#include "lexer.h"
#include "symbols.h"


int match(TokenType token_type, Token *tokens, int next_token, int indent) {
    if (tokens[next_token].type != token_type) {
        PANIC(
            "Expected %s but found %s\n",
            token_type_to_static_string(token_type),
            token_type_to_static_string(tokens[next_token].type)
        );
    }
    CREATE_TOKEN_STRING(tokens[next_token]);
    PARSE_TREE_INDENT(indent); PARSE_TREE_PRINT("- %s\n", token_str);
    return next_token + 1;
}
int match_inttype(Token *tokens, int next_token, IntType *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_inttype) {
        if (t.int_value == 8) {
            *dest = int_u8;
        } else if (t.int_value == 16) {
            *dest = int_u16;
        } else if (t.int_value == 32) {
            *dest = int_u32;
        } else {
            PANIC("INVALID INTTYPE");
        }
    }
    return match(t_inttype, tokens, next_token, indent);
}
int match_intliteral(Token *tokens, int next_token, int *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_intliteral) {
        *dest = t.int_value;
    }
    return match(t_intliteral, tokens, next_token, indent);
}
int match_id(Token *tokens, int next_token, StringRef *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_id) {
        *dest = t.lexeme;
    }
    return match(t_id, tokens, next_token, indent);
}

int name(Token *tokens, int next_token, int max, StringRef *dests, int *num_read, int indent) {
    int i = 1;
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Name:\n");
    next_token = match_id(tokens, next_token, &dests[0], indent);
    while (tokens[next_token].type == t_dot && i < max) {
        next_token = match(t_dot, tokens, next_token, indent);
        next_token = match_id(tokens, next_token, &dests[i], indent);
        i++;
    }
    *num_read = i;
    return next_token;
}

int expression(Token *tokens, int next_token, SymbolTable *symbols, int indent);

int function_call(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- FunctionCall:\n");
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightparen) {
        next_token = expression(tokens, next_token, symbols, indent);
        if (tokens[next_token].type != t_rightparen) {
            next_token = match(t_comma, tokens, next_token, indent);
        }
    }
    next_token = match(t_rightparen, tokens, next_token, indent);
    return next_token;
}
int expression_term(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ExpressionTerm:\n");
    if (tokens[next_token].type == t_intliteral) {
        next_token = match(t_intliteral, tokens, next_token, indent);
    } else {
        StringRef name_ids[2];
        int num_names;
        int mmp_index = -1;
        int si_index = -1;
        next_token = name(tokens, next_token, 2, name_ids, &num_names, indent);
        if (num_names == 2) {
            // Must be field on an MMP
            StringRef mmp_name = name_ids[0];
            StringRef struct_item_name = name_ids[1];
            mmp_index = find_mmp_index(symbols, &mmp_name);
            if (mmp_index == -1) {
                STRINGREF_TO_CSTR1(&mmp_name, 512);
                PANIC("Cannot initialize undefined MemoryMappedPeripheral: %s\n", cstr1);
            }
            si_index = find_struct_item_index(symbols, mmp_index, &struct_item_name);
            if (si_index == -1) {
                STRINGREF_TO_CSTR1(&struct_item_name, 512);
                STRINGREF_TO_CSTR2(&symbols->mmps[mmp_index].name, 512);
                PANIC("Field '%s' does not exist in '%s'\n", cstr1, cstr2);
            }
        }
    }
    return next_token;
}
int expression_shift(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ShiftExpression:\n");
    next_token = expression_term(tokens, next_token, symbols, indent);
    switch (tokens[next_token].type) {
        case t_shiftleft:
            next_token = match(t_shiftleft, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_term(tokens, next_token, symbols, indent);
    return next_token;
}
int expression_bit(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitExpression:\n");
    next_token = expression_shift(tokens, next_token, symbols, indent);
    switch (tokens[next_token].type) {
        case t_and:
            next_token = match(t_and, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_shift(tokens, next_token, symbols, indent);
    return next_token;
}
int expression_sum(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- SumExpression:\n");
    next_token = expression_bit(tokens, next_token, symbols, indent);
    switch (tokens[next_token].type) {
        case t_plus:
            next_token = match(t_plus, tokens, next_token, indent);
            break;
        case t_minus:
            next_token = match(t_minus, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_bit(tokens, next_token, symbols, indent);
    return next_token;
}
int expression(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    // top level expression is comparison
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Expression:\n");
    next_token = expression_sum(tokens, next_token, symbols, indent);
    switch (tokens[next_token].type) {
        case t_equalsequals:
            next_token = match(t_equalsequals, tokens, next_token, indent);
            break;
        case t_lessthan:
            next_token = match(t_lessthan, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_sum(tokens, next_token, symbols, indent);
    return next_token;
}

int bitfield_value(Token *tokens, int next_token, SymbolTable *symbols, int si_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldValue:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldValueItem:\n");
        StringRef bf_item_name;
        next_token = match_id(tokens, next_token, &bf_item_name, indent);
        int bfi_index = find_bitfield_item_index(symbols, si_index, &bf_item_name);
        if (bfi_index == -1) {
            STRINGREF_TO_CSTR1(&bf_item_name, 512);
            STRINGREF_TO_CSTR2(&symbols->struct_items[si_index].name, 512);
            PANIC("BitField field '%s' does not exist in struct item '%s'\n", cstr1, cstr2);
        }
        next_token = match(t_equals, tokens, next_token, indent);
        if (symbols->bitfield_items[bfi_index].type == bfi_int) {
            // TODO actually process
            next_token = match(t_intliteral, tokens, next_token, indent);
        } else {
            StringRef be_item_name;
            next_token = match_id(tokens, next_token, &be_item_name, indent);
            int be_index = find_bitenum_item_index(symbols, bfi_index, &be_item_name);
            if (be_index == -1) {
                STRINGREF_TO_CSTR1(&bf_item_name, 512);
                STRINGREF_TO_CSTR2(&be_item_name, 512);
                PANIC("BitEnum for field '%s' does not include value called '%s'\n", cstr1, cstr2);
            }
        }
        next_token = match(t_semicolon, tokens, next_token, indent);
    }

    indent--;
    next_token = match(t_rightbrace, tokens, next_token, indent);

    return next_token;
}

int mmp_def_structure_item_bf_item_enum_item(Token *tokens, int next_token, BitEnumItem *bei, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnumItem:\n");
    next_token = match_id(tokens, next_token, &bei->name, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, &bei->value, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf_item_enum(Token *tokens, int next_token, SymbolTable *symbols, BitEnum *be, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnum:\n");
    next_token = match(t_be, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    be->be_items_index = -1;
    int bei_index = 0;
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        BitEnumItem bei;
        // any number of be items
        next_token = mmp_def_structure_item_bf_item_enum_item(tokens, next_token, &bei, indent);

        bei_index = add_bitenum_item(symbols, bei);
        if (be->be_items_index == -1) {
            // first one
            be->be_items_index = bei_index;
        }
    }
    be->be_items_len = (bei_index + 1) - be->be_items_index;

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf_item(Token *tokens, int next_token, SymbolTable *symbols, BitFieldItem *bfi, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldItem:\n");
    bfi->type = bfi_int;
    if (tokens[next_token].type == t_id) {
        next_token = match_id(tokens, next_token, &bfi->name, indent);
    } else {
        next_token = match(t_unused, tokens, next_token, indent);
        bfi->type = bfi_unused;
    }
    next_token = match(t_colon, tokens, next_token, indent);
    if (tokens[next_token].type == t_intliteral) {
        next_token = match_intliteral(tokens, next_token, &bfi->width, indent);
    } else {
        next_token = mmp_def_structure_item_bf_item_enum(tokens, next_token, symbols, &bfi->be, indent);
        bfi->type = bfi_enum;
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf(Token *tokens, int next_token, SymbolTable *symbols, StructItem *si, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitField:\n");
    Token t = tokens[next_token];
    if (t.type == t_bf) {
        si->bf.width = t.int_value;
    }
    next_token = match(t_bf, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    si->bf.bf_items_index = -1;
    int bfi_index = 0;
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        BitFieldItem bfi;
        // any number of bf items
        next_token = mmp_def_structure_item_bf_item(tokens, next_token, symbols, &bfi, indent);

        bfi_index = add_bitfield_item(symbols, bfi);
        if (si->bf.bf_items_index == -1) {
            // first one
            si->bf.bf_items_index = bfi_index;
        }
    }
    si->bf.bf_items_len = (bfi_index + 1) - si->bf.bf_items_index;

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item(Token *tokens, int next_token, SymbolTable *symbols, StructItem *si, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- StructureItem:\n");
    if (tokens[next_token].type == t_id) {
        next_token = match_id(tokens, next_token, &si->name, indent);
    } else {
        next_token = match(t_unused, tokens, next_token, indent);
        si->type = si_unused;
    }
    next_token = match(t_colon, tokens, next_token, indent);
    if (tokens[next_token].type == t_inttype) {
        next_token = match_inttype(tokens, next_token, &si->int_type, indent);
        if (si->type != si_unused) {
            si->type = si_int;
        }
    } else {
        next_token = mmp_def_structure_item_bf(tokens, next_token, symbols, si, indent);
        si->type = si_bf;
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure(Token *tokens, int next_token, SymbolTable *symbols, MemoryMappedPeripheral *mmp, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Structure:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);
    mmp->struct_items_index = -1;
    int si_index = 0;
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        StructItem si;
        next_token = mmp_def_structure_item(tokens, next_token, symbols, &si, indent);
        si_index = add_struct_item(symbols, si);
        if (mmp->struct_items_index == -1) {
            // first one
            mmp->struct_items_index = si_index;
        }
    }
    mmp->struct_items_len = (si_index + 1) - mmp->struct_items_index;
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_opt_interrupt_num(Token *tokens, int next_token, int *dest, int indent) {
    if (tokens[next_token].type != t_bang) {
        PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- NoInterruptNum:\n");
        *dest = -1;
        return next_token;
    }
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- InterruptNum:\n");
    next_token = match(t_bang, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, dest, indent);
    return next_token;
}
int mmp_def_base_address(Token *tokens, int next_token, int *dest, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BaseAddress:\n");
    next_token = match(t_at, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, dest, indent);
    return next_token;
}
int mmp_def(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- MemoryMappedPeripheral:\n");
    MemoryMappedPeripheral mmp;

    next_token = match(t_mmp, tokens, next_token, indent);
    next_token = match_id(tokens, next_token, &mmp.name, indent);
    next_token = mmp_def_base_address(tokens, next_token, &mmp.base_address, indent);
    next_token = mmp_def_opt_interrupt_num(tokens, next_token, &mmp.interrupt_number, indent);
    next_token = mmp_def_structure(tokens, next_token, symbols, &mmp, indent);

    add_mmp(symbols, mmp);
    return next_token;
}
int initialize_statement(Token *tokens, int next_token, SymbolTable *symbols, int mmp_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- InitializeStatement:\n");
    StringRef struct_item_name;
    next_token = match_id(tokens, next_token, &struct_item_name, indent);
    int si_index = find_struct_item_index(symbols, mmp_index, &struct_item_name);
    if (si_index == -1) {
        STRINGREF_TO_CSTR1(&struct_item_name, 512);
        STRINGREF_TO_CSTR2(&symbols->mmps[mmp_index].name, 512);
        PANIC("Field '%s' does not exist in '%s'\n", cstr1, cstr2);
    }

    next_token = match(t_equals, tokens, next_token, indent);

    if (symbols->struct_items[si_index].type == si_bf) {
        // TODO create IR
        next_token = bitfield_value(tokens, next_token, symbols, si_index, indent);
    } else {
        // TODO create IR
        int value = 0;
        next_token = match_intliteral(tokens, next_token, &value, indent);
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int initialize(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Initialize:\n");
    next_token = match(t_initialize, tokens, next_token, indent);

    StringRef mmp_name;
    next_token = match_id(tokens, next_token, &mmp_name, indent);
    int mmp_index = find_mmp_index(symbols, &mmp_name);
    if (mmp_index == -1) {
        STRINGREF_TO_CSTR1(&mmp_name, 512);
        PANIC("Cannot initialize undefined MemoryMappedPeripheral: %s\n", cstr1);
    }

    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of intialization statements
        next_token = initialize_statement(tokens, next_token, symbols, mmp_index, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}

int function_statement(Token *tokens, int next_token, SymbolTable *symbols, int indent);

int function_statement_return(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Return:\n");
    next_token = match(t_return, tokens, next_token, indent);
    next_token = expression(tokens, next_token, symbols, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int function_statement_assignment(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Assignment:\n");
    // TODO support static vars and local vars
    StringRef name_ids[2];
    int num_names;
    int mmp_index = -1;
    int si_index = -1;
    next_token = name(tokens, next_token, 2, name_ids, &num_names, indent);
    if (num_names == 2) {
        // Must be field on an MMP
        StringRef mmp_name = name_ids[0];
        StringRef struct_item_name = name_ids[1];
        mmp_index = find_mmp_index(symbols, &mmp_name);
        if (mmp_index == -1) {
            STRINGREF_TO_CSTR1(&mmp_name, 512);
            PANIC("Cannot initialize undefined MemoryMappedPeripheral: %s\n", cstr1);
        }
        si_index = find_struct_item_index(symbols, mmp_index, &struct_item_name);
        if (si_index == -1) {
            STRINGREF_TO_CSTR1(&struct_item_name, 512);
            STRINGREF_TO_CSTR2(&symbols->mmps[mmp_index].name, 512);
            PANIC("Field '%s' does not exist in '%s'\n", cstr1, cstr2);
        }
    }

    next_token = match(t_equals, tokens, next_token, indent);
    if (tokens[next_token].type == t_id && tokens[next_token + 1].type == t_leftparen) {
        next_token = function_call(tokens, next_token, symbols, indent);
    } else if (tokens[next_token].type == t_leftbrace) {
        // TODO
        next_token = bitfield_value(tokens, next_token, symbols, si_index, indent);
    } else {
        next_token = expression(tokens, next_token, symbols, indent);
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int function_statement_if(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- If:\n");
    next_token = match(t_if, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);
    next_token = expression(tokens, next_token, symbols, indent);
    next_token = match(t_rightparen, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);

    if (tokens[next_token].type == t_else) {
        next_token = match(t_else, tokens, next_token, indent);
        next_token = match(t_leftbrace, tokens, next_token, indent);

        while (tokens[next_token].type != t_rightbrace) {
            // any number of function statements
            next_token = function_statement(tokens, next_token, symbols, indent);
        }

        next_token = match(t_rightbrace, tokens, next_token, indent);
    }

    return next_token;
}
int function_statement_local_var(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- LocalVariable:\n");
    next_token = match(t_inttype, tokens, next_token, indent);
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match(t_intliteral, tokens, next_token, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int function_statement_for_loop(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ForLoop:\n");
    next_token = match(t_for, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, symbols, indent);

    next_token = expression(tokens, next_token, symbols, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, symbols, indent);

    next_token = match(t_rightparen, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int function_statement(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    switch (tokens[next_token].type) {
        case t_for:
            return function_statement_for_loop(tokens, next_token, symbols, indent);
        case t_inttype:
            return function_statement_local_var(tokens, next_token, indent);
        case t_if:
            return function_statement_if(tokens, next_token, symbols, indent);
        case t_id:
            if (tokens[next_token+1].type == t_leftparen) {
                next_token = function_call(tokens, next_token, symbols, indent);
                next_token = match(t_semicolon, tokens, next_token, indent);
                return next_token;
            } else {
                return function_statement_assignment(tokens, next_token, symbols, indent);
            }
        case t_return:
            return function_statement_return(tokens, next_token, symbols, indent);
        default:
            PANIC("invalid token at beginning of function statement\n");
    }
}
int function_argument(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- FunctionArgument:\n");
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_colon, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_inttype, tokens, next_token, indent);
    return next_token;
}
int function(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Function:\n");
    next_token = match(t_fun, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightparen) {
        // any number of args
        next_token = function_argument(tokens, next_token, indent);
        if (tokens[next_token].type == t_rightparen) {
            break;
        }
        next_token = match(t_comma, tokens, next_token, indent);
    }
    next_token = match(t_rightparen, tokens, next_token, indent);

    // optional return type
    if (tokens[next_token].type == t_colon) {
        next_token = match(t_colon, tokens, next_token, indent);
        next_token = match(t_inttype, tokens, next_token, indent);
    }

    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}

int static_var(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- StaticVariable:\n");
    next_token = match(t_static, tokens, next_token, indent);
    next_token = match(t_inttype, tokens, next_token, indent);
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match(t_intliteral, tokens, next_token, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}

int on_interrupt(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- OnInterrupt:\n");
    next_token = match(t_on_interrupt, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}

int root_statement(Token *tokens, int next_token, SymbolTable *symbols) {
    switch (tokens[next_token].type) {
        case t_mmp:
            return mmp_def(tokens, next_token, symbols, 0);
        case t_initialize:
            return initialize(tokens, next_token, symbols, 0);
        case t_fun:
            return function(tokens, next_token, symbols, 0);
        case t_static:
            return static_var(tokens, next_token, 0);
        case t_on_interrupt:
            return on_interrupt(tokens, next_token, symbols, 0);
        default:
            PANIC("invalid token at beginning of root statement\n");
    }
}

int parse(Token *tokens, int token_num, SymbolTable *symbols) {
    int next_token = 0;
    while (next_token < token_num) {
        next_token = root_statement(tokens, next_token, symbols);
    }
}