// This file contains the implementation of the Lang808 lexer
// The entrypoint of the lexer is the "lex" function defined at the bottom of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

// This is a helper function which turns the token type enum into a printable string
char *token_type_to_static_string(TokenType token_type) {
    switch (token_type) {
        case t_NONE: return "NONE";
        case t_IGNORE: return "IGNORE";
        case t_INVALID: return "INVALID";

        case t_equals: return "equals";
        case t_leftparen: return "leftparen";
        case t_rightparen: return "rightparen";
        case t_leftbrace: return "leftbrace";
        case t_rightbrace: return "rightbrace";
        case t_colon: return "colon";
        case t_semicolon: return "semicolon";
        case t_dot: return "dot";
        case t_comma: return "comma";
        case t_at: return "at";
        case t_bang: return "bang";

        case t_shiftleft: return "shiftleft";
        case t_equalsequals: return "equalsequals";
        case t_lessthan: return "lessthan";
        case t_greaterthan: return "greaterthan";
        case t_plus: return "plus";
        case t_minus: return "minus";
        case t_and: return "and";

        case t_mmp: return "mmp";
        case t_unused: return "unused";
        case t_initialize: return "initialize";
        case t_on_interrupt: return "on_interrupt";
        case t_fun: return "fun";
        case t_static: return "static";
        case t_while: return "while";
        case t_if: return "if";
        case t_else: return "else";
        case t_return: return "return";

        case t_inttype: return "inttype";
        case t_bf: return "bf";
        case t_be: return "be";

        case t_id: return "id";
        case t_intliteral: return "intliteral";
        default: return "ERROR_INVALID_TOKEN_TYPE";
    }
}

// A helper function which returns true if a character is present in a list of characters
bool char_is_in(char c, char *valid, int valid_len) {
    for (int i = 0; i < valid_len; i++) {
        if (c == valid[i]) {
            return true;
        }
    }
    return false;
}

// This detects if a StringRef is a full comment
// Comments continue till the end of the line, so this doesn't return true until the
// last character is a newline
bool is_comment(StringRef str) {
    if (str.len < 2) {
        return false;
    }
    if (str.str[0] == '#' && str.str[str.len - 1] == '\n') {
        return true;
    }
    return false;
}

// This detects if a StringRef matches "BitEnumX" where X is a number
bool is_be(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (str.len < 7 || strncmp(str.str, "BitEnum", 7) != 0) {
        return false;
    }
    for (int i = 7; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            return false;
        }
    }
    return true;
}
// This detects if a StringRef matches "BitFieldX" where X is a number
bool is_bf(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (str.len < 8 || strncmp(str.str, "BitField", 8) != 0) {
        return false;
    }
    for (int i = 8; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            return false;
        }
    }
    return true;
}

// This detects if a StringRef matches one of the supported int types of Lang808
// u8, u16 or u32
bool is_inttype(StringRef str, char *lookahead) {
    if (str.len == 2 && strncmp(str.str, "u8", str.len) == 0) {
        return true;
    } else if (str.len == 3 && strncmp(str.str, "u16", str.len) == 0) {
        return true;
    } else if (str.len == 3 && strncmp(str.str, "u32", str.len) == 0) {
        return true;
    } else {
        return false;
    }
}

// This detects if a StringRef is a valid id of Lang808.
// ids are typically variables or programmer defined field of some sort.
// ids must be alphanumeric and start with a letter, underscores are allowed
bool is_id(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (!char_is_in(str.str[0], VALID_ID_CHARS_START, VALID_ID_CHARS_START_LEN)) {
        return false;
    }
    for (int i = 1; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], VALID_ID_CHARS, VALID_ID_CHARS_LEN)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}

