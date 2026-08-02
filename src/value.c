#include <stdio.h>
#include <string.h>

#ifndef NAN_BOXING
#include "common.h"
#endif

#include "memory.h"
#include "object.h"
#include "value.h"

void ValueArray_Init(ValueArray *array)
{
	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}

void ValueArray_Write(ValueArray *array, Value value)
{
	if (array->capacity < array->count + 1) {
		size_t old_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(old_capacity);
		array->values = GROW_ARRAY(Value, array->values, old_capacity,
					   array->capacity);
	}
	array->values[array->count] = value;
	array->count++;
}

void ValueArray_Free(ValueArray *array)
{
	FREE_ARRAY(Value, array->values, array->capacity);
	ValueArray_Init(array);
}

void Value_Print(Value value)
{
#ifdef NAN_BOXING
	if (Bool_Check(value)) {
		printf(AS_BOOL(value) ? "true" : "false");
	} else if (Nil_Check(value)) {
		printf("nil");
	} else if (Number_Check(value)) {
		printf("%g", AS_NUMBER(value));
	} else if (Object_Check(value)) {
		Object_Print(value);
	}
#else
	switch (value.type) {
	case VAL_BOOL:
		printf(AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NIL:
		printf("nil");
		break;
	case VAL_NUMBER:
		// TODO(abduaziz): needs better handling
		printf("%f", AS_NUMBER(value));
		break;
	case VAL_OBJ:
		Object_Print(value);
		break;
	default:
		UNREACHABLE();
	}
#endif
}

bool Value_Equal(Value a, Value b)
{
#ifdef NAN_BOXING
	if (Number_Check(a) && Number_Check(b)) {
		return AS_NUMBER(a) == AS_NUMBER(b);
	}
	return a == b;
#else
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case VAL_BOOL:
		return AS_BOOL(a) == AS_BOOL(b);
	case VAL_NIL:
		return true;
	case VAL_NUMBER:
		return AS_NUMBER(a) == AS_NUMBER(b);
	case VAL_OBJ:
		return AS_OBJ(a) == AS_OBJ(b);
	default:
		UNREACHABLE();
	}
#endif
}
