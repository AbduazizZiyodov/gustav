#include <stdlib.h>

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

	if (result == NULL)
		exit(EXIT_FAILURE);

	return result;
}

static void free_object(Obj *object)
{
	switch (object->type) {
	case OBJ_STRING: {
		ObjString *string = (ObjString *)object;
		LOG_TRACE("Freeing string object: object=%p string=%p\n",
			  object, string->chars);
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(ObjString, object);
	}
	}
}

void free_objects(void)
{
	Obj *object = vm.objects;

	while (object != NULL) {
		Obj *next = object->next;
		free_object(object);
		object = next;
	}
}
