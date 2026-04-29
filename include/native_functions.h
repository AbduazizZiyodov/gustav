#ifndef NATIVE_FUNCTIONS_H
#include "object.h"
#include "value.h"

value_t gustav_clock_native(size_t arg_count [[maybe_unused]],
			    value_t *args [[maybe_unused]]);

value_t gustav_sleep_native(size_t arg_count [[maybe_unused]],
			    value_t *args [[maybe_unused]]);

value_t gustav_max_native(size_t arg_count [[maybe_unused]],
			  value_t *args [[maybe_unused]]);

typedef struct {
	native_fn function;
	const char *name;
} NativeFunctionPair;

extern const NativeFunctionPair NATIVE_FUNCTIONS[3];

#endif
