#include "native_functions.h"

const NativeFunctionPair NATIVE_FUNCTIONS[2] = {
	//< clang-format off
	{ clock_native, "clock" },
	{ gustav_sleep, "sleep" }
	//< clang-format on
};
