#pragma once

#include <stdlib.h>

void die(const char *fmt, ...);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nmemb, size_t size);
