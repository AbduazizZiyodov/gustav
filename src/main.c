#include "chunk.h"
#include "debug.h"
#include "common.h"

int main(void)
{
	Chunk chunk;

	init_chunk(&chunk);
	LOG_INFO("Chunk was initialized");
	LOG_CHUNK(chunk);

	size_t constant = add_constant(&chunk, 2.1);

	write_chunk(&chunk, OP_CONSTANT, 132);
	write_chunk(&chunk, constant, 132);
	write_chunk(&chunk, OP_RETURN, 132);

	LOG_INFO("Chunks were added");
	LOG_CHUNK(chunk)

	disassemble_chunk(&chunk, "Test Chunk");
	free_chunk(&chunk);

	LOG_INFO("Chunk was freed");

	return EXIT_SUCCESS;
}
