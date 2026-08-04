#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void chunk_init(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	init_value_array(&chunk->constants);
}

void chunk_write(Chunk *chunk, uint8_t byte, size_t line)
{
	if (chunk->capacity < chunk->count + 1) {
		size_t old_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(old_capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = (int)line;
	chunk->count++;
}

void chunk_free(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);

	free_value_array(&chunk->constants);
	chunk_init(chunk);
}

size_t chunk_add_constant(Chunk *chunk, Value value)
{
	vm_push(value);
	write_to_value_array(&chunk->constants, value);

	if (chunk->constants.count > UINT8_MAX) {
		gustav_error(EXIT_FAILURE, "Too many constants in one chunk");
	}
	vm_pop();
	return chunk->constants.count - 1;
}
