#include <criterion/criterion.h>

#include "chunk.h"
#include "vm.h"

static void chunk_suite_init(void)
{
	init_vm();
}

static void chunk_suite_fini(void)
{
	free_vm();
}

TestSuite(chunk, .init = chunk_suite_init, .fini = chunk_suite_fini);

Test(chunk, write_bytes_and_constants)
{
	Chunk chunk;
	chunk_init(&chunk);

	chunk_write(&chunk, OP_NIL, 1);
	chunk_write(&chunk, OP_RETURN, 1);
	size_t index = chunk_add_constant(&chunk, NUMBER_VAL(7.0));
	chunk_write(&chunk, OP_CONSTANT, 2);
	chunk_write(&chunk, (uint8_t)index, 2);

	cr_assert_eq(chunk.count, (size_t)4);
	cr_assert_eq(chunk.code[0], (uint8_t)OP_NIL);
	cr_assert_eq(chunk.code[1], (uint8_t)OP_RETURN);
	cr_assert_eq(chunk.code[2], (uint8_t)OP_CONSTANT);
	cr_assert_eq(chunk.code[3], (uint8_t)index);
	cr_assert_eq(chunk.lines[0], 1);
	cr_assert_eq(chunk.lines[2], 2);
	cr_assert_eq(chunk.constants.count, (size_t)1);
	cr_assert(Number_Check(chunk.constants.values[0]));
	cr_assert_float_eq(AS_NUMBER(chunk.constants.values[0]), 7.0, 1e-12);

	chunk_free(&chunk);
	cr_assert_eq(chunk.count, (size_t)0);
	cr_assert_null(chunk.code);
}
