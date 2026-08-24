#include <criterion/criterion.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

/* Minimal stubs so value.c links without the VM / GC / full object runtime. */
void Object_Print(FILE *stream, Value value)
{
	(void)value;
	fputs("<obj>", stream);
}

void *mem_realloc(void *pointer, size_t old_size, size_t new_size)
{
	(void)old_size;

	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, new_size);
	cr_assert_not_null(result, "realloc failed in unit-test stub");
	return result;
}

Test(value, equality_primitives)
{
	cr_assert(value_equal(BOOL_VAL(true), BOOL_VAL(true)));
	cr_assert(value_equal(BOOL_VAL(false), BOOL_VAL(false)));
	cr_assert_not(value_equal(BOOL_VAL(true), BOOL_VAL(false)));

	cr_assert(value_equal(NIL_VAL, NIL_VAL));
	cr_assert_not(value_equal(NIL_VAL, BOOL_VAL(false)));
	cr_assert_not(value_equal(NIL_VAL, NUMBER_VAL(0)));

	cr_assert(value_equal(NUMBER_VAL(1.5), NUMBER_VAL(1.5)));
	cr_assert_not(value_equal(NUMBER_VAL(1.0), NUMBER_VAL(2.0)));
	cr_assert_not(value_equal(NUMBER_VAL(0), BOOL_VAL(false)));
}

Test(value, equality_uninitialized)
{
	cr_assert_not(value_equal(UNINITIALIZED_VAL, UNINITIALIZED_VAL));
	cr_assert_not(value_equal(UNINITIALIZED_VAL, NIL_VAL));
}

Test(value, equality_object_identity)
{
	/* Object equality is pointer identity. */
	struct Object a;
	struct Object b;
	memset(&a, 0, sizeof(a));
	memset(&b, 0, sizeof(b));

	cr_assert(value_equal(OBJ_VAL(&a), OBJ_VAL(&a)));
	cr_assert_not(value_equal(OBJ_VAL(&a), OBJ_VAL(&b)));
	cr_assert_not(value_equal(OBJ_VAL(&a), NUMBER_VAL(0)));
}

Test(value, type_predicates_and_macros)
{
	Value t = BOOL_VAL(true);
	Value n = NUMBER_VAL(42);
	Value z = NIL_VAL;

	cr_assert(Bool_Check(t));
	cr_assert(AS_BOOL(t));
	cr_assert(Number_Check(n));
	cr_assert_float_eq(AS_NUMBER(n), 42.0, 1e-12);
	cr_assert(Nil_Check(z));
	cr_assert(Uninitialized_Check(UNINITIALIZED_VAL));
}

Test(value, value_array_grows_and_stores)
{
	ValueArray array;
	init_value_array(&array);

	cr_assert_eq(array.count, 0);
	cr_assert_eq(array.capacity, 0);
	cr_assert_null(array.values);

	for (int i = 0; i < 20; i++) {
		write_to_value_array(&array, NUMBER_VAL((double)i));
	}

	cr_assert_eq(array.count, 20);
	cr_assert(array.capacity >= 20);

	for (int i = 0; i < 20; i++) {
		cr_assert(Number_Check(array.values[i]));
		cr_assert_float_eq(AS_NUMBER(array.values[i]), (double)i, 1e-12);
	}

	free_value_array(&array);
	cr_assert_eq(array.count, 0);
	cr_assert_eq(array.capacity, 0);
	cr_assert_null(array.values);
}

Test(value, grow_capacity_macro)
{
	cr_assert_eq(GROW_CAPACITY(0), 8);
	cr_assert_eq(GROW_CAPACITY(7), 8);
	cr_assert_eq(GROW_CAPACITY(8), 16);
	cr_assert_eq(GROW_CAPACITY(16), 32);
}