// This detects if a StringRef is a hex number of the form "0x12345"
bool is_inthexliteral(StringRef str, char *lookahead) {
    if (str.len < 3) {
        return false;
    }
    if (str.str[0] != '0' || str.str[1] != 'x') {
        return false;
    }
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    for (int i = 2; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890abcdefABCDEF", 22)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}
// This detects if a StringRef is a number
bool is_intdecliteral(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    for (int i = 0; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}
// This detects if a StringRef is either a (decimal) number or a hex number
bool is_intliteral(StringRef str, char *lookahead) {
    return is_intdecliteral(str, lookahead) || is_inthexliteral(str, lookahead);
}

// This detects the situation where a lexeme is invalid
// This is checked last in the token_type detection below, so if all other matches fail,
// but this one succeeds, then the lexeme is considered invalid.
bool is_invalid(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (str.str[0] == '#') {
        return false;
    }
    return true;
}

// Given a StringRef and a pointer to the next characters after the StringRef,
// this function returns a TokenType enum value for what kind of token it is.
// If the string is definitely invalid, then it returns t_INVALID.
// If the string is whitespace or a valid comment, then it returns t_IGNORE.
// If no token type can be determined, then it returns t_NONE.
TokenType token_type(StringRef str, char *lookahead) {
    if (str.str[0] == '(' && str.len == 1) {
        return t_leftparen;
    } else if (str.str[0] == ')' && str.len == 1) {
        return t_rightparen;
    } else if (str.str[0] == '{' && str.len == 1) {
        return t_leftbrace;
    } else if (str.str[0] == '}' && str.len == 1) {
        return t_rightbrace;
    } else if (str.str[0] == ':' && str.len == 1) {
        return t_colon;
    } else if (str.str[0] == ';' && str.len == 1) {
        return t_semicolon;
    } else if (str.str[0] == '.' && str.len == 1) {
        return t_dot;
    } else if (str.str[0] == ',' && str.len == 1) {
        return t_comma;
    } else if (str.str[0] == '@' && str.len == 1) {
        return t_at;
    } else if (str.str[0] == '!' && str.len == 1) {
        return t_bang;
    } else if (str.len == 2 && strncmp(str.str, "<<", str.len) == 0) {
        return t_shiftleft;
    } else if (str.str[0] == '<' && str.len == 1) {
        if (lookahead[0] == '<') {
            return t_NONE;
        }
        return t_lessthan;
    } else if (str.str[0] == '>' && str.len == 1) {
        return t_greaterthan;
    } else if (str.str[0] == '+' && str.len == 1) {
        return t_plus;
    } else if (str.str[0] == '-' && str.len == 1) {
        return t_minus;
    } else if (str.str[0] == '&' && str.len == 1) {
        return t_and;
    } else if (str.len == 2 && strncmp(str.str, "==", str.len) == 0) {
        return t_equalsequals;
    } else if (str.str[0] == '=' && str.len == 1) {
        if (lookahead[0] == '=') {
            return t_NONE;
        }
        return t_equals;
    } else if (str.len == 22 && strncmp(str.str, "MemoryMappedPeripheral", str.len) == 0) {
        return t_mmp;
    } else if (str.len == 7 && strncmp(str.str, "$unused", str.len) == 0) {
        return t_unused;
    } else if (str.len == 10 && strncmp(str.str, "initialize", str.len) == 0) {
        return t_initialize;
    } else if (str.len == 12 && strncmp(str.str, "on_interrupt", str.len) == 0) {
        return t_on_interrupt;
    } else if (str.len == 3 && strncmp(str.str, "fun", str.len) == 0) {
        return t_fun;
    } else if (str.len == 6 && strncmp(str.str, "static", str.len) == 0) {
        return t_static;
    } else if (str.len == 5 && strncmp(str.str, "while", str.len) == 0) {
        return t_while;
    } else if (str.len == 2 && strncmp(str.str, "if", str.len) == 0) {
        return t_if;
    } else if (str.len == 4 && strncmp(str.str, "else", str.len) == 0) {
        return t_else;
    } else if (str.len == 6 && strncmp(str.str, "return", str.len) == 0) {
        return t_return;
    } else if (is_inttype(str, lookahead)) {
        return t_inttype;
    } else if (is_bf(str, lookahead)) {
        return t_bf;
    } else if (is_be(str, lookahead)) {
        return t_be;
    } else if (is_intliteral(str, lookahead)) {
        return t_intliteral;
    } else if (str.len == 1 && char_is_in(str.str[0], " \n", 2)) {
        // ignore whitespace
        return t_IGNORE;
    } else if (is_comment(str)) {
        // ignore lines starting with #
        return t_IGNORE;
    } else if (is_id(str, lookahead)) {
        return t_id;
    } else if (is_invalid(str, lookahead)) {
        return t_INVALID;
    }
    return t_NONE;
}

// Given a Token, this function looks at the type and the lexeme, and parses
// and int value from the lexeme if appropriate for that token type.
// For example, for an int literal of "0xF", then this will return 15.
int int_value_for_token(Token token) {
    if (token.type == t_intliteral) {
        char buf[256];
        memset(buf, 0, 256);
        strncpy(buf, token.lexeme.str, token.lexeme.len);
        return strtol(buf, NULL, 0);
    } else if (token.type == t_inttype) {
        char buf[256];
        memset(buf, 0, 256);
        strncpy(buf, token.lexeme.str + 1, token.lexeme.len - 1);
        return strtol(buf, NULL, 0);
    } else if (token.type == t_bf) {
        char buf[256];
        memset(buf, 0, 256);
        strncpy(buf, token.lexeme.str + 8, token.lexeme.len - 8);
        return strtol(buf, NULL, 0);
    } else if (token.type == t_be) {
        char buf[256];
        memset(buf, 0, 256);
        strncpy(buf, token.lexeme.str + 7, token.lexeme.len - 7);
        return strtol(buf, NULL, 0);
    } else {
        return 0;
    }
}

// This iterates over the source code and calls the above functions to turn lexemes into Tokens
int lex(char *source, int source_len, Token *tokens) {
    int token_num = 0;

    StringRef curr_str = {source, 0};
    TokenType curr_tok = t_NONE;
    int i_lexed_so_far = 0;

    // iterate over the whole source file contents
    for (int i = 1; i < source_len; i++) {
        // create a StringRef to the current set of characters to check
        curr_str = (StringRef){source + i_lexed_so_far, i - i_lexed_so_far};
        // is the current set of characters a valid token?
        curr_tok = token_type(curr_str, source + i);
        LOG("got %s\n", token_type_to_static_string(curr_tok));

        // Handle special cases of token_type
        if (curr_tok == t_INVALID) {
            // In this case the program is invalid so panic
            char buf[256];
            strncpy(buf, curr_str.str, curr_str.len);
            buf[curr_str.len] = 0;
            PANIC("Found invalid token: %s\n", buf);
        } else if (curr_tok == t_IGNORE) {
            // In this case we just advance the index of what we've completed
            // lexing without saving anything
            i_lexed_so_far = i;
        } else if (curr_tok != t_NONE) {
            // In this case, we have a token!
            // save it into the tokens array
            tokens[token_num] = (Token){curr_tok, curr_str};
            tokens[token_num].int_value = int_value_for_token(tokens[token_num]);
            token_num += 1;
            // and advance the index of what we've completed lexing so far
            i_lexed_so_far = i;
        }
    }
    return token_num;
}

// This prints all the tokens in the tokens array
void print_tokens(Token *tokens, int token_num) {
    char buf[256];
    for (int i = 0; i < token_num; i++) {
        strncpy(buf, tokens[i].lexeme.str, tokens[i].lexeme.len);
        buf[tokens[i].lexeme.len] = 0;
        printf("%s(%s,%lu)    ", token_type_to_static_string(tokens[i].type), buf, tokens[i].int_value);
    }
    printf("\n");
}