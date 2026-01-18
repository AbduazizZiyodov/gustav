#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type) \
	(type *)allocate_object(sizeof(type), object_type)

static obj_t *allocate_object(size_t size, ObjType type)
{
	obj_t *object = (obj_t *)reallocate(NULL, 0, size);
	object->type = type;

	object->next = vm.objects;
	vm.objects = object;

	return object;
}

static obj_string_t *allocate_string(char *chars, size_t length)
{
	obj_string_t *string = ALLOCATE_OBJ(obj_string_t, OBJ_STRING);

	string->length = length;
	string->chars = chars;

	return string;
}

obj_string_t *copy_string(const char *chars, size_t length)
{
	char *heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);

	heap_chars[length] = '\0';

	return allocate_string(heap_chars, length);
}

void print_object(value_t value)
{
	switch (OBJ_TYPE(value)) {
	case OBJ_STRING:
		printf("%s", AS_CSTRING(value));
		break;
	}
}

obj_string_t *take_string(char *chars, size_t length)
{
	return allocate_string(chars, length);
}
