#include "chunk.h"
#include "debug.h"
#include "common.h"
#include "vm.h"

// for testing ...
#define MAKE_CONSTANT(chunk, value, line_no)               \
	do {                                               \
		constant = add_constant(&chunk, value);    \
		write_chunk(&chunk, OP_CONSTANT, line_no); \
		write_chunk(&chunk, constant, line_no);    \
	} while (false)

int main(void)
{
	init_vm();

	Chunk chunk;

	init_chunk(&chunk);

	LOG_CHUNK(chunk);

	const uint8_t line_no = 21; // dummy

	size_t constant;

	MAKE_CONSTANT(chunk, 2.1, line_no);
	write_chunk(&chunk, OP_NEGATE, line_no);

	write_chunk(&chunk, OP_RETURN, line_no);

	LOG_CHUNK(chunk)

	disassemble_chunk(&chunk, "Test Chunk");
	interpret(&chunk);

	free_vm();
	free_chunk(&chunk);

	return EXIT_SUCCESS;
}
