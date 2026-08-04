#pragma once

#include <stdint.h>

#include "chunk.h"
#include "hash_table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define BoundMethod_Check(value) Object_TypeCheck(value, OBJ_BOUND_METHOD)
#define Closure_Check(value) Object_TypeCheck(value, OBJ_CLOSURE)
#define Function_Check(value) Object_TypeCheck(value, OBJ_FUNCTION)
#define Native_Check(value) Object_TypeCheck(value, OBJ_NATIVE)
#define String_Check(value) Object_TypeCheck(value, OBJ_STRING)
#define Class_Check(value) Object_TypeCheck(value, OBJ_CLASS)
#define Instance_Check(value) Object_TypeCheck(value, OBJ_INSTANCE)

#define AS_CLOSURE(value) ((ClosureObject *)AS_OBJ(value))
#define AS_FUNCTION(value) ((FunctionObject *)AS_OBJ(value))
#define AS_NATIVE(value) (((NativeObject *)AS_OBJ(value))->function)
#define AS_STRING(value) ((StringObject *)AS_OBJ(value))
#define AS_CSTRING(value) (((StringObject *)AS_OBJ(value))->chars)
#define AS_CLASS(value) (((ClassObject *)AS_OBJ(value)))
#define AS_INSTANCE(value) (((InstanceObject *)AS_OBJ(value)))
#define AS_BOUND_METHOD(value) (((BoundMethodObject *)AS_OBJ(value)))

#define FOREACH_OBJECT_TYPE(DO) \
	DO(OBJ_CLOSURE)         \
	DO(OBJ_FUNCTION)        \
	DO(OBJ_NATIVE)          \
	DO(OBJ_STRING)          \
	DO(OBJ_UPVALUE)         \
	DO(OBJ_CLASS)           \
	DO(OBJ_INSTANCE)        \
	DO(OBJ_BOUND_METHOD)

typedef enum { FOREACH_OBJECT_TYPE(GENERATE_ENUM) } ObjectType;
static const char *OBJECT_TYPE_STRING[] = { FOREACH_OBJECT_TYPE(GENERATE_STRING) };

struct Object {
	ObjectType type;
	bool is_marked;
	struct Object *next;
};

typedef struct {
	struct Object obj;
	size_t arity;
	int upvalue_count;
	Chunk chunk;
	StringObject *name;
} FunctionObject;

typedef Value (*NativeFn)(int arg_count, Value *args);

typedef struct {
	struct Object obj;
	NativeFn function;
} NativeObject;

struct StringObject {
	Object obj;
	size_t length;
	char *chars;
	uint32_t hash;
};

typedef struct UpvalueObject {
	struct Object obj;
	Value *location;
	Value closed;
	struct UpvalueObject *next;
} UpvalueObject;

typedef struct {
	struct Object obj;
	FunctionObject *function;
	UpvalueObject **upvalues;
	int upvalue_count;
} ClosureObject;

typedef struct {
	struct Object obj;
	StringObject *name;
	HashTable methods;
} ClassObject;

typedef struct {
	struct Object obj;
	ClassObject *klass;
	HashTable fields;
} InstanceObject;

typedef struct {
	struct Object obj;
	Value receiver;
	ClosureObject *method;
} BoundMethodObject;

BoundMethodObject *new_bound_method(Value receiver, ClosureObject *method);
ClassObject *new_class(StringObject *name);
InstanceObject *new_instance(ClassObject *klass);

ClosureObject *new_closure(FunctionObject *function);

FunctionObject *new_function(void);
NativeObject *new_native(NativeFn function);

StringObject *string_from_owned_chars(char *chars, size_t length);
StringObject *string_from_chars(const char *chars, size_t length);

void init_interned_strings(void);
void free_interned_strings(void);
void sweep_interned_strings(void);

UpvalueObject *new_upvalue(Value *slot);

void Object_Print(FILE *stream, Value value);

static inline bool Object_TypeCheck(Value value, ObjectType type)
{
	return (Object_Check(value) && AS_OBJ(value)->type == type);
}

uint32_t hash_string(const char *key, size_t length);
