#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

#define MAX_SOURCE_LEN 65536


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: One argument is required: the path to the Lang808 source file.\n");
        return 1;
    }
    char *source_file_name = argv[1];
    printf("Compiling %s\n", source_file_name);

    char source[MAX_SOURCE_LEN];
    int source_len = 0;
    memset(source, 0, MAX_SOURCE_LEN);

    FILE *source_file = fopen(source_file_name, "r");
    if (source_file == NULL) {
        fprintf(stderr, "ERROR: Can't open source file.\n");
        return 2;
    }

    fseek(source_file, 0L, SEEK_END);
    source_len = ftell(source_file);

    if (source_len >= MAX_SOURCE_LEN) {
        fprintf(stderr, "ERROR: Source file is too big. Maximum is %d.\n", MAX_SOURCE_LEN - 1);
        return 2;
    }
    rewind(source_file);
    for (int i = 0; i < source_len; i++) {
        source[i] = getc(source_file);
    }

    Lexeme lexemes[MAX_SOURCE_LEN];
    memset(lexemes, 0, MAX_SOURCE_LEN * sizeof(Lexeme));

    lex(source, source_len, lexemes);

    char buf[256];
    for (int i = 0; lexemes[i].t != t_NONE; i++) {
        strncpy(buf, lexemes[i].s.str, lexemes[i].s.len);
        buf[lexemes[i].s.len] = 0;
        printf("%s(%s) ", token_type_to_static_string(lexemes[i].t), buf);
    }
    printf("\n");
}