#ifndef NATIVE_FUNCTIONS_H
#include "object.h"
#include "value.h"

value_t clock_native(size_t arg_count __attribute__((unused)),
		     value_t *args __attribute__((unused)));

value_t gustav_sleep(size_t arg_count __attribute__((unused)),
		     value_t *args __attribute__((unused)));

typedef struct {
	native_fn function;
	const char *name;
} NativeFunctionPair;

extern const NativeFunctionPair NATIVE_FUNCTIONS[2];

#endif
