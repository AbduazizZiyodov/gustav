#include "chunk.h"
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "hash_table.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include "log.h"
#include <stdio.h>
#endif

void *reallocate(void *pointer, size_t old_size __attribute__((unused)),
		 size_t new_size)
{
	if (new_size > old_size) {
#ifdef DEBUG_STRESS_GC
		collect_garbage();
#endif
	}

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

void mark_object(obj_t *object)
{
	if (object == NULL) {
		return;
	}
#ifdef DEBUG_LOG_GC
	LOG_TRACE("%p mark ", (void *)object);
	print_value(OBJ_VAL(object));
	printf("\n");
#endif
	object->is_marked = true;
}

void mark_value(value_t value)
{
	if (IS_OBJ(value)) {
		mark_object(AS_OBJ(value));
	}
}

static void free_object(obj_t *object)
{
	ObjClosure *closure;

	switch (object->type) {
	case OBJ_STRING: {
		string_t *string = (string_t *)object;
#ifdef DEBUG_LOG_GC
		LOG_TRACE("Freeing string object: object=%p string_repr=%s\n",
			  object, string->chars);
#endif
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
	case OBJ_NATIVE:
		FREE(ObjNative, object);
		break;
	case OBJ_CLOSURE:
		closure = (ObjClosure *)object;
		/*NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)*/
		FREE_ARRAY(ObjUpvalue *, closure->upvalues,
			   closure->upvalue_count);
		FREE(ObjClosure, object);
		break;
	case OBJ_UPVALUE:
		FREE(ObjUpvalue, object);
		break;
	}
}

static void mark_roots(void)
{
	for (value_t *slot = vm.stack; slot < vm.stack_top; slot++) {
		mark_value(*slot);
	}

	for (size_t i = 0; i < vm.frame_count; i++) {
		mark_object((obj_t *)vm.frames[i].closure);
	}

	for (ObjUpvalue *upvalue = vm.open_upvalues; upvalue != NULL;
	     upvalue = upvalue->next) {
		mark_object((obj_t *)upvalue);
	}

	mark_table(&vm.globals);
	mark_compiler_roots();
}

void collect_garbage(void)
{
#ifdef DEBUG_LOG_GC
	LOG_TRACE("== [GC BEGIN] ==\n");
#endif

	mark_roots();

#ifdef DEBUG_LOG_GC
	LOG_TRACE("== [/GC BEGIN] ==\n");
#endif
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
