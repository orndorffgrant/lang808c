// This file contains the implementation of the Lang808 parser
// The entrypoint of the parser is the "parse" function defined at the bottom of this file.
// The parser is a predictive recursive descent parser, which uses the call stack as it's implicit parse tree.
// The tree is printed as the parser traverses the tree, in yaml format.
// Each function "consumes" part of the Token list by accepting a "next_token" argument, incrementing it
// as it parses tokens, and then returning the index to the next token that it hasn't yet parsed.
// Most functions take a reference the SymbolTable and add or reference entries as appropriate.

#include "parser.h"
#include "common.h"
#include "ir.h"
#include "lexer.h"
#include "symbols.h"


// This asserts that the next token is what we expect and then increments next_token
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

// parse an inttype token, putting the IntType enum into dest
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
// parse an intliteral token, putting the literal value into dest
int match_intliteral(Token *tokens, int next_token, int *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_intliteral) {
        *dest = t.int_value;
    }
    return match(t_intliteral, tokens, next_token, indent);
}
// parse an id token, putting the string value into dest
int match_id(Token *tokens, int next_token, StringRef *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_id) {
        *dest = t.lexeme;
    }
    return match(t_id, tokens, next_token, indent);
}
// parse a BitEnum token, putting the width value into dest
int match_bitenum(Token *tokens, int next_token, int *dest, int indent) {
    Token t = tokens[next_token];
    if (t.type == t_be) {
        *dest = t.int_value;
    }
    return match(t_be, tokens, next_token, indent);
}

enum name_result {
    name_mmp_struct_item,
    name_func_arg,
    name_local_var,
    name_static_var
};
struct NameResolutionResult {
    enum name_result result;
    int mmp_index;
    int si_index;
    int func_arg_index;
    int local_var_index;
    int static_var_index;
};
// parse a "Name" which can be "id" if its a variable, or "id.id" if its a field on a peripheral
// Check that the variable/peripheral reference is valid and detect which it is
int name(Token *tokens, int next_token, SymbolTable *symbols, int func_index, struct NameResolutionResult *result, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Name:\n");
    StringRef first_name;
    StringRef second_name;
    next_token = match_id(tokens, next_token, &first_name, indent);
    if (tokens[next_token].type == t_dot) {
        // Must be field on an MMP
        next_token = match(t_dot, tokens, next_token, indent);
        next_token = match_id(tokens, next_token, &second_name, indent);
        result->mmp_index = find_mmp_index(symbols, &first_name);
        if (result->mmp_index == -1) {
            STRINGREF_TO_CSTR1(&first_name, 512);
            PANIC("Reference to undefined MemoryMappedPeripheral: '%s'\n", cstr1);
        }
        result->si_index = find_struct_item_index(symbols, result->mmp_index, &second_name);
        if (result->si_index == -1) {
            STRINGREF_TO_CSTR1(&second_name, 512);
            STRINGREF_TO_CSTR2(&first_name, 512);
            PANIC("Field '%s' does not exist in '%s'\n", cstr1, cstr2);
        }
        result->result = name_mmp_struct_item;
        return next_token;
    } else {
        // must be a variable, local or static
        result->func_arg_index = find_function_arg(symbols, func_index, &first_name);
        if (result->func_arg_index > -1) {
            result->result = name_func_arg;
            return next_token;
        } 
        result->local_var_index = find_function_variable(symbols, func_index, &first_name);
        if (result->local_var_index > -1) {
            result->result = name_local_var;
            return next_token;
        } 
        result->static_var_index = find_static_variable(symbols, &first_name);
        if (result->static_var_index == -1) {
            STRINGREF_TO_CSTR1(&first_name, 512);
            PANIC("No variable, arg or local or static, named '%s'\n", cstr1);
        }
        result->result = name_static_var;
        return next_token;
    }
}

int expression(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent);

