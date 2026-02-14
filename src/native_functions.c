#include <time.h>
#include <unistd.h>

#include "native_functions.h"
#include "value.h"

value_t clock_native(size_t arg_count __attribute__((unused)),
		     value_t *args __attribute__((unused)))
{
	return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

value_t gustav_sleep(size_t arg_count __attribute__((unused)),
		     value_t *args __attribute__((unused)))
{
	/* NOLINTNEXTLINE(concurrency-mt-unsafe) */
	sleep((unsigned int)AS_NUMBER(args[0]));
	return NIL_VAL;
}

const NativeFunctionPair NATIVE_FUNCTIONS[2] = { { clock_native, "clock" },
						 { gustav_sleep, "sleep" } };
