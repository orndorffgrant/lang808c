#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "symbols.h"
#include "lexer.h"
#include "parser.h"

#define MAX_SOURCE_LEN 65536

// This is the entrypoint of the compiler
// It checks for one command-line argument and uses that as the filename of a lang808 source file
// It reads the whole file, passes it through the lexer, and then parses it.
int main(int argc, char *argv[]) {
    // Check that one argument was supplied
    if (argc != 2) {
        fprintf(stderr, "ERROR: One argument is required: the path to the Lang808 source file.\n");
        return 1;
    }
    char *source_file_name = argv[1];
    printf("Compiling %s\n", source_file_name);

    // Create a buffer to read the source file into
    char source[MAX_SOURCE_LEN];
    int source_len = 0;
    memset(source, 0, MAX_SOURCE_LEN);

    // Open the source file
    FILE *source_file = fopen(source_file_name, "r");
    if (source_file == NULL) {
        fprintf(stderr, "ERROR: Can't open source file.\n");
        return 2;
    }

    // Check that the source file is not bigger than the buffer we pre-allocated
    fseek(source_file, 0L, SEEK_END);
    source_len = ftell(source_file);
    if (source_len >= MAX_SOURCE_LEN) {
        fprintf(stderr, "ERROR: Source file is too big. Maximum is %d.\n", MAX_SOURCE_LEN - 1);
        return 2;
    }
    rewind(source_file);

    // Read the contents of the source file into the buffer
    for (int i = 0; i < source_len; i++) {
        source[i] = getc(source_file);
    }

    // Initialize an array of tokens which the "lex" function will populate
    // "Token" is defined in "lexer.h"
    Token tokens[MAX_SOURCE_LEN];
    memset(tokens, 0, sizeof(tokens));
    // Pass the source to the lexer. It returns the total number of tokens,
    // and populates the tokens array
    // "lex" is declared in "lexer.h" and defined in "lexer.c"
    int token_num = lex(source, source_len, tokens);
    //print_tokens(tokens, token_num);

    // Initialize the symbol table
    // "SymbolTable" is defined in "symbols.h" and some related helper functions
    // are defined in "symbols.c"
    SymbolTable symbols;
    memset(&symbols, 0, sizeof(symbols));
    // Pass the tokens to the "parse" function, which will populate the symbol table
    // "parse" is declared in "parser.h" and defined in "parser.c"
    parse(tokens, token_num, &symbols);
}