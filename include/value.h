#ifndef GUSTAV_VALUE_H
#define GUSTAV_VALUE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER } ValueType;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
	} as;
} Value;

typedef struct {
	size_t count;
	size_t capacity;
	Value *values;
} ValueArray;

#define AS_BOOL(value) (((value)).as.boolean)
#define AS_NUMBER(value) (((value)).as.number)

#define BOOL_VAL(value) ((Value){ VAL_BOOL, { .boolean = value } })
#define NIL_VAL ((Value){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((Value){ VAL_NUMBER, { .number = value } })

#define IS(TYPE, value) ((value).type == VAL_##TYPE)
#define IS_BOOL(value) IS(BOOL, value)
#define IS_NIL(value) IS(NIL, value)
#define IS_NUMBER(value) IS(NUMBER, value)

bool values_equal(Value a, Value b);
bool values_identical(Value a, Value b);
void init_value_array(ValueArray *value_array);
void free_value_array(ValueArray *value_array);
void write_value_array(ValueArray *value_array, Value value);
void print_value(Value value);

#endif
