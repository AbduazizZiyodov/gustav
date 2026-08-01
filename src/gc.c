#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "compiler.h"
#include "gc.h"
#include "hash_table.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void mark_object(obj_t *object)
{
	if (object == NULL) {
		return;
	}

	if (object->is_marked) {
		return;
	}

#ifdef DEBUG_LOG_GC
	LOG_GC("%p [mark] ", (void *)object);
	print_value(OBJ_VAL(object));
	printf("\n");
#endif

	object->is_marked = true;

	if (vm.gray_capacity < vm.gray_count + 1) {
		vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);

		obj_t **new_stack =
			(obj_t **)realloc((void *)vm.gray_stack,
					  sizeof(obj_t *) * vm.gray_capacity);

		if (new_stack == NULL) {
			free((void *)vm.gray_stack);
			_Exit(1);
		}

		vm.gray_stack = new_stack;
	}

	vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(value_t value)
{
	if (IS_OBJ(value)) {
		mark_object(AS_OBJ(value));
	}
}

static void mark_array(value_array_t *array)
{
	for (size_t i = 0; i < array->count; i++) {
		mark_value(array->values[i]);
	}
}

static void blackify(obj_t *object)
{
#ifdef DEBUG_LOG_GC
	LOG_GC("%p [blackify] ", (void *)object);
	print_value(OBJ_VAL(object));
	printf("\n");
#endif

	switch (object->type) {
	case OBJ_BOUND_METHOD: {
		ObjBoundMethod *bound = (ObjBoundMethod *)object;
		mark_value(bound->receiver);
		mark_object((obj_t *)bound->method);
		break;
	}
	case OBJ_CLASS: {
		ObjClass *klass = (ObjClass *)object;
		mark_object((obj_t *)klass->name);
		mark_table(&klass->methods);
		break;
	}
	case OBJ_INSTANCE: {
		ObjInstance *instance = (ObjInstance *)object;
		mark_object((obj_t *)instance->klass);
		mark_table(&instance->fields);
		break;
	}
	case OBJ_CLOSURE: {
		ObjClosure *closure = (ObjClosure *)object;
		mark_object((obj_t *)closure->function);
		for (int i = 0; i < closure->upvalue_count; i++) {
			mark_object((obj_t *)closure->upvalues[i]);
		}
		break;
	}
	case OBJ_FUNCTION: {
		function_t *function = (function_t *)object;
		mark_object((obj_t *)function->name);
		mark_array(&function->chunk.constants);
		break;
	}
	case OBJ_UPVALUE:
		mark_value(((ObjUpvalue *)object)->closed);
		break;
	case OBJ_NATIVE:
	case OBJ_STRING:
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

	mark_object((obj_t *)vm.init_string);
}

static void trace_references(void)
{
	while (vm.gray_count > 0) {
		obj_t *object = vm.gray_stack[--vm.gray_count];
		blackify(object);
	}
}

static void sweep(void)
{
	obj_t *previous = NULL;
	obj_t *object = vm.objects;

	while (object != NULL) {
		if (object->is_marked) {
			object->is_marked = false;
			previous = object;
			object = object->next;
		} else {
			obj_t *unreached = object;
			object = object->next;

			if (previous != NULL) {
				previous->next = object;
			} else {
				vm.objects = object;
			}

			free_object(unreached);
		}
	}
}

void collect_garbage(void)
{
#ifdef DEBUG_LOG_GC
	size_t before = vm.bytes_allocated;
	LOG_GC("== [GC BEGIN] ==\n");
#endif

	mark_roots();
	trace_references();
	ht_remove_white(&vm.strings);
	sweep();

	vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	LOG_GC("== [/GC BEGIN] ==\n");
	LOG_GC("Collected %zu bytes (from %zu to %zu) next at %zu\n",
	       before - vm.bytes_allocated, before, vm.bytes_allocated,
	       vm.next_gc);
#endif
}
