#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ } ValueType;

typedef struct Object Object;
typedef struct StringObject StringObject;

#ifdef NAN_BOXING

#define QNAN ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define TAG_NIL 1 // 01_2
#define TAG_FALSE 2 // 10_2
#define TAG_TRUE 3 // 11_2

typedef uint64_t Value;

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUMBER(value) value_to_num(value)

static inline Object *value_to_obj(Value value)
{
	uint64_t bits = value & ~(SIGN_BIT | QNAN);
	Object *ptr;
	memcpy((void *)&ptr, &bits, sizeof(uint64_t));
	return ptr;
}

#define AS_OBJ(value) value_to_obj(value)

#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define NIL_VAL ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) num_to_value(num)
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))

#define Bool_Check(value) (((value) | 1) == TRUE_VAL)
#define Nil_Check(value) ((value) == NIL_VAL)
#define Number_Check(value) (((value) & QNAN) != QNAN)
#define Object_Check(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

static inline Value num_to_value(double num)
{
	Value value;
	memcpy(&value, &num, sizeof(double));
	return value;
}

static inline double value_to_num(Value value)
{
	double num;
	memcpy(&num, &value, sizeof(Value));
	return num;
}

#else
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

#define IS(TYPE, value) ((value).type == VAL_##TYPE)
#define Bool_Check(value) IS(BOOL, value)
#define Nil_Check(value) IS(NIL, value)
#define Number_Check(value) IS(NUMBER, value)
#define Object_Check(value) IS(OBJ, value)
#endif

typedef struct {
	size_t count;
	size_t capacity;
	Value *values;
} ValueArray;

bool Value_Equal(Value a, Value b);

void ValueArray_Init(ValueArray *value_array);
void ValueArray_Free(ValueArray *value_array);
void ValueArray_Write(ValueArray *value_array, Value value);

void Value_Print(Value value);
