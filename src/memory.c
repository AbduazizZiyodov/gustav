#include "chunk.h"
#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void *reallocate(void *pointer, size_t old_size __attribute__((unused)),
		 size_t new_size)
{
	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, new_size);

	if (result is NULL) {
		gustav_error(EXIT_FAILURE, "Can't perform reallocate");
	}

	return result;
}

static void free_object(obj_t *object)
{
	switch (object->type) {
	case OBJ_STRING: {
		string_t *string = (string_t *)object;
		LOG_TRACE("Freeing string object: object=%p string_repr=%s\n",
			  object, string->chars);
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(string_t, object);
		break;
	}
	case OBJ_FUNCTION: {
		function_t *function = (function_t *)object;
		free_chunk(&function->chunk);
		FREE(function_t, object);
		break;
	}
	}
}

void free_objects(void)
{
	obj_t *object = vm.objects;

	while (object != NULL) {
		obj_t *next = object->next;
		free_object(object);
		object = next;
	}
}
