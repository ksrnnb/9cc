#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    exit(EXIT_FAILURE);
}