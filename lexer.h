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
    t_equals,
    t_leftparen,
    t_rightparen,
    t_leftbrace,
    t_rightbrace,
    t_colon,
    t_semicolon,
    t_dot,
    t_comma,
    t_shiftleft,
    t_fun,
    t_unused,
    t_id,
    t_inthexliteral,
    t_intdecliteral
} TokenType;

typedef struct _Lexeme {
    TokenType t;
    StringRef s;
    long intliteral_value;
} Lexeme;

TokenType token_type(StringRef str, char *lookahead);
void lex(char *source, int source_len, Lexeme *lexemes);
char *token_type_to_static_string(TokenType token_type);

#endif