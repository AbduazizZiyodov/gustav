#include <stdlib.h>

#include <memory.h>

void *reallocate(void *pointer, size_t old_size __attribute__((unused)),
		 size_t new_size)
{
	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, new_size);

	if (result == NULL)
		exit(EXIT_FAILURE);

	return result;
}
