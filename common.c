#include <stdbool.h>
#include <string.h>

#include "common.h"

// This takes two "StringRef"s and compares them
bool string_ref_eq(StringRef *s1, StringRef *s2) {
    STRINGREF_TO_CSTR1(s1, 512);
    STRINGREF_TO_CSTR2(s2, 512);
    LOG("string_ref_eq: s1: %s, s2: %s\n", cstr1, cstr2);
    if (s1->len != s2->len) {
        return false;
    }
    return strncmp(s1->str, s2->str, s1->len) == 0;
}