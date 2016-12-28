#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "debuglog.h"


int debuglev = 3;

void die(char *format, ...) {
    fprintf(stderr, "FATAL: ");

    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);

    va_end(args);

    fprintf(stderr, "\n");

}

void warn(char *format, ...) {
    fprintf(stderr, "WARNING: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void debug(int level, char *format, ...) {
    if (level > debuglev)
        return;
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

