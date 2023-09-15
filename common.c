#include "common.h"

void panic(char * msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}