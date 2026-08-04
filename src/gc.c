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

GC gc;

void gc_init(void)
{
	gc.objects = NULL;

	gc.bytes_allocated = 0;
	gc.next_gc = (size_t)(1 * 1024 * 1024); // 1 MiB

	gc.gray_count = 0;
	gc.gray_capacity = 0;
	gc.gray_stack = NULL;
}

void gc_mark_object(Object *object)
{
	if (object == NULL) {
		return;
	}

	if (object->is_marked) {
		return;
	}

#ifdef DEBUG_LOG_GC
	LOG_GC("%p [mark] ", (void *)object);
	print_value(stdout, OBJ_VAL(object));
	printf("\n");
#endif

	object->is_marked = true;

	if (gc.gray_capacity < gc.gray_count + 1) {
		gc.gray_capacity = GROW_CAPACITY(gc.gray_capacity);

		Object **new_stack = (Object **)realloc((void *)gc.gray_stack,
							sizeof(Object *) * gc.gray_capacity);

		if (new_stack == NULL) {
			free((void *)gc.gray_stack);
			_Exit(1);
		}

		gc.gray_stack = new_stack;
	}

	gc.gray_stack[gc.gray_count++] = object;
}

void gc_mark_value(Value value)
{
	if (Object_Check(value)) {
		gc_mark_object(AS_OBJ(value));
	}
}

static void gc_mark_array(ValueArray *array)
{
	for (size_t i = 0; i < array->count; i++) {
		gc_mark_value(array->values[i]);
	}
}

static void gc_blackify(Object *object)
{
#ifdef DEBUG_LOG_GC
	LOG_GC("%p [gc_blackify] ", (void *)object);
	print_value(stdout, OBJ_VAL(object));
	printf("\n");
#endif

	switch (object->type) {
	case OBJ_BOUND_METHOD: {
		BoundMethodObject *bound = (BoundMethodObject *)object;
		gc_mark_value(bound->receiver);
		gc_mark_object((Object *)bound->method);
		break;
	}
	case OBJ_CLASS: {
		ClassObject *klass = (ClassObject *)object;
		gc_mark_object((Object *)klass->name);
		hash_table_mark(&klass->methods);
		break;
	}
	case OBJ_INSTANCE: {
		InstanceObject *instance = (InstanceObject *)object;
		gc_mark_object((Object *)instance->klass);
		hash_table_mark(&instance->fields);
		break;
	}
	case OBJ_CLOSURE: {
		ClosureObject *closure = (ClosureObject *)object;
		gc_mark_object((Object *)closure->function);
		for (int i = 0; i < closure->upvalue_count; i++) {
			gc_mark_object((Object *)closure->upvalues[i]);
		}
		break;
	}
	case OBJ_FUNCTION: {
		FunctionObject *function = (FunctionObject *)object;
		gc_mark_object((Object *)function->name);
		gc_mark_array(&function->chunk.constants);
		break;
	}
	case OBJ_UPVALUE:
		gc_mark_value(((UpvalueObject *)object)->closed);
		break;
	case OBJ_NATIVE:
	case OBJ_STRING:
		break;
	}
}

static void gc_mark_roots(void)
{
	vm_mark_roots();
	compiler_mark_roots();
}

static void gc_trace_references(void)
{
	while (gc.gray_count > 0) {
		Object *object = gc.gray_stack[--gc.gray_count];
		gc_blackify(object);
	}
}

static void gc_sweep(void)
{
	Object *previous = NULL;
	Object *object = gc.objects;

	while (object != NULL) {
		if (object->is_marked) {
			object->is_marked = false;
			previous = object;
			object = object->next;
		} else {
			Object *unreached = object;
			object = object->next;

			if (previous != NULL) {
				previous->next = object;
			} else {
				gc.objects = object;
			}

			mem_free_object(unreached);
		}
	}
}

void gc_collect(void)
{
#ifdef DEBUG_LOG_GC
	size_t before = gc.bytes_allocated;
	LOG_GC("== [GC BEGIN] ==\n");
#endif

	gc_mark_roots();
	gc_trace_references();
	sweep_interned_strings();
	gc_sweep();

	gc.next_gc = gc.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	LOG_GC("== [/GC BEGIN] ==\n");
	LOG_GC("Collected %zu bytes (from %zu to %zu) next at %zu\n", before - gc.bytes_allocated,
	       before, gc.bytes_allocated, gc.next_gc);
#endif
}
