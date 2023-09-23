#include "parser.h"
#include "common.h"
#include "lexer.h"


int match(TokenType token_type, Token *tokens, int next_token, int indent) {
    if (tokens[next_token].type != token_type) {
        PANIC(
            "Expected %s but found %s",
            token_type_to_static_string(token_type),
            token_type_to_static_string(tokens[next_token].type)
        );
    }
    CREATE_TOKEN_STRING(tokens[next_token]);
    PARSE_TREE_INDENT(indent); PARSE_TREE_PRINT("- %s\n", token_str);
    return next_token + 1;
}

int name(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Name:\n");
    next_token = match(t_id, tokens, next_token, indent);
    while (tokens[next_token].type == t_dot) {
        next_token = match(t_dot, tokens, next_token, indent);
        next_token = match(t_id, tokens, next_token, indent);
    }
    return next_token;
}

int expression(Token *tokens, int next_token, int indent);

int function_call(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- FunctionCall:\n");
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightparen) {
        next_token = expression(tokens, next_token, indent);
        if (tokens[next_token].type != t_rightparen) {
            next_token = match(t_comma, tokens, next_token, indent);
        }
    }
    next_token = match(t_rightparen, tokens, next_token, indent);
    return next_token;
}
int expression_term(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ExpressionTerm:\n");
    if (tokens[next_token].type == t_intliteral) {
        next_token = match(t_intliteral, tokens, next_token, indent);
    } else {
        next_token = name(tokens, next_token, indent);
    }
    return next_token;
}
int expression_shift(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ShiftExpression:\n");
    next_token = expression_term(tokens, next_token, indent);
    switch (tokens[next_token].type) {
        case t_shiftleft:
            next_token = match(t_shiftleft, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_term(tokens, next_token, indent);
    return next_token;
}
int expression_bit(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitExpression:\n");
    next_token = expression_shift(tokens, next_token, indent);
    switch (tokens[next_token].type) {
        case t_and:
            next_token = match(t_and, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_shift(tokens, next_token, indent);
    return next_token;
}
int expression_sum(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- SumExpression:\n");
    next_token = expression_bit(tokens, next_token, indent);
    switch (tokens[next_token].type) {
        case t_plus:
            next_token = match(t_plus, tokens, next_token, indent);
            break;
        default:
            return next_token;
    }
    next_token = expression_bit(tokens, next_token, indent);
    return next_token;
}
int expression(Token *tokens, int next_token, int indent) {
    // top level expression is comparison
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Expression:\n");
    next_token = expression_sum(tokens, next_token, indent);
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
    next_token = expression_sum(tokens, next_token, indent);
    return next_token;
}

int bitfield_value(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldValue:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldValueItem:\n");
        // TODO actually process
        next_token = match(t_id, tokens, next_token, indent);
        next_token = match(t_equals, tokens, next_token, indent);
        if (tokens[next_token].type == t_intliteral) {
            // TODO actually process
            next_token = match(t_intliteral, tokens, next_token, indent);
        } else {
            // TODO actually process
            next_token = match(t_id, tokens, next_token, indent);
        }
        next_token = match(t_semicolon, tokens, next_token, indent);
    }

    indent--;
    next_token = match(t_rightbrace, tokens, next_token, indent);

    return next_token;
}

int mmp_def_structure_item_bf_item_enum_item(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnumItem:\n");
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_intliteral, tokens, next_token, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf_item_enum(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitEnum:\n");
    // TODO actually process
    next_token = match(t_be, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of bf items
        next_token = mmp_def_structure_item_bf_item_enum_item(tokens, next_token, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf_item(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitFieldItem:\n");
    if (tokens[next_token].type == t_id) {
        // TODO actually process
        next_token = match(t_id, tokens, next_token, indent);
    } else {
        // TODO actually process
        next_token = match(t_unused, tokens, next_token, indent);
    }
    next_token = match(t_colon, tokens, next_token, indent);
    if (tokens[next_token].type == t_intliteral) {
        // TODO actually process
        next_token = match(t_intliteral, tokens, next_token, indent);
    } else {
        // TODO
        next_token = mmp_def_structure_item_bf_item_enum(tokens, next_token, indent);
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item_bf(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BitField:\n");
    // TODO actually process
    next_token = match(t_bf, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of bf items
        next_token = mmp_def_structure_item_bf_item(tokens, next_token, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure_item(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- StructureItem:\n");
    if (tokens[next_token].type == t_id) {
        // TODO actually process
        next_token = match(t_id, tokens, next_token, indent);
    } else {
        // TODO actually process
        next_token = match(t_unused, tokens, next_token, indent);
    }
    next_token = match(t_colon, tokens, next_token, indent);
    if (tokens[next_token].type == t_inttype) {
        // TODO actually process
        next_token = match(t_inttype, tokens, next_token, indent);
    } else {
        next_token = mmp_def_structure_item_bf(tokens, next_token, indent);
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int mmp_def_structure(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Structure:\n");
    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type == t_id || tokens[next_token].type == t_unused) {
        // any number of structure items
        next_token = mmp_def_structure_item(tokens, next_token, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int mmp_def_opt_interrupt_num(Token *tokens, int next_token, int indent) {
    if (tokens[next_token].type != t_bang) {
        PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- NoInterruptNum:\n");
        return next_token;
    }
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- InterruptNum:\n");
    next_token = match(t_bang, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_intliteral, tokens, next_token, indent);
    return next_token;
}
int mmp_def_base_address(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- BaseAddress:\n");
    next_token = match(t_at, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_intliteral, tokens, next_token, indent);
    return next_token;
}
int mmp_def(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- MemoryMappedPeripheral:\n");
    next_token = match(t_mmp, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = mmp_def_base_address(tokens, next_token, indent);
    next_token = mmp_def_opt_interrupt_num(tokens, next_token, indent);
    next_token = mmp_def_structure(tokens, next_token, indent);
    return next_token;
}
int initialize_statement(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- InitializeStatement:\n");
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_equals, tokens, next_token, indent);

    if (tokens[next_token].type == t_leftbrace) {
        next_token = bitfield_value(tokens, next_token, indent);
    } else {
        // TODO actually process
        next_token = match(t_intliteral, tokens, next_token, indent);
    }
    next_token = match(t_semicolon, tokens, next_token, indent);
    return next_token;
}
int initialize(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Initialize:\n");
    next_token = match(t_initialize, tokens, next_token, indent);
    // TODO actually process
    next_token = match(t_id, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of intialization statements
        next_token = initialize_statement(tokens, next_token, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}

int function_statement(Token *tokens, int next_token, int indent);

int function_statement_assignment(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- Assignment:\n");
    next_token = name(tokens, next_token, indent);
    next_token = match(t_equals, tokens, next_token, indent);
    next_token = expression(tokens, next_token, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);
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
int function_statement_for_loop(Token *tokens, int next_token, int indent) {
    PARSE_TREE_INDENT(indent); indent++; PARSE_TREE_PRINT("- ForLoop:\n");
    next_token = match(t_for, tokens, next_token, indent);
    next_token = match(t_leftparen, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, indent);

    next_token = expression(tokens, next_token, indent);
    next_token = match(t_semicolon, tokens, next_token, indent);

    next_token = function_statement(tokens, next_token, indent);

    next_token = match(t_rightparen, tokens, next_token, indent);
    next_token = match(t_leftbrace, tokens, next_token, indent);

    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, indent);
    }

    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int function_statement(Token *tokens, int next_token, int indent) {
    switch (tokens[next_token].type) {
        case t_for:
            return function_statement_for_loop(tokens, next_token, indent);
        case t_inttype:
            return function_statement_local_var(tokens, next_token, indent);
        case t_id:
            if (tokens[next_token+1].type == t_leftparen) {
                next_token = function_call(tokens, next_token, indent);
                next_token = match(t_semicolon, tokens, next_token, indent);
                return next_token;
            } else {
                return function_statement_assignment(tokens, next_token, indent);
            }
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
int function(Token *tokens, int next_token, int indent) {
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

    next_token = match(t_leftbrace, tokens, next_token, indent);
    while (tokens[next_token].type != t_rightbrace) {
        // any number of function statements
        next_token = function_statement(tokens, next_token, indent);
    }
    next_token = match(t_rightbrace, tokens, next_token, indent);
    return next_token;
}
int root_statement(Token *tokens, int next_token) {
    switch (tokens[next_token].type) {
        case t_mmp:
            return mmp_def(tokens, next_token, 0);
        case t_initialize:
            return initialize(tokens, next_token, 0);
        case t_fun:
            return function(tokens, next_token, 0);
        default:
            PANIC("invalid token at beginning of root statement\n");
    }
}

int parse(Token *tokens, int token_num) {
    int next_token = 0;
    while (next_token < token_num) {
        next_token = root_statement(tokens, next_token);
    }
}