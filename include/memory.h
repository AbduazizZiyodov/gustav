#pragma once

#include <stddef.h>

#include "value.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (size_t)((capacity) * 2))

#define ALLOCATE(type, count) (type *)Mem_Realloc(NULL, 0, sizeof(type) * (count))

#define GROW_ARRAY(type, pointer, old_count, new_count)                  \
	(type *)Mem_Realloc(pointer, sizeof(type) * (size_t)(old_count), \
			    sizeof(type) * (size_t)(new_count))

#define FREE_ARRAY(type, pointer, old_count)                                       \
	do {                                                                       \
		(void)Mem_Realloc(pointer, sizeof(type) * (size_t)(old_count), 0); \
		(pointer) = NULL;                                                  \
	} while (false)

#define FREE(type, pointer)                                  \
	do {                                                 \
		(void)Mem_Realloc(pointer, sizeof(type), 0); \
		(pointer) = NULL;                            \
	} while (false)

void *Mem_Realloc(void *pointer, size_t old_size, size_t new_size);
void Mem_FreeObjects(void);
void Mem_FreeObject(Object *object);
