#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

// These are some useful constants in lexing and define valid characters
// for IDs as well as the characters that are considered "end" characters
// of a lexeme in some circumstances.
#define VALID_ID_CHARS_START "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define VALID_ID_CHARS_START_LEN 53
#define VALID_ID_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"
#define VALID_ID_CHARS_LEN 63
#define DELIM_CHARS "\0\n =(){}:;.,"
#define DELIM_CHARS_LEN 12

// This is a helper to turn a "Token" (defined below) into a printable string
#define CREATE_TOKEN_STRING(t) char lexeme[256];\
char token_str[512];\
strncpy(lexeme, t.lexeme.str, t.lexeme.len);\
lexeme[t.lexeme.len] = 0;\
sprintf(token_str, "%s(%s,%lu)",\
token_type_to_static_string(t.type), lexeme, t.int_value);

// This is the enum of all available token types in Lang808
// The top three are special sentinel values to help with the "lex" function
typedef enum {
    t_NONE = 0, // specifies a StringRef has not yet successfully matched a token type
    t_IGNORE, // used for comments and whitespace
    t_INVALID, // used when a sequence of characters cannot possibly match any token type

    // punctuation
    t_equals,
    t_leftparen,
    t_rightparen,
    t_leftbrace,
    t_rightbrace,
    t_colon,
    t_semicolon,
    t_dot,
    t_comma,
    t_at,
    t_bang,

    // operators
    t_shiftleft,
    t_equalsequals,
    t_lessthan,
    t_greaterthan,
    t_plus,
    t_minus,
    t_and,

    // keywords
    t_mmp,
    t_unused,
    t_initialize,
    t_on_interrupt,
    t_fun,
    t_static,
    t_while,
    t_if,
    t_else,
    t_return,

    // int types
    t_inttype, // u8, u16, etc.
    t_bf, // BitField8, etc.
    t_be, // BitEnum6, etc.

    // ids and literals
    t_id,
    t_intliteral,
} TokenType;

// A Token consists of its type, a StringRef to the actual lexeme and an int_value
// int_value is only populated for token types where it applies. For example, for an
// int literal of "0xF", then the int_value would be set to 15.
typedef struct _Token {
    TokenType type;
    StringRef lexeme;
    long int_value;
} Token;

// These functions are defined in lexer.c
TokenType token_type(StringRef str, char *lookahead);
int lex(char *source, int source_len, Token *tokens);
void print_tokens(Token *tokens, int token_num);
char *token_type_to_static_string(TokenType token_type);

#endif