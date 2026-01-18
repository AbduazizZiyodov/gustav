#ifndef GUSTAV_VALUE_H
#define GUSTAV_VALUE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ } ValueType;

typedef struct Obj obj_t;
typedef struct string_t string_t;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		obj_t *obj;
	} as;
} value_t;

typedef struct {
	size_t count;
	size_t capacity;
	value_t *values;
} value_array_t;

#define AS_BOOL(value) (((value)).as.boolean)
#define AS_NUMBER(value) (((value)).as.number)
#define AS_OBJ(value) (((value)).as.obj)

#define BOOL_VAL(value) ((value_t){ VAL_BOOL, { .boolean = (value) } })
#define NIL_VAL ((value_t){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((value_t){ VAL_NUMBER, { .number = (value) } })
#define OBJ_VAL(value) ((value_t){ VAL_OBJ, { .obj = (obj_t *)(value) } })

#define IS(TYPE, value) ((value).type == VAL_##TYPE)
#define IS_BOOL(value) IS(BOOL, value)
#define IS_NIL(value) IS(NIL, value)
#define IS_NUMBER(value) IS(NUMBER, value)
#define IS_OBJ(value) IS(OBJ, value)

bool values_equal(value_t a, value_t b);

void init_value_array(value_array_t *value_array);
void free_value_array(value_array_t *value_array);
void write_value_array(value_array_t *value_array, value_t value);

void print_value(value_t value);

#endif
