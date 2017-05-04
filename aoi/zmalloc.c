#include "zmalloc.h"

#include <string.h>

void * zmalloc(size_t size) {
	if (size <= 0) {
		return NULL;
	}
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void * zcalloc(size_t size) {
	if (size <= 0) {
		return NULL;
	}
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void   zfree(void *ptr) {
	if (ptr == NULL) {
		return;
	}
	free(ptr);
}
