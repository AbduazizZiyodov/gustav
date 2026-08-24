#include <criterion/criterion.h>
#include <stdio.h>

#include "hash_table.h"
#include "object.h"
#include "vm.h"

static void hash_table_suite_init(void)
{
	init_vm();
}

static void hash_table_suite_fini(void)
{
	free_vm();
}

TestSuite(hash_table, .init = hash_table_suite_init, .fini = hash_table_suite_fini);

Test(hash_table, set_get_overwrite_delete)
{
	HashTable table;
	init_hash_table(&table);

	StringObject *key = string_from_chars("answer", 6);
	cr_assert_not_null(key);

	cr_assert(hash_table_set_item(&table, key, NUMBER_VAL(42)));

	Value out = NIL_VAL;
	cr_assert(hash_table_get_item(&table, key, &out));
	cr_assert(Number_Check(out));
	cr_assert_float_eq(AS_NUMBER(out), 42.0, 1e-12);

	/* same key is not a new insertion */
	cr_assert_not(hash_table_set_item(&table, key, NUMBER_VAL(7)));
	cr_assert(hash_table_get_item(&table, key, &out));
	cr_assert_float_eq(AS_NUMBER(out), 7.0, 1e-12);

	StringObject *interned = string_from_chars("answer", 6);
	cr_assert_eq(interned, key);

	cr_assert(hash_table_delete_item(&table, key));
	cr_assert_not(hash_table_get_item(&table, key, &out));
	cr_assert_not(hash_table_delete_item(&table, key));

	free_tash_table(&table);
}

Test(hash_table, grows_past_initial_capacity)
{
	HashTable table;
	init_hash_table(&table);

	for (int i = 0; i < 32; i++) {
		char name[16];
		int written = snprintf(name, sizeof(name), "k%d", i);
		cr_assert(written > 0);
		StringObject *key = string_from_chars(name, (size_t)written);
		cr_assert(hash_table_set_item(&table, key, NUMBER_VAL((double)i)));
	}

	cr_assert(table.capacity >= 32);
	cr_assert_eq(table.count, (size_t)32);

	StringObject *probe = string_from_chars("k17", 3);
	Value out = NIL_VAL;
	cr_assert(hash_table_get_item(&table, probe, &out));
	cr_assert_float_eq(AS_NUMBER(out), 17.0, 1e-12);

	free_tash_table(&table);
}