// parse a function call e.g. "function_name(arg1, arg2)"
// checks that the function exists and that the correct number of arguments are passed
int function_call(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- FunctionCall:\n");
    StringRef func_name;
    next_token = match_id(tokens, next_token, &func_name, indent);
    int func_index = find_function_index(symbols, &func_name);
    if (func_index == -1) {
        STRINGREF_TO_CSTR1(&func_name, 512);
        PANIC("Function '%s' does not exist\n", cstr1);
    }
    int num_args = 0;
    next_token = match(t_leftparen, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightparen) {
        next_token = expression(tokens, next_token, symbols, func_index, indent);
        num_args++;
        if (tokens[next_token].type != t_rightparen) {
            next_token = match(t_comma, tokens, next_token, indent);
        }
    }
    int expected_args = symbols->functions[func_index].func_args_len;
    if (num_args != expected_args) {
        STRINGREF_TO_CSTR1(&func_name, 512);
        PANIC("Incorrect number of arguments to function '%s'. Expected %d but got %d.\n", cstr1, expected_args, num_args);
    }
    next_token = match(t_rightparen, tokens, next_token, indent);
    return next_token;
}
// terminal in an expression, either an int literal or a "name"
int expression_term(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ExpressionTerm:\n");
    if (tokens[next_token].type == t_intliteral) {
        next_token = match(t_intliteral, tokens, next_token, indent);
    } else {
        struct NameResolutionResult name_result;
        next_token = name(tokens, next_token, symbols, func_index, &name_result, indent);
    }
    return next_token;
}
// parse a shift expression e.g. "1 << 30"
int expression_shift(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ShiftExpression:\n");
    next_token = expression_term(tokens, next_token, symbols, func_index, indent);
    switch (tokens[next_token].type) {
        case t_shiftleft:
            next_token = match(t_shiftleft, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_term(tokens, next_token, symbols, func_index, indent);
    return next_token;
}
// parse a bitwise operation expression e.g. "1 & 30"
int expression_bit(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitExpression:\n");
    next_token = expression_shift(tokens, next_token, symbols, func_index, indent);
    switch (tokens[next_token].type) {
        case t_and:
            next_token = match(t_and, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_shift(tokens, next_token, symbols, func_index, indent);
    return next_token;
}
// parse an addition or subtraction operation expression e.g. "1 - 30"
int expression_sum(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- SumExpression:\n");
    next_token = expression_bit(tokens, next_token, symbols, func_index, indent);
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
    next_token = expression_bit(tokens, next_token, symbols, func_index, indent);
    return next_token;
}
// parse a comparison expression e.g. "1 > 30"
int expression(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    // top level expression is comparison
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Expression:\n");
    next_token = expression_sum(tokens, next_token, symbols, func_index, indent);
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
    next_token = expression_sum(tokens, next_token, symbols, func_index, indent);
    return next_token;
}

// parse a value being assigned to an item that is a BitField
// e.g. { val = 4; val2 = 6; val3 = enum_name }
// check that the fields in the bitfield exist
// check that enum names used for a particular field exist
int bitfield_value(Token *tokens, int next_token, SymbolTable *symbols, int si_index, int *dest, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldValue:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);
    int full_value = 0;

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
        BitFieldItem *bfi = &symbols->bitfield_items[bfi_index];
        int field_value = 0;
        next_token = match(t_equals, tokens, next_token, indent);
        if (symbols->bitfield_items[bfi_index].type == bfi_int) {
            next_token = match_intliteral(tokens, next_token, &field_value, indent);
        } else {
            // TODO
            StringRef be_item_name;
            next_token = match_id(tokens, next_token, &be_item_name, indent);
            int bei_index = find_bitenum_item_index(symbols, bfi_index, &be_item_name);
            if (bei_index == -1) {
                STRINGREF_TO_CSTR1(&bf_item_name, 512);
                STRINGREF_TO_CSTR2(&be_item_name, 512);
                PANIC("BitEnum for field '%s' does not include value called '%s'\n", cstr1, cstr2);
            }
            BitEnumItem *bei = &symbols->bitenum_items[bei_index];
            field_value = bei->value;
        }
        next_token = match(t_semicolon, tokens, next_token, indent);
        full_value |= (field_value << bfi->offset);
        indent--;
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);

    *dest = full_value;

    return next_token;
}

// parse an item definition of a BitEnum e.g. "clock4 = 0x4;"
// put name and value of enum item into *bei
int mmp_def_structure_item_bf_item_enum_item(Token *tokens, int next_token, BitEnumItem *bei, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnumItem:\n");
    next_token = match_id(tokens, next_token, &bei->name, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, &bei->value, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
// parse a BitEnum definition e.g. "BitEnum5 { clock4 = 0x4; }"
// add bit enum items to symbol table and link *be to those items
int mmp_def_structure_item_bf_item_enum(Token *tokens, int next_token, SymbolTable *symbols, BitEnum *be, int *width, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnum:\n");
    next_token = match_bitenum(tokens, next_token, width, indent);
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
// parse an item definition within a BitField e.g. "clock_id: 32;"
// put name and type into *bf
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
        next_token = mmp_def_structure_item_bf_item_enum(tokens, next_token, symbols, &bfi->be, &bfi->width, indent);
        bfi->type = bfi_enum;
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
// parse a BitField e.g. "BitField32 { clock_id: 32; }"
// add BitField items to symbol table and link *si to those items
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
    int offset = 0;
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        BitFieldItem bfi;
        // any number of bf items
        next_token = mmp_def_structure_item_bf_item(tokens, next_token, symbols, &bfi, indent);

        bfi.offset = offset;
        offset += bfi.width;
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
// parse a StructItem of a MemoryMappedPeripheral e.g. "field: u32;"
// put name and type into *si
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
// parse a Structure for a peripheral e.g. "{ field: u32; field2:u16 }"
// add StructItems to symbol table and link *mmp to those items
int mmp_def_structure(Token *tokens, int next_token, SymbolTable *symbols, MemoryMappedPeripheral *mmp, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Structure:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);
    mmp->struct_items_index = -1;
    int si_index = 0;
    int address = mmp->base_address;
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        StructItem si;
        next_token = mmp_def_structure_item(tokens, next_token, symbols, &si, indent);
        si.address = address;
        if (si.type == si_int || si.type == si_unused) {
            if (si.int_type == int_u8) {
                address += 1;
            } else if (si.int_type == int_u16) {
                address += 2;
            } else if (si.int_type == int_u32) {
                address += 4;
            }
        } else { // si.type == si_bf
            address += (si.bf.width / 8);
        }
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
// parse optional interrupt number for a peripheral, e.g. "!42"
// put interrupt num, if present, in dest
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
// parse base address for a peripheral, e.g. "@0x40000000"
// put base address in dest
int mmp_def_base_address(Token *tokens, int next_token, int *dest, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BaseAddress:\n");
    next_token = match(t_at, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, dest, indent);
    return next_token;
}
// parse MemoryMappedPeripheral, e.g. "MemoryMappedPeripheral PeripheralName @0x40000000 !42 {}"
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

// parse a statement inside an initialize block
// check that the fields and values are valid for the peripheral being initialized
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

    int value = 0;
    if (symbols->struct_items[si_index].type == si_bf) {
        next_token = bitfield_value(tokens, next_token, symbols, si_index, &value, indent);
    } else {
        next_token = match_intliteral(tokens, next_token, &value, indent);
    }

    IROp op = {0};
    op.opcode = ir_copy;
    op.result.type = irv_mmp_struct_item;
    op.result.mmp_index = mmp_index;
    op.result.mmp_struct_item_index = si_index;
    op.arg1.type = irv_immediate;
    op.arg1.immediate_value = value;
    add_function_ir(symbols, INIT_FUNC_INDEX, op);

    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
// parse an initialize block
// check that the peripheral name is valid
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

int function_statement(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent);

// parse return statement, e.g. "return 5 + var;"
int function_statement_return(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Return:\n");
    next_token = match(t_return, tokens, next_token, indent);
    next_token = expression(tokens, next_token, symbols, func_index, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
// parse assignment statement, e.g. "var = 5 + var;"
// check that the variable being assigned to is valid
int function_statement_assignment(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Assignment:\n");
    struct NameResolutionResult name_result;
    next_token = name(tokens, next_token, symbols, func_index, &name_result, indent);

    next_token = match(t_equals, tokens, next_token, indent);
    if (name_result.result == name_mmp_struct_item) {
        if (symbols->struct_items[name_result.si_index].type == si_bf) {
            int value = 0;
            next_token = bitfield_value(tokens, next_token, symbols, name_result.si_index, &value, indent);
        } else {
            next_token = expression(tokens, next_token, symbols, func_index, indent);
        }
    } else {
        // local or static variable
        if (tokens[next_token].type == t_id && tokens[next_token + 1].type == t_leftparen) {
            next_token = function_call(tokens, next_token, symbols, indent);
        } else {
            next_token = expression(tokens, next_token, symbols, func_index, indent);
        }
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
// parse an if-else statement, e.g. "if (1) { x = 2 } else { x = 3 }"
// "else" is optional
int function_statement_if(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- If:\n");
    next_token = match(t_if, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);
    next_token = expression(tokens, next_token, symbols, func_index, indent);
    next_token = match(t_rightparen, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, func_index, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);

    if (tokens[next_token].type == t_else) {
        next_token = match(t_else, tokens, next_token, indent);
        next_token = match(t_leftbrace, tokens, next_token, indent);

        while (tokens[next_token].type != t_rightbrace) {
            // any number of function statements
            next_token = function_statement(tokens, next_token, symbols, func_index, indent);
        }

        next_token = match(t_rightbrace, tokens, next_token, indent);
    }

    return next_token;
}
// parse local variable declaration, e.g. "u32 var = 4;"
// initial value is required
// puts the variable into the symbol table
int function_statement_local_var(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- LocalVariable:\n");
    Variable var;

    next_token = match_inttype(tokens, next_token, &var.int_type, indent);
    next_token = match_id(tokens, next_token, &var.name, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, &var.initial_value, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);

    int var_index = add_function_variable(symbols, var);
    if (symbols->functions[func_index].func_vars_index == -1) {
        symbols->functions[func_index].func_vars_index = var_index;
    }
    symbols->functions[func_index].func_vars_len++;

    IROp op = {0};
    op.opcode = ir_copy;
    op.result.type = irv_local_variable;
    op.result.local_variable_index = var_index;
    op.arg1.type = irv_immediate;
    op.arg1.immediate_value = var.initial_value;
    add_function_ir(symbols, func_index, op);
    return next_token;
}
// parse for-loop, e.g. "for (u32 i = 1; i < 10; i = i + 1) {}"
int function_statement_for_loop(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ForLoop:\n");
    next_token = match(t_for, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, symbols, func_index, indent);

    next_token = expression(tokens, next_token, symbols, func_index, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, symbols, func_index, indent);

    next_token = match(t_rightparen, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, func_index, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
// parse any function statement, lookahead at next token to determine which kind of statement it will be
int function_statement(Token *tokens, int next_token, SymbolTable *symbols, int func_index, int indent) {
    switch (tokens[next_token].type) {
        case t_for:
            return function_statement_for_loop(tokens, next_token, symbols, func_index, indent);
        case t_inttype:
            return function_statement_local_var(tokens, next_token, symbols, func_index, indent);
        case t_if:
            return function_statement_if(tokens, next_token, symbols, func_index, indent);
        case t_id:
            if (tokens[next_token+1].type == t_leftparen) {
                next_token = function_call(tokens, next_token, symbols, indent);
                next_token = match(t_semicolon, tokens, next_token, indent);
                return next_token;
            } else {
                return function_statement_assignment(tokens, next_token, symbols, func_index, indent);
            }
        case t_return:
            return function_statement_return(tokens, next_token, symbols, func_index, indent);
        default:
            PANIC("invalid token at beginning of function statement\n");
    }
}
// parse function argument definition
// put the name and type into *fa
int function_argument(Token *tokens, int next_token, FunctionArg *fa, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- FunctionArgument:\n");
    next_token = match_id(tokens, next_token, &fa->name, indent);
    next_token = match(t_colon, tokens, next_token, indent);
    next_token = match_inttype(tokens, next_token, &fa->int_type, indent);
    return next_token;
}
// parse a whole function
// put the function name, argument references, and variable references into the symbol table
int function(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Function:\n");
    Function func;
    func.func_vars_index = -1;
    func.func_vars_len = 0;
    func.ir_code_index = -1;
    func.ir_code_len = 0;

    next_token = match(t_fun, tokens, next_token, indent);
    next_token = match_id(tokens, next_token, &func.name, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);

    int func_index = add_function(symbols, func);
    Function *func_ref = &symbols->functions[func_index];

    func_ref->func_args_index = -1;
    func_ref->func_args_len = 0;
    int fa_index = 0;
    while (tokens[next_token].type != t_rightparen) {
        // any number of args
        FunctionArg fa;
        next_token = function_argument(tokens, next_token, &fa, indent);
        fa_index = add_function_arg(symbols, fa);
        if (func_ref->func_args_index == -1) {
            // first one
            func_ref->func_args_index = fa_index;
        }
        func_ref->func_args_len++;

        if (tokens[next_token].type == t_rightparen) {
            break;
        }
        next_token = match(t_comma, tokens, next_token, indent);
    }

    next_token = match(t_rightparen, tokens, next_token, indent);

    // optional return type
    if (tokens[next_token].type == t_colon) {
        func_ref->returns = true;
        next_token = match(t_colon, tokens, next_token, indent);
        next_token = match_inttype(tokens, next_token, &func.return_type, indent);
    } else {
        func_ref->returns = false;
    }

    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, func_index, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);

    return next_token;
}

// parse a static (global) variable, e.g "static u32 global = 45;"
// put the variable in the symbol table
int static_var(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- StaticVariable:\n");
    Variable var;

    next_token = match(t_static, tokens, next_token, indent);
    next_token = match_inttype(tokens, next_token, &var.int_type, indent);
    next_token = match_id(tokens, next_token, &var.name, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = match_intliteral(tokens, next_token, &var.initial_value, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);

    int index = add_static_variable(symbols, var);

    IROp op = {0};
    op.opcode = ir_copy;
    op.result.type = irv_static_variable;
    op.result.static_variable_index = index;
    op.arg1.type = irv_immediate;
    op.arg1.immediate_value = var.initial_value;
    add_function_ir(symbols, INIT_FUNC_INDEX, op);
    return next_token;
}

// parse an on_interrupt block, e.g. "on_interrupt PeripheralName {}"
// check that the peripheral exists and has an interrupt number defined
// create a function with no arguments and parse the statements into that function
int on_interrupt(Token *tokens, int next_token, SymbolTable *symbols, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- OnInterrupt:\n");
    next_token = match(t_on_interrupt, tokens, next_token, indent);

    StringRef mmp_name;
    next_token = match_id(tokens, next_token, &mmp_name, indent);
    int mmp_index = find_mmp_index(symbols, &mmp_name);
    if (mmp_index == -1) {
        STRINGREF_TO_CSTR1(&mmp_name, 512);
        PANIC("Cannot set interrupt handler for undefined MemoryMappedPeripheral: %s\n", cstr1);
    }
    if (symbols->mmps[mmp_index].interrupt_number == -1) {
        STRINGREF_TO_CSTR1(&mmp_name, 512);
        PANIC("Cannot set interrupt handler for '%s' - no interrupt number defined\n", cstr1);
    }

    InterruptHandler handler;
    handler.interrupt_number = symbols->mmps[mmp_index].interrupt_number;

    Function func;
    func.func_args_index = -1;
    func.func_args_len = 0;
    func.ir_code_index = -1;
    func.ir_code_len = 0;
    func.func_vars_index = -1;
    func.func_vars_len = 0;
    func.returns = false;
    func.name.str = "____interrupt_handler";
    func.name.len = 21;

    int func_index = add_function(symbols, func);

    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, symbols, func_index, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}

// parse any top-level statement, lookahead at next token to determine which kind of statement it will be
int root_statement(Token *tokens, int next_token, SymbolTable *symbols) {
    switch (tokens[next_token].type) {
        case t_mmp:
            return mmp_def(tokens, next_token, symbols, 0);
        case t_initialize:
            return initialize(tokens, next_token, symbols, 0);
        case t_fun:
            return function(tokens, next_token, symbols, 0);
        case t_static:
            return static_var(tokens, next_token, symbols, 0);
        case t_on_interrupt:
            return on_interrupt(tokens, next_token, symbols, 0);
        default:
            PANIC("invalid token at beginning of root statement\n");
    }
}

// The entry-point for the parser
// calls root_statement until all tokens are parsed.
void parse(Token *tokens, int token_num, SymbolTable *symbols) {
    // set up the special init function as function index 0
    Function func;
    func.func_args_index = -1;
    func.func_args_len = 0;
    func.ir_code_index = -1;
    func.ir_code_len = 0;
    func.func_vars_index = -1;
    func.func_vars_len = 0;
    func.returns = false;
    func.name.str = "____init";
    func.name.len = 8;
    int func_index = add_function(symbols, func);
    if (func_index != INIT_FUNC_INDEX) {
        PANIC("somehow the init function isn't index 0");
    }
    int next_token = 0;
    while (next_token < token_num) {
        next_token = root_statement(tokens, next_token, symbols);
    }
}