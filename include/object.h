#ifndef GUSTAV_OBJECT_H
#define GUSTAV_OBJECT_H

#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define AS_STRING(value) ((obj_string_t *)AS_OBJ(value))
#define AS_CSTRING(value) (((obj_string_t *)AS_OBJ(value))->chars)

typedef enum {
	OBJ_STRING,
} ObjType;

struct Obj {
	ObjType type;
	struct Obj *next;
};

struct obj_string_t {
	obj_t obj;
	size_t length;
	char *chars;
};

obj_string_t *take_string(char *chars, size_t length);
obj_string_t *copy_string(const char *chars, size_t length);

void print_object(value_t value);

static inline bool is_obj_type(value_t value, ObjType type)
{
	return (IS_OBJ(value) && AS_OBJ(value)->type == type);
}

#endif
