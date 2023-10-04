#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// These are some macros that make Logging easy to turn off and on without
// commenting/uncommenting all the log statements
#ifndef DEBUG_LOG
#define DEBUG_LOG 0
#endif
#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)

// This is a helper macro to easily print an error message and end the program
#define PANIC(...) fprintf(stderr, __VA_ARGS__); exit(1);

// All strings in the compiler are "StringRef"s as defined here.
// This struct includes the length of the string and a char * that is NOT necessarily
// null-terminated (and that's okay because we have the length).
typedef struct _StringRef {
    char *str;
    int len;
} StringRef;

// These are helpers for turning StringRefs into local null-terminated char arrays.
// These are useful for printing StringRefs in output
#define STRINGREF_TO_CSTR1(strref, size) char cstr1[size];\
    strncpy(cstr1, (strref)->str, (strref)->len);\
    cstr1[(strref)->len] = 0;
#define STRINGREF_TO_CSTR2(strref, size) char cstr2[size];\
    strncpy(cstr2, (strref)->str, (strref)->len);\
    cstr2[(strref)->len] = 0;

// A comparison function for StringRefs, defined in common.c
bool string_ref_eq(StringRef *s1, StringRef *s2);

#endif