#pragma once

#include <stddef.h>

#include "object.h"
#include "value.h"

Value gustav_clock_native(int arg_count [[maybe_unused]],
			  Value *args [[maybe_unused]]);

Value gustav_sleep_native(int arg_count [[maybe_unused]],
			  Value *args [[maybe_unused]]);

Value gustav_max_native(int arg_count [[maybe_unused]],
			Value *args [[maybe_unused]]);

typedef struct {
	NativeFn function;
	const char *name;
} NativeFunctionPair;

extern const NativeFunctionPair NATIVE_FUNCTIONS[];
extern const size_t NATIVE_FUNCTION_COUNT;
