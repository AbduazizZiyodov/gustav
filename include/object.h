#ifndef GUSTAV_OBJECT_H
#define GUSTAV_OBJECT_H

#include "chunk.h"
#include <stdint.h>

#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((function_t *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((string_t *)AS_OBJ(value))
#define AS_CSTRING(value) (((string_t *)AS_OBJ(value))->chars)

typedef enum {
	OBJ_CLOSURE,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_STRING,
	OBJ_UPVALUE,
} ObjType;

struct Obj {
	ObjType type;
	struct Obj *next;
};

typedef struct {
	struct Obj obj;
	size_t arity;
	int upvalue_count;
	chunk_t chunk;
	string_t *name;
} function_t;

typedef value_t (*native_fn)(size_t arg_count, value_t *args);

typedef struct {
	struct Obj obj;
	native_fn function;
} ObjNative;

struct string_t {
	obj_t obj;
	size_t length;
	char *chars;
	uint32_t hash;
};

typedef struct ObjUpvalue {
	struct Obj obj;
	value_t *location;
	value_t closed;
	struct ObjUpvalue *next;
} ObjUpvalue;

typedef struct {
	struct Obj obj;
	function_t *function;
	ObjUpvalue **upvalues;
	int upvalue_count;
} ObjClosure;

ObjClosure *new_closure(function_t *function);

function_t *new_function(void);
ObjNative *new_native(native_fn function);

string_t *take_string(char *chars, size_t length);
string_t *copy_string(const char *chars, size_t length);

ObjUpvalue *new_upvalue(value_t *slot);

void print_object(value_t value);

static inline bool is_obj_type(value_t value, ObjType type)
{
	return (IS_OBJ(value) && AS_OBJ(value)->type == type);
}

uint32_t hash_string(const char *key, size_t length);

#endif // GUSTAV_OBJECT_H
