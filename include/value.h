#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
	VAL_OBJ,
	VAL_UNINITIALIZED,
} ValueType;

typedef struct Object Object;
typedef struct StringObject StringObject;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		Object *obj;
	} as;
} Value;

#define AS_BOOL(value) (((value)).as.boolean)
#define AS_NUMBER(value) (((value)).as.number)
#define AS_OBJ(value) (((value)).as.obj)

#define BOOL_VAL(value) ((Value){ VAL_BOOL, { .boolean = (value) } })
#define NIL_VAL ((Value){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((Value){ VAL_NUMBER, { .number = (value) } })
#define OBJ_VAL(value) ((Value){ VAL_OBJ, { .obj = (Object *)(value) } })
#define UNINITIALIZED_VAL ((Value){ VAL_UNINITIALIZED, { .number = 0 } })

#define IS(TYPE, value) ((value).type == VAL_##TYPE)
#define Bool_Check(value) IS(BOOL, value)
#define Nil_Check(value) IS(NIL, value)
#define Number_Check(value) IS(NUMBER, value)
#define Object_Check(value) IS(OBJ, value)
#define Uninitialized_Check(value) ((value).type == VAL_UNINITIALIZED)

typedef struct {
	size_t count;
	size_t capacity;
	Value *values;
} ValueArray;

bool Value_Equal(Value a, Value b);

void ValueArray_Init(ValueArray *value_array);
void ValueArray_Free(ValueArray *value_array);
void ValueArray_Write(ValueArray *value_array, Value value);

void Value_Print(FILE *stream, Value value);
