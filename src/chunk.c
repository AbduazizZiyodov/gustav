#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void Chunk_Init(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	ValueArray_Init(&chunk->constants);
}

void Chunk_Write(Chunk *chunk, uint8_t byte, size_t line)
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

void Chunk_Free(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);

	ValueArray_Free(&chunk->constants);
	Chunk_Init(chunk);
}

size_t Chunk_AddConstant(Chunk *chunk, Value value)
{
	VM_Push(value);
	ValueArray_Write(&chunk->constants, value);

	if (chunk->constants.count > UINT8_MAX) {
		gustav_error(EXIT_FAILURE, "Too many constants in one chunk");
	}
	VM_Pop();
	return chunk->constants.count - 1;
}
