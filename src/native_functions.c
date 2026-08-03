#include <stddef.h>

#include "common.h"
#include "native_functions.h"

// TODO(abduaziz): came up with even prettier option(s) for native function registration
const NativeFunctionPair NATIVE_FUNCTIONS[] = { { gustav_clock_native, "clock" },
						{ gustav_sleep_native, "sleep" },
						{ gustav_max_native, "max" } };

const size_t NATIVE_FUNCTION_COUNT = ARRAY_LENGTH(NATIVE_FUNCTIONS);
