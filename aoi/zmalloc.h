#pragma once

#define random rand

#include <stdlib.h>

void * zmalloc(size_t size);
void * zcalloc(size_t size);
void   zfree(void *ptr);
