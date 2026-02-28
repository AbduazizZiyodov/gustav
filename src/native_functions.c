#include "native_functions.h"

const NativeFunctionPair NATIVE_FUNCTIONS[2] = {
	//< clang-format off
	{ gustav_clock_native, "clock" },
	{ gustav_sleep_native, "sleep" }
	//< clang-format on
};
