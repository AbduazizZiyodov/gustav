#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void init_chunk(chunk_t *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	init_value_array(&chunk->constants);
}

void write_chunk(chunk_t *chunk, uint8_t byte, size_t line)
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
	chunk->lines[chunk->count] = (int)line;
	chunk->count++;
}

void free_chunk(chunk_t *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);

	free_value_array(&chunk->constants);
	init_chunk(chunk);
}

size_t add_constant(chunk_t *chunk, value_t value)
{
	push(value);
	write_value_array(&chunk->constants, value);

	if (chunk->constants.count > UINT8_MAX) {
		gustav_error(EXIT_FAILURE, "Too many constants in one chunk");
	}
	pop();
	return chunk->constants.count - 1;
}
