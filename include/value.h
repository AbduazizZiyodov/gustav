#ifndef GUSTAV_VALUE_H
#define GUSTAV_VALUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ } ValueType;

typedef struct Obj obj_t;
typedef struct string_t string_t;

#ifdef NAN_BOXING

#define QNAN ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define TAG_NIL 1 // 01_2
#define TAG_FALSE 2 // 10_2
#define TAG_TRUE 3 // 11_2

typedef uint64_t value_t;

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUMBER(value) value_to_num(value)

static inline obj_t *value_to_obj(value_t value)
{
	uint64_t bits = value & ~(SIGN_BIT | QNAN);
	obj_t *ptr;
	memcpy((void *)&ptr, &bits, sizeof(uint64_t));
	return ptr;
}

#define AS_OBJ(value) value_to_obj(value)

#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define NIL_VAL ((value_t)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) num_to_value(num)
#define OBJ_VAL(obj) (value_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define FALSE_VAL ((value_t)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((value_t)(uint64_t)(QNAN | TAG_TRUE))

#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_NIL(value) ((value) == NIL_VAL)
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

static inline value_t num_to_value(double num)
{
	value_t value;
	memcpy(&value, &num, sizeof(double));
	return value;
}

static inline double value_to_num(value_t value)
{
	double num;
	memcpy(&num, &value, sizeof(value_t));
	return num;
}

#else
typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		obj_t *obj;
	} as;
} value_t;

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
#endif

typedef struct {
	size_t count;
	size_t capacity;
	value_t *values;
} value_array_t;

bool values_equal(value_t a, value_t b);

void init_value_array(value_array_t *value_array);
void free_value_array(value_array_t *value_array);
void write_value_array(value_array_t *value_array, value_t value);

void print_value(value_t value);

#endif // GUSTAV_VALUE_H
