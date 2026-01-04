#include <chunk.h>
#include "value.h"
#include "memory.h"

void init_chunk(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	init_value_array(&chunk->constants);
	LOG_DEBUG("Chunk was initialized");
}

void write_chunk(Chunk *chunk, uint8_t byte, int line)
{
	if (chunk->capacity < chunk->count + 1) {
		size_t old_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(old_capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity,
					 chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity,
					  chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = line;
	chunk->count++;
}

void free_chunk(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);

	free_value_array(&chunk->constants);
	init_chunk(chunk);
	LOG_DEBUG("Chunk was freed");
}

size_t add_constant(Chunk *chunk, Value value)
{
	write_value_array(&chunk->constants, value);

	if (chunk->constants.count > UINT8_MAX) {
		fprintf(stderr, "Error: Too many constants in one chunk.\n");
		exit(EXIT_FAILURE);
	}

	return chunk->constants.count - 1;
}
