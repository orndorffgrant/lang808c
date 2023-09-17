#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG_LOG
#define DEBUG_LOG 0
#endif
#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)
#ifndef PRINT_PARSE_TREE
#define PRINT_PARSE_TREE 1
#endif
#define PARSE_TREE_PRINT(...) if (PRINT_PARSE_TREE) printf(__VA_ARGS__)
#define PANIC(...) fprintf(stderr, __VA_ARGS__); exit(1);

typedef struct _StringRef {
    char *str;
    int len;
} StringRef;

#endif