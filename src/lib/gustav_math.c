#include <stddef.h>

#include "common.h"
#include "native_functions.h"
#include "value.h"


value_t gustav_max_native(size_t arg_count __attribute__((unused)),
			  value_t *args __attribute__((unused)))
{
	if (arg_count != 2) {
		gustav_error(-1, "Should be twins you mazafaka!");
	}
	double first = AS_NUMBER(args[0]);
	double second = AS_NUMBER(args[1]);

	return NUMBER_VAL(first > second ? first : second);
}
