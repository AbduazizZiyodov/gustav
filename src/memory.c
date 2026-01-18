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

	if (result == NULL) {
		gustav_error(EXIT_FAILURE, "Can't perform reallocate");
	}

	return result;
}

static void free_object(obj_t *object)
{
	switch (object->type) {
	case OBJ_STRING: {
		obj_string_t *string = (obj_string_t *)object;
		LOG_TRACE("Freeing string object: object=%p string=%p\n",
			  object, string->chars);
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(obj_string_t, object);
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
