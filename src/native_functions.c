#include "native_functions.h"

// TODO(abduaziz): came up with even prettier option(s) for native function registration
const NativeFunctionPair NATIVE_FUNCTIONS[3] = {
	//< clang-format off
	{ gustav_clock_native, "clock" },
	{ gustav_sleep_native, "sleep" },
	{ gustav_max_native, "max" }
	//< clang-format on
};
