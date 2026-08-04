#include <stddef.h>
#include <stdlib.h>

#include "common.h"
#include "native_functions.h"
#include "value.h"

Value gustav_max_native(int arg_count [[maybe_unused]], Value *args [[maybe_unused]])
{
	if (arg_count != 2) {
		Gustav_Error(EXIT_FAILURE, "Should be twins you mazafaka!");
	}
	double first = AS_NUMBER(args[0]);
	double second = AS_NUMBER(args[1]);

	return NUMBER_VAL(first > second ? first : second);
}
