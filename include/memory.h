#ifndef GUSTAV_MEMORY_H
#define GUSTAV_MEMORY_H
#include <stddef.h>

#include "value.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (size_t)((capacity) * 2))

#define ALLOCATE(type, count) \
	(type *)reallocate(NULL, 0, sizeof(type) * (count))

#define GROW_ARRAY(type, pointer, old_count, new_count)                 \
	(type *)reallocate(pointer, sizeof(type) * (size_t)(old_count), \
			   sizeof(type) * (size_t)(new_count))

#define FREE_ARRAY(type, pointer, old_count)                                  \
	do {                                                                  \
		(void)reallocate(pointer, sizeof(type) * (size_t)(old_count), \
				 0);                                          \
		(pointer) = NULL;                                             \
	} while (false)

#define FREE(type, pointer)                                 \
	do {                                                \
		(void)reallocate(pointer, sizeof(type), 0); \
		(pointer) = NULL;                           \
	} while (false)

void *reallocate(void *pointer, size_t old_size, size_t new_size);
void mark_object(obj_t *object);
void mark_value(value_t value);
void collect_garbage(void);
void free_objects(void);

#endif // GUSTAV_MEMORY_H
