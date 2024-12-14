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
    void *ptr= malloc(size);
    if (!ptr)
        die("out of memory\n");
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (!ptr)
        die("out of memory\n");
    return ptr;
}

void *xcalloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (!ptr)
        die("out of memory\n");
    return ptr;
}
