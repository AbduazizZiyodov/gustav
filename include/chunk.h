#ifndef GUSTAV_CHUNK_H
#define GUSTAV_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_RETURN,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE
} OpCode;

typedef struct {
	size_t count;
	size_t capacity;
	uint8_t *code;
	ValueArray constants;
	int *lines;
} Chunk;

void init_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);
void write_chunk(Chunk *chunk, uint8_t byte, int line);
size_t add_constant(Chunk *chunk, Value value);
void write_constant(Chunk *chunk, Value value, int line);

#define LOG_CHUNK(chunk) \
	LOG_DEBUG("Chunk(capacity=%d count=%d)", chunk.capacity, chunk.count);

#endif
