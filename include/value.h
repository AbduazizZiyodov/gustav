#ifndef GUSTAV_VALUE_H
#define GUSTAV_VALUE_H

#include <stddef.h>

typedef double Value;

typedef struct {
	size_t count;
	size_t capacity;
	Value *values;
} ValueArray;

void init_value_array(ValueArray *value_array);
void free_value_array(ValueArray *value_array);
void write_value_array(ValueArray *value_array, Value value);
void print_value(Value value);

#endif
