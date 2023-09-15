#include "parser.h"

int statement(Token *tokens, int next_token) {
    switch (tokens[next_token].type) {
        case t_id:
            printf("id\n");
        default:
            panic("invalid token at beginning of statement");
    }
    return 9999999;
}

int parse(Token *tokens, int token_num) {
    int next_token = 0;
    while (next_token < token_num) {
        next_token = statement(tokens, next_token);
    }
}