#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *fmt, ...) {
    va_list ap;	
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void *xmalloc(size_t size) {
    void *data = malloc(size);
    if (!data)
        die("out of memory\n");
    return data;
}
