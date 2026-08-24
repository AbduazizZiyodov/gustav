#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>

#include "value.h"
#include "vm.h"

static void value_suite_init(void)
{
	init_vm();
}

static void value_suite_fini(void)
{
	free_vm();
}

TestSuite(value, .init = value_suite_init, .fini = value_suite_fini);

Test(value, macros_tag_payloads)
{
	Value t = BOOL_VAL(true);
	Value f = BOOL_VAL(false);
	Value n = NIL_VAL;
	Value num = NUMBER_VAL(3.14);
	Value uninit = UNINITIALIZED_VAL;

	cr_assert(Bool_Check(t));
	cr_assert(AS_BOOL(t));
	cr_assert(Bool_Check(f));
	cr_assert_not(AS_BOOL(f));
	cr_assert(Nil_Check(n));
	cr_assert(Number_Check(num));
	cr_assert_float_eq(AS_NUMBER(num), 3.14, 1e-12);
	cr_assert(Uninitialized_Check(uninit));
}

Test(value, equality)
{
	cr_assert(value_equal(NUMBER_VAL(1.0), NUMBER_VAL(1.0)));
	cr_assert_not(value_equal(NUMBER_VAL(1.0), NUMBER_VAL(2.0)));
	cr_assert(value_equal(BOOL_VAL(true), BOOL_VAL(true)));
	cr_assert_not(value_equal(BOOL_VAL(true), BOOL_VAL(false)));
	cr_assert(value_equal(NIL_VAL, NIL_VAL));
	cr_assert_not(value_equal(NIL_VAL, NUMBER_VAL(0.0)));
	cr_assert_not(value_equal(UNINITIALIZED_VAL, UNINITIALIZED_VAL));
}

Test(value, value_array_grows_and_stores)
{
	ValueArray array;
	init_value_array(&array);

	for (int i = 0; i < 20; i++) {
		write_to_value_array(&array, NUMBER_VAL((double)i));
	}

	cr_assert_eq(array.count, (size_t)20);
	cr_assert(array.capacity >= array.count);
	cr_assert_float_eq(AS_NUMBER(array.values[0]), 0.0, 1e-12);
	cr_assert_float_eq(AS_NUMBER(array.values[19]), 19.0, 1e-12);

	free_value_array(&array);
	cr_assert_eq(array.count, (size_t)0);
	cr_assert_eq(array.capacity, (size_t)0);
	cr_assert_null(array.values);
}

Test(value, print_primitives)
{
	char buffer[64];
	FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
	cr_assert_not_null(stream);

	print_value(stream, NUMBER_VAL(12));
	fclose(stream);
	cr_assert(strstr(buffer, "12") != NULL);

	memset(buffer, 0, sizeof(buffer));
	stream = fmemopen(buffer, sizeof(buffer), "w");
	cr_assert_not_null(stream);
	print_value(stream, BOOL_VAL(true));
	fclose(stream);
	cr_assert_str_eq(buffer, "true");

	memset(buffer, 0, sizeof(buffer));
	stream = fmemopen(buffer, sizeof(buffer), "w");
	cr_assert_not_null(stream);
	print_value(stream, NIL_VAL);
	fclose(stream);
	cr_assert_str_eq(buffer, "nil");
}
