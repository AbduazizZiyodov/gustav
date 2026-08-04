#pragma once

#include <stddef.h>

#include "value.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (size_t)((capacity) * 2))

#define ALLOCATE(type, count) (type *)mem_realloc(NULL, 0, sizeof(type) * (count))

#define GROW_ARRAY(type, pointer, old_count, new_count)                  \
	(type *)mem_realloc(pointer, sizeof(type) * (size_t)(old_count), \
			    sizeof(type) * (size_t)(new_count))

#define FREE_ARRAY(type, pointer, old_count)                                       \
	do {                                                                       \
		(void)mem_realloc(pointer, sizeof(type) * (size_t)(old_count), 0); \
		(pointer) = NULL;                                                  \
	} while (false)

#define FREE(type, pointer)                                  \
	do {                                                 \
		(void)mem_realloc(pointer, sizeof(type), 0); \
		(pointer) = NULL;                            \
	} while (false)

void *mem_realloc(void *pointer, size_t old_size, size_t new_size);
void mem_free_objects(void);
void mem_free_object(Object *object);
