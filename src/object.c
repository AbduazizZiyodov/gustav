#include "chunk.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "hash_table.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type) (type *)allocate_object(sizeof(type), object_type)

static HashTable interned_strings;

void init_interned_strings(void)
{
	init_hash_table(&interned_strings);
}

void free_interned_strings(void)
{
	free_tash_table(&interned_strings);
}

void sweep_interned_strings(void)
{
	hash_table_remove_white(&interned_strings);
}

static Object *allocate_object(size_t size, ObjectType type)
{
	Object *object = (Object *)mem_realloc(NULL, 0, size);
	object->type = type;
	object->is_marked = false;
	object->next = gc.objects;
	gc.objects = object;

	LOG_GC("%p allocate %zu for %d\n", (void *)object, size, type);

	return object;
}

BoundMethodObject *new_bound_method(Value receiver, ClosureObject *method)
{
	BoundMethodObject *bound = ALLOCATE_OBJ(BoundMethodObject, OBJ_BOUND_METHOD);
	bound->receiver = receiver;
	bound->method = method;
	return bound;
}

ClassObject *new_class(StringObject *name)
{
	ClassObject *klass = ALLOCATE_OBJ(ClassObject, OBJ_CLASS);
	klass->name = name;
	init_hash_table(&klass->methods);
	return klass;
}

InstanceObject *new_instance(ClassObject *klass)
{
	InstanceObject *instance = ALLOCATE_OBJ(InstanceObject, OBJ_INSTANCE);
	instance->klass = klass;
	init_hash_table(&instance->fields);
	return instance;
}

ClosureObject *new_closure(FunctionObject *function)
{
	UpvalueObject **upvalues = ALLOCATE(UpvalueObject *, (size_t)function->upvalue_count);

	for (int i = 0; i < function->upvalue_count; i++) {
		upvalues[i] = NULL;
	}

	ClosureObject *closure = ALLOCATE_OBJ(ClosureObject, OBJ_CLOSURE);

	closure->function = function;
	closure->upvalues = upvalues;
	closure->upvalue_count = function->upvalue_count;

	return closure;
}

FunctionObject *new_function(void)
{
	FunctionObject *function = ALLOCATE_OBJ(FunctionObject, OBJ_FUNCTION);

	function->arity = 0;
	function->upvalue_count = 0;
	function->name = NULL;
	chunk_init(&function->chunk);

	return function;
}

NativeObject *new_native(NativeFn function)
{
	NativeObject *native = ALLOCATE_OBJ(NativeObject, OBJ_NATIVE);
	native->function = function;
	return native;
}

static StringObject *allocate_string(char *chars, size_t length, uint32_t hash)
{
	StringObject *string = ALLOCATE_OBJ(StringObject, OBJ_STRING);

	string->length = length;
	string->chars = chars;
	string->hash = hash;

	vm_push(OBJ_VAL(string));
	hash_table_set_item(&interned_strings, string, NIL_VAL);
	vm_pop();

	return string;
}

StringObject *string_from_chars(const char *chars, size_t length)
{
	uint32_t hash = hash_string(chars, length);

	StringObject *interned = hash_table_find_string(&interned_strings, chars, length, hash);

	if (interned != NULL) {
		return interned;
	}

	char *heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);

	heap_chars[length] = '\0';

	return allocate_string(heap_chars, length, hash);
}

UpvalueObject *new_upvalue(Value *slot)
{
	UpvalueObject *upvalue = ALLOCATE_OBJ(UpvalueObject, OBJ_UPVALUE);
	upvalue->location = slot;
	upvalue->closed = NIL_VAL;
	upvalue->next = NULL;
	return upvalue;
}

static void print_function(FILE *stream, FunctionObject *function)
{
	if (function->name == NULL) {
		fprintf(stream, "<script>");
		return;
	}

	fprintf(stream, "<fn %s/%zu>", function->name->chars, function->arity);
}

void Object_Print(FILE *stream, Value value)
{
	switch (OBJ_TYPE(value)) {
	case OBJ_STRING:
		fprintf(stream, "%s", AS_CSTRING(value));
		break;
	case OBJ_FUNCTION:
		print_function(stream, AS_FUNCTION(value));
		break;
	case OBJ_CLOSURE:
		print_function(stream, AS_CLOSURE(value)->function);
		break;
	case OBJ_NATIVE:
		fprintf(stream, "<native_fn>");
		break;
	case OBJ_UPVALUE:
		fprintf(stream, "<upvalue>");
		break;
	case OBJ_CLASS:
		fprintf(stream, "%s", AS_CLASS(value)->name->chars);
		break;
	case OBJ_INSTANCE:
		fprintf(stream, "%s<Instance>", AS_INSTANCE(value)->klass->name->chars);
		break;
	case OBJ_BOUND_METHOD:
		print_function(stream, AS_BOUND_METHOD(value)->method->function);
		break;
	}
}

StringObject *string_from_owned_chars(char *chars, size_t length)
{
	uint32_t hash = hash_string(chars, length);

	StringObject *interned = hash_table_find_string(&interned_strings, chars, length, hash);

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
