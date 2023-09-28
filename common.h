#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEBUG_LOG
#define DEBUG_LOG 0
#endif
#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)

#define PANIC(...) fprintf(stderr, __VA_ARGS__); exit(1);

typedef struct _StringRef {
    char *str;
    int len;
} StringRef;

#define STRINGREF_TO_CSTR1(strref, size) char cstr1[size];\
    strncpy(cstr1, (strref)->str, (strref)->len);\
    cstr1[(strref)->len] = 0;
#define STRINGREF_TO_CSTR2(strref, size) char cstr2[size];\
    strncpy(cstr2, (strref)->str, (strref)->len);\
    cstr2[(strref)->len] = 0;

bool string_ref_eq(StringRef *s1, StringRef *s2);

#endif