#ifndef GUSTAV_MEMORY_H
#define GUSTAV_MEMORY_H

#include <stddef.h>

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (size_t)((capacity) * 2))

#define ALLOCATE(type, count) \
	(type *)reallocate(NULL, 0, sizeof(type) * (count))

#define GROW_ARRAY(type, pointer, old_count, new_count)                 \
	(type *)reallocate(pointer, sizeof(type) * (size_t)(old_count), \
			   sizeof(type) * (size_t)(new_count))

#define FREE_ARRAY(type, pointer, old_count) \
	reallocate(pointer, sizeof(type) * (size_t)(old_count), 0)

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

void *reallocate(void *pointer, size_t old_size, size_t new_size);
void free_objects(void);

#endif
