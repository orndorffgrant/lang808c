#ifndef COMMON_H
#define COMMON_H

#ifndef DEBUG_LOG
#define DEBUG_LOG 0
#endif
#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)

typedef struct _StringRef {
    char *str;
    int len;
} StringRef;

#endif