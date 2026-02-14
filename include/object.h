#ifndef GUSTAV_OBJECT_H
#define GUSTAV_OBJECT_H

#include "chunk.h"
#include <stdint.h>

#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_FUNCTION(value) ((function_t *)AS_OBJ(value))
#define AS_STRING(value) ((string_t *)AS_OBJ(value))
#define AS_CSTRING(value) (((string_t *)AS_OBJ(value))->chars)

typedef enum {
	OBJ_FUNCTION,
	OBJ_STRING,
} ObjType;

struct Obj {
	ObjType type;
	struct Obj *next;
};

typedef struct {
	struct Obj obj;
	size_t arity;
	chunk_t chunk;
	string_t *name;
} function_t;

struct string_t {
	obj_t obj;
	size_t length;
	char *chars;
	uint32_t hash;
};

function_t *new_function(void);

string_t *take_string(char *chars, size_t length);
string_t *copy_string(const char *chars, size_t length);

void print_object(value_t value);

static inline bool is_obj_type(value_t value, ObjType type)
{
	return (IS_OBJ(value) && AS_OBJ(value)->type == type);
}

uint32_t hash_string(const char *key, size_t length);

#endif // GUSTAV_OBJECT_H
