#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

#define VALID_ID_CHARS_START "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define VALID_ID_CHARS_START_LEN 53
#define VALID_ID_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"
#define VALID_ID_CHARS_LEN 63
#define DELIM_CHARS "\0\n =(){}:;.,"
#define DELIM_CHARS_LEN 12

typedef enum {
    t_NONE = 0,
    t_IGNORE,
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

    // keywords
    t_mmp,
    t_unused,
    t_initialize,
    t_on_interrupt,
    t_fun,

    // int types
    t_inttype, // u8, u16, etc.
    t_bf, // BitField8, etc.
    t_be, // BitEnum6, etc.

    // ids and literals
    t_id,
    t_inthexliteral,
    t_intdecliteral
} TokenType;

typedef struct _Token {
    TokenType type;
    StringRef lexeme;
    long int_value;
} Token;

TokenType token_type(StringRef str, char *lookahead);
int lex(char *source, int source_len, Token *tokens);
char *token_type_to_static_string(TokenType token_type);

#endif