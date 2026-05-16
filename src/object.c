#include "chunk.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hash_table.h"
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
	object->is_marked = false;
	object->next = vm.objects;
	vm.objects = object;

	LOG_GC("%p allocate %zu for %d\n", (void *)object, size, type);

	return object;
}

ObjBoundMethod *new_bound_method(value_t receiver, ObjClosure *method)
{
	ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
	bound->receiver = receiver;
	bound->method = method;
	return bound;
}

ObjClass *new_class(string_t *name)
{
	ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
	klass->name = name;
	init_hash_table(&klass->methods);
	return klass;
}

ObjInstance *new_instance(ObjClass *klass)
{
	ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
	instance->klass = klass;
	init_hash_table(&instance->fields);
	return instance;
}

ObjClosure *new_closure(function_t *function)
{
	ObjUpvalue **upvalues =
		ALLOCATE(ObjUpvalue *, (size_t)function->upvalue_count);

	for (int i = 0; i < function->upvalue_count; i++) {
		upvalues[i] = NULL;
	}

	ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);

	closure->function = function;
	closure->upvalues = upvalues;
	closure->upvalue_count = function->upvalue_count;

	return closure;
}

function_t *new_function(void)
{
	function_t *function = ALLOCATE_OBJ(function_t, OBJ_FUNCTION);

	function->arity = 0;
	function->upvalue_count = 0;
	function->name = NULL;
	init_chunk(&function->chunk);

	return function;
}

ObjNative *new_native(native_fn function)
{
	ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
	native->function = function;
	return native;
}

static string_t *allocate_string(char *chars, size_t length, uint32_t hash)
{
	string_t *string = ALLOCATE_OBJ(string_t, OBJ_STRING);

	string->length = length;
	string->chars = chars;
	string->hash = hash;

	push(OBJ_VAL(string));
	ht_insert(&vm.strings, string, NIL_VAL);
	pop();

	return string;
}

string_t *copy_string(const char *chars, size_t length)
{
	uint32_t hash = hash_string(chars, length);

	string_t *interned = ht_find_string(&vm.strings, chars, length, hash);

	if (interned != NULL) {
		return interned;
	}

	char *heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);

	heap_chars[length] = '\0';

	return allocate_string(heap_chars, length, hash);
}

ObjUpvalue *new_upvalue(value_t *slot)
{
	ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
	upvalue->location = slot;
	upvalue->closed = NIL_VAL;
	upvalue->next = NULL;
	return upvalue;
}

static void print_function(function_t *function)
{
	if (function->name == NULL) {
		printf("<script>");
		return;
	}

	printf("<fn %s/%zu>", function->name->chars, function->arity);
}

void print_object(value_t value)
{
	switch (OBJ_TYPE(value)) {
	case OBJ_STRING:
		printf("%s", AS_CSTRING(value));
		break;
	case OBJ_FUNCTION:
		print_function(AS_FUNCTION(value));
		break;
	case OBJ_CLOSURE:
		print_function(AS_CLOSURE(value)->function);
		break;
	case OBJ_NATIVE:
		printf("<native_fn>");
		break;
	case OBJ_UPVALUE:
		printf("<upvalue>");
		break;
	case OBJ_CLASS:
		printf("%s", AS_CLASS(value)->name->chars);
		break;
	case OBJ_INSTANCE:
		printf("%s<Instance>", AS_INSTANCE(value)->klass->name->chars);
		break;
	case OBJ_BOUND_METHOD:
		print_function(AS_BOUND_METHOD(value)->method->function);
		break;
	}
}

string_t *take_string(char *chars, size_t length)
{
	uint32_t hash = hash_string(chars, length);

	string_t *interned = ht_find_string(&vm.strings, chars, length, hash);

	if (interned != NULL) {
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}

	return allocate_string(chars, length, hash);
}

uint32_t hash_string(const char *key, size_t length)
{
	uint32_t hash = 2166136261U; // axuyet

	for (size_t i = 0; i < length; i++) {
		hash ^= (uint8_t)key[i];
		hash *= 16777619; // axuyet
	}

	return hash;
}
